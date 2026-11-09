"""train_nn_dll.py —— 用 sim.dll 做数据采集、训练、回测、导出权重

这样训练和游戏用的是同一份规则（C 后端），不会两套逻辑漂移。
"""
import ctypes, os, numpy as np

BASE = os.path.expanduser(r"~\Desktop\C\stock-moon")
DLL = ctypes.CDLL(os.path.join(BASE, "_archive", "sim.dll"))

DLL.sim_new.restype = ctypes.c_void_p
DLL.sim_new.argtypes = [ctypes.c_int, ctypes.c_uint]
DLL.sim_free.argtypes = [ctypes.c_void_p]
DLL.sim_step.argtypes = [ctypes.c_void_p]
DLL.sim_day.argtypes = [ctypes.c_void_p]; DLL.sim_day.restype = ctypes.c_int
DLL.sim_price.argtypes = [ctypes.c_void_p, ctypes.c_int]; DLL.sim_price.restype = ctypes.c_int
DLL.sim_change.argtypes = [ctypes.c_void_p, ctypes.c_int]; DLL.sim_change.restype = ctypes.c_int
DLL.sim_shares.argtypes = [ctypes.c_void_p, ctypes.c_int]; DLL.sim_shares.restype = ctypes.c_int
DLL.sim_mood100.argtypes = [ctypes.c_void_p]; DLL.sim_mood100.restype = ctypes.c_int
DLL.sim_news_idx.argtypes = [ctypes.c_void_p]; DLL.sim_news_idx.restype = ctypes.c_int
DLL.sim_news_pct.argtypes = [ctypes.c_void_p]; DLL.sim_news_pct.restype = ctypes.c_int
DLL.sim_cash.argtypes = [ctypes.c_void_p]; DLL.sim_cash.restype = ctypes.c_longlong
DLL.sim_slope.argtypes = [ctypes.c_void_p, ctypes.c_int]; DLL.sim_slope.restype = ctypes.c_int
DLL.sim_base.argtypes = [ctypes.c_int]; DLL.sim_base.restype = ctypes.c_int
DLL.sim_get_hist.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]
DLL.sim_buy.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]; DLL.sim_buy.restype = ctypes.c_int
DLL.sim_sell.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]; DLL.sim_sell.restype = ctypes.c_int

N = 12
HIST = 20


class Sim:
    def __init__(self, difficulty=1, seed=1):
        self.p = DLL.sim_new(difficulty, seed)

    def __del__(self):
        if getattr(self, "p", None):
            DLL.sim_free(self.p)

    def step(self): DLL.sim_step(self.p)
    def day(self): return DLL.sim_day(self.p)
    def price(self, i): return DLL.sim_price(self.p, i)
    def change(self, i): return DLL.sim_change(self.p, i)
    def shares(self, i): return DLL.sim_shares(self.p, i)
    def mood(self): return DLL.sim_mood100(self.p) / 100.0
    def news(self): return DLL.sim_news_idx(self.p), DLL.sim_news_pct(self.p)
    def cash(self): return DLL.sim_cash(self.p)
    def slope(self, i): return DLL.sim_slope(self.p, i)
    def base(self, i): return DLL.sim_base(i)
    def buy(self, i, q): return DLL.sim_buy(self.p, i, q)
    def sell(self, i, q): return DLL.sim_sell(self.p, i, q)

    def hist(self, i):
        buf = (ctypes.c_int * HIST)()
        DLL.sim_get_hist(self.p, i, buf)
        return list(buf)

    def features(self, i):
        px = max(1, self.price(i))
        h = self.hist(i)              # 20 根收盘，索引 19 是今天
        ni, npct = self.news()

        def ret(k):                   # k 天前到今天的收益
            j = 19 - k
            if j < 0 or h[j] <= 0:
                return 0.0
            return px / h[j] - 1.0

        ma5 = sum(h[15:]) / 5.0
        ma20 = sum(h) / 20.0

        rs = []
        for k in range(1, 6):
            a, b = h[19 - k], h[20 - k] if 20 - k < 20 else h[19]
            rs.append((a - b) / b if b else 0.0)
        vol5 = (sum(r * r for r in rs) / 5) ** 0.5

        hi = max(h)
        return [
            px / self.base(i),          # 0 相对基准价
            self.change(i) / 20.0,      # 1 当日涨跌
            ret(1),                     # 2 近 1 日
            ret(3),                     # 3 近 3 日
            ret(5),                     # 4 近 5 日
            ret(10),                    # 5 近 10 日
            px / ma5 - 1.0 if ma5 else 0.0,     # 6 偏离 5 日均线
            px / ma20 - 1.0 if ma20 else 0.0,   # 7 偏离 20 日均线
            vol5,                       # 8 近 5 日波动
            self.mood(),                # 9 情绪
            npct / 25.0 if ni == i else 0.0,    # 10 当天新闻
            px / hi - 1.0 if hi else 0.0,       # 11 距 20 日高点
        ]

    def net_worth(self):
        return self.cash() + sum(self.shares(i) * self.price(i) for i in range(N))


