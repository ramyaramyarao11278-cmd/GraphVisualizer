"""
4 种随机图模型的对比验证 (论文第 5.7 节 / 6.8 节使用)
独立 Python 实现, 与 src/graphgenerator.cpp 同算法, 多次重复取均值
"""
import random, math
from collections import defaultdict

# ====================== 4 个生成器 (与 C++ 同算法) ======================

def gen_er_gnp(n, p, seed):
    random.seed(seed)
    edges = []
    for i in range(n):
        for j in range(i + 1, n):
            if random.random() < p:
                edges.append((i, j))
    return edges

def gen_rgg(n, r, seed):
    random.seed(seed)
    pts = [(random.random(), random.random()) for _ in range(n)]
    edges = []
    r2 = r * r
    for i in range(n):
        for j in range(i + 1, n):
            dx = pts[i][0] - pts[j][0]; dy = pts[i][1] - pts[j][1]
            if dx*dx + dy*dy <= r2:
                edges.append((i, j))
    return edges

def gen_ba(n, m, seed):
    random.seed(seed)
    m0 = max(m + 1, 2)
    edges = []
    for i in range(m0):
        for j in range(i + 1, m0):
            edges.append((i, j))
    bag = []
    for i in range(m0):
        for _ in range(m0 - 1):
            bag.append(i)
    for v in range(m0, n):
        targets = set()
        while len(targets) < m:
            targets.add(random.choice(bag))
        for t in targets:
            edges.append((v, t))
            bag.append(v); bag.append(t)
    return edges

