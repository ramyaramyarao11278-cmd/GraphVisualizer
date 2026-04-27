"""
论文配图生成 (输出 PDF + PNG, 300 DPI)
图 1: 4 模型度分布对比
图 2: RGG 连通性相变曲线 (扫 r)
图 3: WS 小世界曲线 (C(p)/L(p))
图 4: BA 度分布双对数图 (验证幂律)
"""
import os, math, random
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from collections import Counter, defaultdict, deque

from verify_models import gen_er_gnp, gen_rgg, gen_ba, gen_ws, metrics

OUT = "figures"
os.makedirs(OUT, exist_ok=True)
plt.rcParams.update({
    'font.size': 11, 'figure.dpi': 100, 'savefig.dpi': 300,
    'font.sans-serif': ['Microsoft YaHei', 'SimHei', 'DejaVu Sans'],
    'axes.unicode_minus': False,
})

def degree_seq(n, edges):
    deg = [0] * n
    for a, b in edges:
        deg[a] += 1; deg[b] += 1
    return deg

# ====================== 图 1: 4 模型度分布 ======================
def fig_degree_distributions():
    n, trials = 200, 5
    configs = [
        ("ER G(n,p=0.05)", lambda s: gen_er_gnp(n, 0.05, s)),
        ("RGG r=0.15",      lambda s: gen_rgg(n, 0.15, s)),
        ("BA m=3",          lambda s: gen_ba(n, 3, s)),
        ("WS k=10, p=0.1",  lambda s: gen_ws(n, 10, 0.1, s)),
    ]
    fig, axes = plt.subplots(2, 2, figsize=(10, 7))
    for ax, (name, gen) in zip(axes.flat, configs):
        all_deg = []
        for s in range(trials):
            all_deg.extend(degree_seq(n, gen(s)))
        ax.hist(all_deg, bins=range(0, max(all_deg)+2), edgecolor='black',
                color='steelblue', alpha=0.8)
        ax.set_title(name); ax.set_xlabel('度 k'); ax.set_ylabel('顶点数')
        ax.grid(alpha=0.3)
    fig.suptitle(f'四种随机图模型的度分布 (n={n}, {trials} 次合并)', fontsize=13)
    fig.tight_layout()
    for ext in ('pdf', 'png'):
        fig.savefig(f"{OUT}/fig1_degree_distributions.{ext}", bbox_inches='tight')
    plt.close(fig)
    print(f"[OK] {OUT}/fig1_degree_distributions.{{pdf,png}}")

# ====================== 图 2: RGG 连通性相变 ======================
def fig_rgg_phase_transition():
    n, trials = 100, 30
    rs = [0.02 * i for i in range(1, 16)]
    rc = math.sqrt(math.log(n) / (math.pi * n))
    largest, comps = [], []
    for r in rs:
        lg, cp = 0, 0
        for s in range(trials):
            edges = gen_rgg(n, r, s)
            m = metrics(n, edges)
            lg += m['largest_comp']; cp += m['components']
        largest.append(lg / trials / n)
        comps.append(cp / trials)

    fig, ax1 = plt.subplots(figsize=(8, 5))
    color1 = 'tab:blue'
    ax1.plot(rs, largest, 'o-', color=color1, linewidth=2, markersize=6,
             label='最大连通分量占比')
    ax1.set_xlabel('连接半径 r')
    ax1.set_ylabel('最大连通分量 / n', color=color1)
    ax1.tick_params(axis='y', labelcolor=color1)
    ax1.axvline(rc, color='red', linestyle='--', alpha=0.7,
                label=f'理论阈值 r_c≈{rc:.3f}')
    ax1.grid(alpha=0.3)

    ax2 = ax1.twinx()
    color2 = 'tab:orange'
    ax2.plot(rs, comps, 's--', color=color2, linewidth=2, markersize=5,
             label='连通分量总数')
    ax2.set_ylabel('连通分量数', color=color2)
    ax2.tick_params(axis='y', labelcolor=color2)

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc='center right')
    plt.title(f'RGG 连通性相变 (n={n}, {trials} 次重复)')
    fig.tight_layout()
    for ext in ('pdf', 'png'):
        fig.savefig(f"{OUT}/fig2_rgg_phase_transition.{ext}", bbox_inches='tight')
    plt.close(fig)
    print(f"[OK] {OUT}/fig2_rgg_phase_transition.{{pdf,png}}")

