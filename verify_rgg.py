"""
验证 RGG 算法正确性 — 与 src/graphgenerator.cpp 中 generateRandomGeometric 同算法
跑两组用例 + 多次随机重复, 报告平均统计量
"""
import random, math
from collections import defaultdict

def rgg(n, r, seed=None):
    if seed is not None:
        random.seed(seed)
    pts = [(random.random(), random.random()) for _ in range(n)]
    edges = []
    r2 = r * r
    for i in range(n):
        xi, yi = pts[i]
        for j in range(i + 1, n):
            xj, yj = pts[j]
            dx, dy = xi - xj, yi - yj
            if dx * dx + dy * dy <= r2:
                edges.append((i, j))
    return pts, edges

def metrics(n, edges):
    deg = [0] * n
    nbrs = defaultdict(set)
    for a, b in edges:
        deg[a] += 1; deg[b] += 1
        nbrs[a].add(b); nbrs[b].add(a)

    # 连通分量 (DFS)
    visited = [False] * n
    comps, largest = 0, 0
    for s in range(n):
        if visited[s]: continue
        comps += 1
        stack, sz = [s], 0
        while stack:
            u = stack.pop()
            if visited[u]: continue
            visited[u] = True; sz += 1
            for w in nbrs[u]:
                if not visited[w]: stack.append(w)
        largest = max(largest, sz)

    # 聚类系数
    csum, cn = 0.0, 0
    for v in range(n):
        ns = list(nbrs[v])
        k = len(ns)
        if k < 2: continue
        tri = 0
        for i in range(k):
            for j in range(i+1, k):
                if ns[j] in nbrs[ns[i]]: tri += 1
        csum += tri / (k * (k - 1) / 2); cn += 1

    return {
        'edges': len(edges),
        'avg_deg': sum(deg) / n,
        'max_deg': max(deg),
        'isolated': sum(1 for d in deg if d == 0),
        'components': comps,
        'largest_comp': largest,
        'clustering': csum / cn if cn else 0.0,
    }

def run_case(n, r, trials=20):
    print(f"\n=== n={n}, r={r}  (理论 r_c = {math.sqrt(math.log(n)/(math.pi*n)):.4f}) ===")
    print(f"    预期边数 ~ C(n,2)·π·r² = {n*(n-1)/2 * math.pi * r * r:.1f}")
    print(f"    预期平均度 ~ (n-1)·π·r² = {(n-1)*math.pi*r*r:.2f}")

    agg = defaultdict(float)
    for t in range(trials):
        _, edges = rgg(n, r, seed=t)
        m = metrics(n, edges)
        for k, v in m.items(): agg[k] += v
    for k in agg: agg[k] /= trials

    print(f"    实测 ({trials}次平均):")
    print(f"      边数         = {agg['edges']:.1f}")
    print(f"      平均度       = {agg['avg_deg']:.2f}")
    print(f"      最大度       = {agg['max_deg']:.1f}")
    print(f"      孤立点数     = {agg['isolated']:.1f}")
    print(f"      连通分量数   = {agg['components']:.1f}")
    print(f"      最大分量大小 = {agg['largest_comp']:.1f}")
    print(f"      聚类系数     = {agg['clustering']:.4f}")

if __name__ == "__main__":
    print("RGG 算法验证 — 复刻 graphgenerator.cpp::generateRandomGeometric")
    run_case(100, 0.05)
    run_case(100, 0.20)
    run_case(100, 0.121)  # 接近理论阈值 r_c
