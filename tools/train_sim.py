"""train_sim.py —— 独立后端：复刻游戏规则，批量生成数据、训练 MLP、评估策略

规则对齐 Moon Stocks：
  价格基准 100+i*17；日涨跌 = U(-v,v+1) + 情绪漂移(-2..2) + 交易压力，clamp[-100,30]
  新闻 45%/天，随机一只 ±10..25%；情绪 0.85 平滑、无新闻时 *0.98
  跌破 0：持仓清零、价格重置回基准
"""
import numpy as np

N = 12
BASE = 100 + np.arange(N) * 17
VOL = {0: 5, 1: 8, 2: 12}


class Sim:
    def __init__(self, difficulty=1, seed=0):
        self.rng = np.random.default_rng(seed)
        self.v = VOL[difficulty]
        self.price = BASE.astype(np.int64).copy()
        self.change = np.zeros(N, dtype=np.int64)
        self.shares = np.zeros(N, dtype=np.int64)
        self.hist = np.zeros((N, 20), dtype=np.int64)
        for i in range(N):
            self.hist[i] = self.price[i]
            for j in range(18, -1, -1):
                chg = self.rng.integers(-5, 6)
                val = self.hist[i, j + 1] * (100 - chg) // 100
                self.hist[i, j] = max(1, val) if j + 1 < 20 else val
            self.hist[i, 19] = self.price[i]
        self.mood = 0.0
        self.pressure = np.zeros(N, dtype=np.int64)
        self.news_idx = -1
        self.news_pct = 0
        self.day = 1

    def slope(self, i, n=10):
        xs = self.hist[i, -n:].astype(float)
        m = len(xs)
        if m < 2:
            return 0.0
        sx = np.arange(m).sum(); sy = xs.sum()
        sxx = (np.arange(m) ** 2).sum(); sxy = (np.arange(m) * xs).sum()
        den = m * sxx - sx * sx
        return 0.0 if den == 0 else (m * sxy - sx * sy) / den

    def features(self, i):
        price = max(1, int(self.price[i]))
        avg20 = self.hist[i].mean()
        news = self.news_pct / 25.0 if self.news_idx == i else 0.0
        return [price / BASE[i], self.change[i] / 20.0, self.slope(i) / price,
                price / avg20 - 1.0 if avg20 else 0.0, self.mood, news, 1.0]

    def step(self):
        rng = self.rng
        self.news_idx = -1
        self.news_pct = 0
        if rng.integers(0, 100) < 45:
            self.news_idx = int(rng.integers(0, N))
            pct = int(rng.integers(10, 26))
            if rng.integers(0, 2) == 0:
                pct = -pct
            self.news_pct = pct
            v = pct / 25.0
            self.mood = max(-1.0, min(1.0, self.mood * 0.85 + v * 0.15))
        else:
            self.mood *= 0.98

        drift = int(self.mood * 2.0)
        for i in range(N):
            change = int(rng.integers(-self.v, self.v + 1)) + drift
            if rng.integers(0, 2) == 0:      # 一半的日子 +1%，长期近似零和
                change += 1
            if i == self.news_idx:
                change += self.news_pct
            change += int(self.pressure[i])
            self.pressure[i] = 0
            change = max(-100, min(30, change))

            if self.price[i] * (100 + change) // 100 < 1:
                self.shares[i] = 0
                self.price[i] = BASE[i]
                self.change[i] = 0
                self.hist[i] = BASE[i]
                continue

            close = max(0, int(self.price[i] * (100 + change) // 100))
            self.hist[i] = np.roll(self.hist[i], -1)
            self.hist[i, -1] = close
            self.price[i] = close
            self.change[i] = change
        self.day += 1


def collect(episodes=60):
    X, Y = [], []
    for e in range(episodes):
        sim = Sim(1, seed=e)
        while sim.day <= 365:
            before = sim.price.copy()
            feats = [sim.features(i) for i in range(N)]
            news_i, news_p = sim.news_idx, sim.news_pct
            sim.step()
            for i in range(N):
                p0 = before[i]
                if p0 <= 0:
                    continue
                ret = (sim.price[i] - p0) / p0
                # 退市重置：价格跳回基准价，这种样本要剔除（否则标签被假涨幅污染）
                if sim.price[i] == BASE[i] and p0 < BASE[i] * 0.5:
                    continue
                if abs(ret) > 0.3:
                    continue
                X.append(feats[i])
                Y.append(ret)
    return np.array(X), np.array(Y)


class Net:
    def __init__(self, d=7, h=32, seed=0):
        rng = np.random.default_rng(seed)
        self.W1 = rng.standard_normal((d, h)) * 0.1
        self.b1 = np.zeros((1, h))
        self.W2 = rng.standard_normal((h, 1)) * 0.1
        self.b2 = np.zeros((1, 1))

    def forward(self, x):
        h = np.tanh(x @ self.W1 + self.b1)
        return h @ self.W2 + self.b2, h

    def fit(self, X, Y, epochs=40, lr=3e-3, bs=256, seed=0):
        mu, sd = X.mean(0), X.std(0)
        sd[sd == 0] = 1
        Xs = (X - mu) / sd
        rng = np.random.default_rng(seed)
        m = [np.zeros_like(p) for p in (self.W1, self.b1, self.W2, self.b2)]
        v = [np.zeros_like(p) for p in (self.W1, self.b1, self.W2, self.b2)]
        step = 0
        for ep in range(epochs):
            idx = rng.permutation(len(Xs))
            for s in range(0, len(idx), bs):
                b = idx[s:s + bs]
                out, h = self.forward(Xs[b])
                err = out - Y[b][:, None]
                dh = (err @ self.W2.T) * (1 - h * h)
                grads = [Xs[b].T @ dh / len(b), dh.mean(0, keepdims=True),
                         h.T @ err / len(b), err.mean(0, keepdims=True)]
                step += 1
                for i in range(4):
                    p = [self.W1, self.b1, self.W2, self.b2][i]
                    m[i] = 0.9 * m[i] + 0.1 * grads[i]
                    v[i] = 0.999 * v[i] + 0.001 * grads[i] ** 2
                    p -= lr * (m[i] / (1 - 0.9 ** step)) / (np.sqrt(v[i] / (1 - 0.999 ** step)) + 1e-8)
        self.mu, self.sd = mu, sd
        return self

    def predict(self, x):
        return float(self.forward((np.array(x) - self.mu) / self.sd)[0].ravel()[0])


def run_policy(net, seed, max_pos=3):
    sim = Sim(1, seed=seed)
    cash = 20000
    while sim.day <= 365:
        preds = []
        for i in range(N):
            if sim.price[i] <= BASE[i] * 0.4:
                continue
            preds.append((net.predict(sim.features(i)), i))
        preds.sort(reverse=True)

        for pr, i in preds:
            if sim.shares[i] > 0 and pr < 0:
                cash += sim.shares[i] * sim.price[i]
                sim.pressure[i] -= sim.shares[i] // 100
                sim.shares[i] = 0

        held = int((sim.shares > 0).sum())
        slots = max_pos - held
        if slots > 0:
            budget = cash // slots
            for pr, i in preds[:slots]:
                if pr > 0 and sim.shares[i] == 0:
                    qty = budget // sim.price[i]
                    if qty > 0:
                        cash -= qty * sim.price[i]
                        sim.shares[i] += qty
                        sim.pressure[i] += qty // 100
        sim.step()
    return cash + int((sim.shares * sim.price).sum())


def run_hold(seed):
    sim = Sim(1, seed=seed)
    cash = 20000
    while sim.day <= 365:
        sim.step()
    return cash + int((sim.shares * sim.price).sum())


if __name__ == "__main__":
    X, Y = collect(40)
    print("data", X.shape, "mean ret %.5f" % Y.mean())
    net = Net().fit(X, Y, epochs=30)
    pred = np.array([net.predict(x) for x in X[:3000]])
    ic = np.corrcoef(pred, Y[:3000])[0, 1]
    print("IC(train-ish) %.3f" % ic)

    scores = [run_policy(net, s) for s in range(10)]
    print("policy net worth: mean %.0f  min %d  max %d" % (np.mean(scores), min(scores), max(scores)))
    np.savez("sim_model.npz", W1=net.W1, b1=net.b1, W2=net.W2, b2=net.b2, mu=net.mu, sd=net.sd)
    print("saved sim_model.npz")