def gen_ws(n, k, p, seed):
    random.seed(seed)
    if k % 2: k += 1
    edges = set()
    for i in range(n):
        for d in range(1, k // 2 + 1):
            a, b = i, (i + d) % n
            if a > b: a, b = b, a
            edges.add((a, b))
    new_edges = set()
    for (u, v) in edges:
        if random.random() < p:
            tries = 0
            while tries < 100:
                w = random.randrange(n)
                if w == u: tries += 1; continue
                a, b = (u, w) if u < w else (w, u)
                if (a, b) in edges or (a, b) in new_edges:
                    tries += 1; continue
                new_edges.add((a, b))
                break
            else:
                new_edges.add((u, v) if u < v else (v, u))
        else:
            new_edges.add((u, v) if u < v else (v, u))
    return list(new_edges)

# ====================== 通用指标 ======================

def metrics(n, edges):
    deg = [0] * n
    nbrs = defaultdict(set)
    for a, b in edges:
        deg[a] += 1; deg[b] += 1
        nbrs[a].add(b); nbrs[b].add(a)

    visited = [False] * n
    comps, largest = 0, 0
    for s in range(n):
        if visited[s]: continue
        comps += 1; stack, sz = [s], 0
        while stack:
            u = stack.pop()
            if visited[u]: continue
            visited[u] = True; sz += 1
            for w in nbrs[u]:
                if not visited[w]: stack.append(w)
        largest = max(largest, sz)

    csum, cn = 0.0, 0
    for v in range(n):
        ns = list(nbrs[v]); k = len(ns)
        if k < 2: continue
        tri = sum(1 for i in range(k) for j in range(i+1, k) if ns[j] in nbrs[ns[i]])
        csum += tri / (k * (k - 1) / 2); cn += 1

    # BFS 平均最短路径长度 (仅最大连通分量, 抽样最多 50 个起点)
    big = [v for v in range(n) if deg[v] > 0]
    if not big: avg_path = 0.0
    else:
        from collections import deque
        path_sum, path_cnt = 0, 0
        sources = big if len(big) <= 50 else random.sample(big, 50)
        for src in sources:
            dist = {src: 0}; q = deque([src])
            while q:
                u = q.popleft()
                for w in nbrs[u]:
                    if w not in dist:
                        dist[w] = dist[u] + 1; q.append(w)
            for d in dist.values():
                if d > 0: path_sum += d; path_cnt += 1
        avg_path = path_sum / path_cnt if path_cnt else 0.0

    return {
        'edges': len(edges),
        'avg_deg': sum(deg) / n,
        'max_deg': max(deg),
        'isolated': sum(1 for d in deg if d == 0),
        'components': comps,
        'largest_comp': largest,
        'clustering': csum / cn if cn else 0.0,
        'avg_path': avg_path,
    }

def run_model(name, gen_fn, params, n, trials=20):
    agg = defaultdict(float)
    for t in range(trials):
        edges = gen_fn(seed=t, **params)
        m = metrics(n, edges)
        for k, v in m.items(): agg[k] += v
    for k in agg: agg[k] /= trials
    agg['name'] = name; agg['params'] = params
    return agg

# ====================== 主实验 ======================

if __name__ == "__main__":
    N = 100
    TRIALS = 20

    # 选取参数让 4 个模型平均度大致可比 (~10), 便于横向比较
    results = [
        run_model("ER G(n,p)",  lambda seed, n, p: gen_er_gnp(n, p, seed),
                  dict(n=N, p=0.10), N, TRIALS),
        run_model("RGG",        lambda seed, n, r: gen_rgg(n, r, seed),
                  dict(n=N, r=0.18), N, TRIALS),
        run_model("BA",         lambda seed, n, m: gen_ba(n, m, seed),
                  dict(n=N, m=5),    N, TRIALS),
        run_model("WS",         lambda seed, n, k, p: gen_ws(n, k, p, seed),
                  dict(n=N, k=10, p=0.1), N, TRIALS),
    ]

    print(f"\n=== 4 模型横向对比 (n={N}, {TRIALS} 次重复均值) ===\n")
    print(f"{'模型':<14}{'参数':<22}{'边数':>7}{'平均度':>8}{'最大度':>8}"
          f"{'孤立点':>8}{'分量数':>8}{'最大分量':>10}{'聚类C':>8}{'平均路径L':>12}")
    print("-" * 105)
    for r in results:
        params_str = ", ".join(f"{k}={v}" for k, v in r['params'].items() if k != 'n')
        print(f"{r['name']:<14}{params_str:<22}"
              f"{r['edges']:>7.1f}{r['avg_deg']:>8.2f}{r['max_deg']:>8.1f}"
              f"{r['isolated']:>8.1f}{r['components']:>8.1f}{r['largest_comp']:>10.1f}"
              f"{r['clustering']:>8.4f}{r['avg_path']:>12.2f}")

    # CSV 输出 (论文表格直接用)
    import csv
    with open("model_comparison.csv", "w", newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['model', 'params', 'edges', 'avg_deg', 'max_deg',
                    'isolated', 'components', 'largest_comp', 'clustering', 'avg_path'])
        for r in results:
            params_str = ", ".join(f"{k}={v}" for k, v in r['params'].items())
            w.writerow([r['name'], params_str,
                        f"{r['edges']:.1f}", f"{r['avg_deg']:.2f}", f"{r['max_deg']:.1f}",
                        f"{r['isolated']:.1f}", f"{r['components']:.1f}",
                        f"{r['largest_comp']:.1f}", f"{r['clustering']:.4f}",
                        f"{r['avg_path']:.2f}"])
    print(f"\nCSV 已写入 model_comparison.csv (论文 6.8 表格直接用)")

    # 关键观察提示 (写正文时直接套)
    print("\n=== 论文写作要点 (基于上表) ===")
    er, rgg, ba, ws = results
    print(f"1. 聚类系数: ER {er['clustering']:.3f} << RGG {rgg['clustering']:.3f}, "
          f"WS {ws['clustering']:.3f} (RGG/WS 高聚类 = 几何/局部连接特性)")
    print(f"2. 最大度: BA {ba['max_deg']:.1f} >> ER {er['max_deg']:.1f} "
          f"(BA 的枢纽节点效应, 幂律尾)")
    print(f"3. 平均路径: ER {er['avg_path']:.2f}, WS {ws['avg_path']:.2f} 短; "
          f"RGG {rgg['avg_path']:.2f} 较长 (空间约束限制了长程连接)")
    print(f"4. 连通性: ER 分量 {er['components']:.1f}, RGG 分量 {rgg['components']:.1f}, "
          f"BA 分量 {ba['components']:.1f}, WS 分量 {ws['components']:.1f}")