# ====================== 图 3: WS 小世界曲线 C(p), L(p) ======================
def fig_ws_small_world():
    n, k, trials = 100, 6, 5
    ps = [0] + [10 ** (-3 + 0.25 * i) for i in range(13)]  # 0, 1e-3 ~ 1
    C, L = [], []
    # 归一化用 p=0 (规则图) 的值
    edges0 = gen_ws(n, k, 0, 0)
    m0 = metrics(n, edges0)
    C0, L0 = m0['clustering'], m0['avg_path']
    for p in ps:
        cs, ls = 0.0, 0.0
        for s in range(trials):
            edges = gen_ws(n, k, p, s)
            m = metrics(n, edges)
            cs += m['clustering']; ls += m['avg_path']
        C.append(cs / trials / C0); L.append(ls / trials / L0)

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.semilogx([p if p > 0 else 1e-4 for p in ps], C, 'o-', label='C(p) / C(0)',
                color='tab:blue', linewidth=2, markersize=7)
    ax.semilogx([p if p > 0 else 1e-4 for p in ps], L, 's-', label='L(p) / L(0)',
                color='tab:orange', linewidth=2, markersize=7)
    ax.axvspan(0.01, 0.1, alpha=0.15, color='green', label='小世界区间')
    ax.set_xlabel('重连概率 p')
    ax.set_ylabel('归一化值')
    ax.set_title(f'Watts-Strogatz 小世界特性 (n={n}, k={k}, {trials} 次重复)')
    ax.legend(); ax.grid(alpha=0.3, which='both')
    fig.tight_layout()
    for ext in ('pdf', 'png'):
        fig.savefig(f"{OUT}/fig3_ws_small_world.{ext}", bbox_inches='tight')
    plt.close(fig)
    print(f"[OK] {OUT}/fig3_ws_small_world.{{pdf,png}}")

# ====================== 图 4: BA 度分布双对数 (验证幂律) ======================
def fig_ba_power_law():
    n, m, trials = 1000, 3, 10
    deg_count = Counter()
    for s in range(trials):
        edges = gen_ba(n, m, s)
        for d in degree_seq(n, edges):
            if d > 0: deg_count[d] += 1
    ks = sorted(deg_count)
    pk = [deg_count[k] / (n * trials) for k in ks]

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.loglog(ks, pk, 'o', color='tab:purple', markersize=6, label='实测 P(k)')
    # 理论参考线: P(k) ~ k^{-3}
    if ks:
        kref = [k for k in ks if k >= m]
        c = pk[0] * ks[0]**3 if pk[0] > 0 else 1
        ref = [c * k**(-3) for k in kref]
        ax.loglog(kref, ref, '--', color='red', linewidth=2, label='理论 k^-3')
    ax.set_xlabel('度 k (log)')
    ax.set_ylabel('P(k) (log)')
    ax.set_title(f'BA 模型度分布 (n={n}, m={m}, {trials} 次合并)')
    ax.legend(); ax.grid(alpha=0.3, which='both')
    fig.tight_layout()
    for ext in ('pdf', 'png'):
        fig.savefig(f"{OUT}/fig4_ba_power_law.{ext}", bbox_inches='tight')
    plt.close(fig)
    print(f"[OK] {OUT}/fig4_ba_power_law.{{pdf,png}}")

if __name__ == "__main__":
    print("生成论文配图...")
    fig_degree_distributions()
    fig_rgg_phase_transition()
    fig_ws_small_world()
    fig_ba_power_law()
    print(f"\n全部完成. 4 张图保存在 {OUT}/ 目录, 每张含 PDF 和 PNG 两种格式.")
