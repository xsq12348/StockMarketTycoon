"""play_nn.py —— 用训练好的 MLP 预测次日涨跌并交易"""
import os, time, sys
import numpy as np

BASE = os.path.expanduser(r"~\Desktop\C\stock-moon")
BUILD = os.path.join(BASE, "build")
STATE = os.path.join(BUILD, "ai_state.txt")
CMD = os.path.join(BUILD, "ai_cmd.txt")

m = np.load(os.path.join(BASE, "model.npz"))
W1, b1, W2, b2, mu, sd = m["W1"], m["b1"], m["W2"], m["b2"], m["mu"], m["sd"]


def predict(x):
    x = (np.array(x) - mu) / sd
    h = np.tanh(x @ W1 + b1)
    return float((h @ W2 + b2).ravel()[0])


def read_state():
    if not os.path.exists(STATE):
        return None
    st = {"stocks": {}, "hist": {}, "mood": 0.0, "news": None}
    with open(STATE, "r", encoding="utf-8", errors="ignore") as f:
        for ln in f:
            p = ln.split()
            if not p:
                continue
            k = p[0]
            if k == "day":
                st["day"] = int(p[1])
            elif k == "cash":
                st["cash"] = int(p[1])
            elif k == "net":
                st["net"] = int(p[1])
            elif k == "mood":
                st["mood"] = float(p[1])
            elif k == "news" and len(p) >= 3:
                st["news"] = (int(p[1]), int(p[2]))
            elif k == "stock":
                st["stocks"][int(p[1])] = {"name": p[2], "price": int(p[3]),
                                           "change": int(p[4]), "shares": int(p[5])}
            elif k == "hist":
                st["hist"][int(p[1])] = [int(x) for x in p[2:]]
            elif k == "npc":
                st.setdefault("npc", []).append((" ".join(p[2:-1]), int(p[-1])))
    return st if "day" in st else None


def slope(xs, n=10):
    v = xs[-n:]
    mm = len(v)
    if mm < 2:
        return 0.0
    sx = sum(range(mm)); sy = sum(v)
    sxx = sum(i * i for i in range(mm)); sxy = sum(i * v[i] for i in range(mm))
    den = mm * sxx - sx * sx
    return 0.0 if den == 0 else (mm * sxy - sx * sy) / den


def feats(st, i):
    s = st["stocks"][i]
    xs = st["hist"].get(i, [])
    price = max(1, s["price"])
    base = 100 + i * 17
    avg20 = sum(xs[-20:]) / max(1, len(xs[-20:]))
    news = 0.0
    if st.get("news") and st["news"][0] == i:
        news = st["news"][1] / 25.0
    return [price / base, s["change"] / 20.0, slope(xs) / price,
            price / avg20 - 1.0 if avg20 else 0.0, st.get("mood", 0.0), news,
            min(len(xs), 20) / 20.0]


def wait_day(prev, timeout=3.0):
    t0 = time.time()
    while time.time() - t0 < timeout:
        st = read_state()
        if st and st["day"] > prev:
            return st
        time.sleep(0.02)
    return read_state()


def main():
    st = read_state()
    if not st:
        print("no state")
        return 1

    max_pos = 3
    while st and st["day"] <= 365:
        cash = st.get("cash", 0)
        cmds = []
        scored = []

        for i, s in st["stocks"].items():
            pr = predict(feats(st, i))
            scored.append((pr, i, s["price"], s["shares"]))

        for pr, i, price, shares in scored:
            if shares > 0 and pr < 0:
                cmds.append("sell %d %d" % (i, shares))

        scored.sort(reverse=True)
        held = [1 for pr, i, price, shares in scored if shares > 0]
        slots = max_pos - len(held)

        if slots > 0 and cash > 0:
            budget = cash // slots
            for pr, i, price, shares in scored[:slots]:
                if pr > 0 and shares == 0:
                    qty = budget // price
                    if qty > 0:
                        cmds.append("buy %d %d" % (i, qty))

        cmds.append("next")
        with open(CMD, "w", encoding="ascii") as f:
            f.write("\n".join(cmds))

        prev = st["day"]
        st = wait_day(prev)
        if not st:
            break
        if st["day"] % 60 == 1:
            print("day %3d  cash %7d  net %7d" % (st["day"], st["cash"], st["net"]))

    st = read_state()
    print("FINAL day %s  cash %s  net %s" % (st["day"], st["cash"], st["net"]))
    for name, worth in st.get("npc", []):
        print("  %-12s %d" % (name, worth))
    return 0


if __name__ == "__main__":
    sys.exit(main())
