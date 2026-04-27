#ifndef GRAPHGENERATOR_H
#define GRAPHGENERATOR_H

#include "graph.h"

namespace GraphGenerator {

struct LayoutConfig {
    double radius = 0.0;        // 0 = 自动按 n 缩放
    QPointF center = QPointF(0, 0);
};

struct WeightConfig {
    bool randomWeights = false;
    int minWeight = 1;
    int maxWeight = 10;
};

// Erdős-Rényi G(n, p): n 个顶点，每条可能的边以概率 p 独立存在。
// 期望边数 = p * C(n,2)。
void generateErdosRenyiGnp(Graph *graph, int n, double p,
                            const WeightConfig &wcfg = {},
                            const LayoutConfig &lcfg = {});

// Erdős-Rényi G(n, m): 从 C(n,2) 条可能边里均匀随机选 m 条。
void generateErdosRenyiGnm(Graph *graph, int n, int m,
                            const WeightConfig &wcfg = {},
                            const LayoutConfig &lcfg = {});

// Random Geometric Graph (RGG): 在单位正方形 [0,1]^2 内均匀撒 n 个点,
//   两点欧氏距离 <= r 时连边.
//   连通性阈值: r_c ≈ sqrt(ln(n) / (π n)).
//   参考: Penrose, "Random Geometric Graphs", Oxford University Press, 2003.
void generateRandomGeometric(Graph *graph, int n, double r,
                              const WeightConfig &wcfg = {},
                              const LayoutConfig &lcfg = {});

// Barabási-Albert: 优先连接机制的无标度网络。
//   初始 m0 个顶点构成完全图；随后每加入 1 个新顶点就向 m 个已有顶点连边，
//   连接概率与已有顶点的度数成正比。
//   要求 2 <= m0 <= n, 1 <= m <= m0。
void generateBarabasiAlbert(Graph *graph, int n, int m, int m0 = -1,
                             const WeightConfig &wcfg = {},
                             const LayoutConfig &lcfg = {});

// Watts-Strogatz: 小世界网络。
//   从环形格子 (每顶点连 k 个最近邻, k 必须为偶数) 开始,
//   每条边以概率 p 被"重连"到一个随机新端点。
//   要求 k 为偶数, 0 < k < n-1, 0 <= p <= 1。
void generateWattsStrogatz(Graph *graph, int n, int k, double p,
                            const WeightConfig &wcfg = {},
                            const LayoutConfig &lcfg = {});

// 图指标 — 用于论文分析对比不同模型的结构差异。
struct GraphMetrics {
    int vertexCount = 0;
    int edgeCount = 0;
    double avgDegree = 0.0;
    int maxDegree = 0;
    int minDegree = 0;
    double clusteringCoefficient = 0.0;   // 仅对无向图有意义
    int componentCount = 0;               // 无向: 连通分量; 有向: 弱连通分量
    int largestComponentSize = 0;
};

GraphMetrics computeMetrics(const Graph *graph);

} // namespace GraphGenerator

#endif
