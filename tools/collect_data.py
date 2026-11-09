"""collect_data.py —— 通过接口跑一局，采集 (特征 -> 次日收益) 数据

每只股票每天一条样本，标签是它第二天的涨跌幅。
"""
import os, time, json, sys

BUILD = os.path.expanduser(r"~\Desktop\C\stock-moon\build")
STATE = os.path.join(BUILD, "ai_state.txt")
CMD = os.path.join(BUILD, "ai_cmd.txt")
OUT = os.path.expanduser(r"~\Desktop\C\stock-moon\dataset.csv")


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
            elif k == "mood":
                st["mood"] = float(p[1])
            elif k == "news" and len(p) >= 3:
                st["news"] = (int(p[1]), int(p[2]))
            elif k == "stock":
                st["stocks"][int(p[1])] = {"name": p[2], "price": int(p[3]),
                                           "change": int(p[4]), "shares": int(p[5])}
            elif k == "hist":
                st["hist"][int(p[1])] = [int(x) for x in p[2:]]
    return st if "day" in st else None


def slope(xs, n=10):
    v = xs[-n:]
    m = len(v)
    if m < 2:
        return 0.0
    sx = sum(range(m)); sy = sum(v)
    sxx = sum(i * i for i in range(m)); sxy = sum(i * v[i] for i in range(m))
    den = m * sxx - sx * sx
    return 0.0 if den == 0 else (m * sxy - sx * sy) / den


def features(st, i):
    s = st["stocks"][i]
    xs = st["hist"].get(i, [])
    price = max(1, s["price"])
    base = 100 + i * 17
    avg20 = sum(xs[-20:]) / max(1, len(xs[-20:]))
    k = slope(xs)
    news = 0.0
    if st.get("news") and st["news"][0] == i:
        news = st["news"][1] / 25.0
    return [
        price / base,
        s["change"] / 20.0,
        k / price,
        price / avg20 - 1.0 if avg20 else 0.0,
        st.get("mood", 0.0),
        news,
        min(len(xs), 20) / 20.0,
    ]


def main():
    rows = []
    prev = None
    st = read_state()
    if not st:
        print("no state")
        return 1

    while st and st["day"] <= 365:
        with open(CMD, "w", encoding="ascii") as f:
            f.write("next")

        day = st["day"]
        t0 = time.time()
        while time.time() - t0 < 3:
            st2 = read_state()
            if st2 and st2["day"] > day:
                break
            time.sleep(0.02)
        if not st2 or st2["day"] <= day:
            break

        for i, s in st["stocks"].items():
            p0 = s["price"]
            if i in st2["stocks"] and p0 > 0:
                p1 = st2["stocks"][i]["price"]
                rows.append(features(st, i) + [(p1 - p0) / p0])

        st = st2
        if st["day"] % 60 == 1:
            print("collected day", st["day"], "rows", len(rows))

    with open(OUT, "w", encoding="utf-8") as f:
        for r in rows:
            f.write(",".join("%.6f" % v for v in r) + "\n")
    print("wrote", OUT, "rows", len(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
