"""train_nn.py —— 用采集到的数据训练一个小 MLP 预测次日涨跌

网络：7 -> 32 (tanh) -> 1，MSE，Adam；输出标准化后可比较排序。
"""
import numpy as np, os

BASE = os.path.expanduser(r"~\Desktop\C\stock-moon")
DATA = os.path.join(BASE, "dataset.csv")
MODEL = os.path.join(BASE, "model.npz")

X, Y = [], []
with open(DATA, encoding="utf-8") as f:
    for ln in f:
        v = [float(x) for x in ln.strip().split(",")]
        X.append(v[:-1])
        Y.append(v[-1])

X = np.array(X, dtype=np.float64)
Y = np.array(Y, dtype=np.float64)[:, None]
print("data", X.shape)

mu = X.mean(axis=0)
sd = X.std(axis=0)
sd[sd == 0] = 1.0
Xs = (X - mu) / sd

rng = np.random.default_rng(20261107)
perm = rng.permutation(len(Xs))
split = int(len(Xs) * 0.8)
tr, va = perm[:split], perm[split:]

W1 = rng.standard_normal((7, 32)) * 0.1
b1 = np.zeros((1, 32))
W2 = rng.standard_normal((32, 1)) * 0.1
b2 = np.zeros((1, 1))
params = [W1, b1, W2, b2]
m = [np.zeros_like(p) for p in params]
v = [np.zeros_like(p) for p in params]
lr, b1a, b2a, eps = 3e-3, 0.9, 0.999, 1e-8
step = 0


def forward(xb):
    h = np.tanh(xb @ W1 + b1)
    return h @ W2 + b2, h


for epoch in range(300):
    idx = rng.permutation(tr)
    for s in range(0, len(idx), 64):
        b = idx[s:s + 64]
        xb, yb = Xs[b], Y[b]
        out, h = forward(xb)
        err = out - yb
        dW2 = h.T @ err / len(b)
        db2 = err.mean(axis=0, keepdims=True)
        dh = (err @ W2.T) * (1 - h * h)
        dW1 = xb.T @ dh / len(b)
        db1 = dh.mean(axis=0, keepdims=True)

        grads = [dW1, db1, dW2, db2]
        step += 1
        for i, (p, g) in enumerate(zip(params, grads)):
            m[i] = b1a * m[i] + (1 - b1a) * g
            v[i] = b2a * v[i] + (1 - b2a) * (g * g)
            mh = m[i] / (1 - b1a ** step)
            vh = v[i] / (1 - b2a ** step)
            p -= lr * mh / (np.sqrt(vh) + eps)

train_loss = float(np.mean((forward(Xs[tr])[0] - Y[tr]) ** 2))
val_loss = float(np.mean((forward(Xs[va])[0] - Y[va]) ** 2))
pred = forward(Xs[va])[0].ravel()
ic = float(np.corrcoef(pred, Y[va].ravel())[0, 1])
print("train_loss %.6f  val_loss %.6f  IC %.4f" % (train_loss, val_loss, ic))
print("baseline(总是预测均值) val_loss %.6f" % float(np.mean((Y[va] - Y[tr].mean()) ** 2)))

np.savez(MODEL, W1=W1, b1=b1, W2=W2, b2=b2, mu=mu, sd=sd)
print("saved", MODEL)
