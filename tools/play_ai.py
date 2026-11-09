"""play_ai.py —— 通过 ai_state.txt / ai_cmd.txt 玩一整局 Moon Stocks

策略：
  情绪（mood）决定仓位：mood < -0.15 清仓避险，mood > 0.05 才建仓
  个股：10 日斜率为正、且价格不高于 20 日均价 3%（回调买入）
  满仓 3 只，斜率为负或 mood 转空就卖
"""
import os, time, sys

BUILD = os.path.expanduser(r"~\Desktop\C\stock-moon\build")
STATE = os.path.join(BUILD, "ai_state.txt")
CMD = os.path.join(BUILD, "ai_cmd.txt")


def read_state():
    if not os.path.exists(STATE):
        return None
    st = {"stocks": {}, "npc": [], "hist": {}, "mood": 0.0}
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
            elif k == "stock":
                st["stocks"][int(p[1])] = {"name": p[2], "price": int(p[3]),
                                           "change": int(p[4]), "shares": int(p[5])}
            elif k == "hist":
                st["hist"][int(p[1])] = [int(x) for x in p[2:]]
            elif k == "npc":
                st["npc"].append((" ".join(p[2:-1]), int(p[-1])))
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


def avg(xs, n=20):
    v = xs[-n:]
    return sum(v) / len(v) if v else 0


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
        mood = st.get("mood", 0.0)
        cash = st.get("cash", 0)
        held = [i for i, s in st["stocks"].items() if s["shares"] > 0]

        cmds = []
        for i, s in st["stocks"].items():
            xs = st["hist"].get(i, [])
            k = slope(xs)
            a = avg(xs)

            if s["shares"] > 0:
                if k < 0 or mood < -0.15:
                    cmds.append("sell %d %d" % (i, s["shares"]))

        cand = []
        for i, s in st["stocks"].items():
            if s["shares"] > 0:
                continue
            xs = st["hist"].get(i, [])
            k = slope(xs)
            a = avg(xs)
            if k > 0 and s["price"] <= a * 1.03:
                cand.append((k, i, s["price"]))

        cand.sort(reverse=True)
        slots = max_pos - len(held)
        if mood > 0.05 and slots > 0 and cand and cash > 0:
            budget = cash // slots
            for k, i, price in cand[:slots]:
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
            print("day %3d  cash %7d  net %7d  mood %+0.2f" % (st["day"], st["cash"], st["net"], st["mood"]))

    st = read_state()
    if not st:
        print("state lost")
        return 1
    print("FINAL day %s  cash %s  net %s  mood %+0.2f" % (st["day"], st["cash"], st["net"], st["mood"]))
    for name, worth in st["npc"]:
        print("  %-12s %d" % (name, worth))
    return 0


if __name__ == "__main__":
    sys.exit(main())
