import numpy as np, os, io

BASE = os.path.expanduser(r"~\Desktop\C\stock-moon")
m = np.load(os.path.join(BASE, "_archive", "sim_model.npz"))
W1, b1, W2, b2, mu, sd = m["W1"], m["b1"], m["W2"], m["b2"], m["mu"], m["sd"]

def arr(name, a, fmt="%.6ff"):
    flat = np.asarray(a, dtype=np.float64).ravel()
    vals = ", ".join(fmt % v for v in flat)
    return "static const float %s[%d] = { %s };\n" % (name, flat.size, vals)

out = io.StringIO()
out.write("/* nn_weights.h —— 由 train_sim.py 训练后自动导出，7 -> 32 -> 1 */\n")
out.write("#pragma once\n\n")
out.write(arr("NN_W1", W1))
out.write(arr("NN_B1", b1))
out.write(arr("NN_W2", W2))
out.write(arr("NN_B2", b2))
out.write(arr("NN_MU", mu))
out.write(arr("NN_SD", sd))
io.open(os.path.join(BASE, "src", "nn_weights.h"), "w", encoding="gbk", newline="\n").write(out.getvalue())
print("wrote nn_weights.h", W1.shape, W2.shape)