def collect(episodes=60):
    X, Y = [], []
    for e in range(episodes):
        s = Sim(1, e + 1)
        while s.day() <= 365:
            p0 = [s.price(i) for i in range(N)]
            feats = [s.features(i) for i in range(N)]
            s.step()
            day_rows = []
            for i in range(N):
                if p0[i] <= 0:
                    continue
                if s.price(i) == s.base(i) and p0[i] < s.base(i) * 0.5:
                    continue
                ret = (s.price(i) - p0[i]) / p0[i]
                if abs(ret) > 0.3:
                    continue
                day_rows.append((feats[i], ret))
            if day_rows:
                m = sum(r for _, r in day_rows) / len(day_rows)
                for fx, r in day_rows:
                    X.append(fx)
                    Y.append(r - m)     # 相对市场的超额收益
    return np.array(X), np.array(Y)


class Net:
    def __init__(self, d=12, h=32):
        rng = np.random.default_rng(7)
        self.W1 = rng.standard_normal((d, h)) * 0.1
        self.b1 = np.zeros((1, h))
        self.W2 = rng.standard_normal((h, 1)) * 0.1
        self.b2 = np.zeros((1, 1))

    def fwd(self, x):
        h = np.tanh(x @ self.W1 + self.b1)
        return h @ self.W2 + self.b2, h

    def fit(self, X, Y, epochs=30, lr=3e-3, bs=256):
        self.mu = X.mean(0); self.sd = X.std(0); self.sd[self.sd == 0] = 1
        Xs = (X - self.mu) / self.sd
        rng = np.random.default_rng(11)
        P = [self.W1, self.b1, self.W2, self.b2]
        m = [np.zeros_like(p) for p in P]; v = [np.zeros_like(p) for p in P]
        step = 0
        for _ in range(epochs):
            idx = rng.permutation(len(Xs))
            for k in range(0, len(idx), bs):
                b = idx[k:k + bs]
                out, h = self.fwd(Xs[b])
                err = out - Y[b][:, None]
                dh = (err @ self.W2.T) * (1 - h * h)
                g = [Xs[b].T @ dh / len(b), dh.mean(0, keepdims=True),
                     h.T @ err / len(b), err.mean(0, keepdims=True)]
                step += 1
                for i in range(4):
                    m[i] = 0.9 * m[i] + 0.1 * g[i]
                    v[i] = 0.999 * v[i] + 0.001 * g[i] ** 2
                    P[i] -= lr * (m[i] / (1 - 0.9 ** step)) / (np.sqrt(v[i] / (1 - 0.999 ** step)) + 1e-8)
        return self

    def predict(self, x):
        return float(self.fwd((np.array(x) - self.mu) / self.sd)[0].ravel()[0])


def run_policy(net, seed, max_pos=3):
    s = Sim(1, seed)
    while s.day() <= 365:
        scored = []
        for i in range(N):
            if s.price(i) <= s.base(i) * 0.4:
                continue
            scored.append((net.predict(s.features(i)), i))
        scored.sort(reverse=True)

        for pr, i in scored:
            if s.shares(i) > 0 and pr < 0:
                s.sell(i, s.shares(i))

        held = sum(1 for i in range(N) if s.shares(i) > 0)
        slots = max_pos - held
        if slots > 0:
            budget = s.cash() // slots
            for pr, i in scored[:slots]:
                if pr > 0 and s.shares(i) == 0:
                    qty = budget // s.price(i)
                    if qty > 0:
                        s.buy(i, qty)
        s.step()
    return s.net_worth()


if __name__ == "__main__":
    X, Y = collect(60)
    print("samples", X.shape, "mean ret %.5f" % Y.mean())
    net = Net().fit(X, Y, epochs=30)
    pred = np.array([net.predict(x) for x in X[:5000]])
    print("IC %.3f" % np.corrcoef(pred, Y[:5000])[0, 1])

    scores = [run_policy(net, s + 100) for s in range(10)]
    print("policy: mean %.0f min %d max %d" % (np.mean(scores), min(scores), max(scores)))

    # 导出
    def arr(name, a):
        flat = np.asarray(a, dtype=np.float64).ravel()
        return "static const float %s[%d] = { %s };\n" % (name, flat.size, ", ".join("%.6ff" % v for v in flat))

    txt = "/* nn_weights.h —— 由 sim.dll 训练后导出 */\n#pragma once\n\n"
    txt += arr("NN_W1", net.W1) + arr("NN_B1", net.b1) + arr("NN_W2", net.W2) + arr("NN_B2", net.b2)
    txt += arr("NN_MU", net.mu) + arr("NN_SD", net.sd)
    open(os.path.join(BASE, "src", "nn_weights.h"), "w", encoding="gbk", newline="\n").write(txt)
    print("nn_weights.h updated")
