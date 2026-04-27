#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "algorithmbase.h"
#include <QStack>
#include <QQueue>
#include <QSet>
#include <QMap>

// ==================== DFS ====================
class DFSAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit DFSAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("DFS (深度优先搜索)"); }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

private:
    QStack<int> m_stack;
    QList<int> m_visitOrder;
    bool m_initialized;
};

// ==================== BFS ====================
class BFSAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit BFSAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("BFS (广度优先搜索)"); }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

private:
    QQueue<int> m_queue;
    QList<int> m_visitOrder;
    bool m_initialized;
};

// ==================== Dijkstra ====================
class DijkstraAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit DijkstraAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("Dijkstra (最短路径)"); }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

    QMap<int, int> distances() const { return m_dist; }
    QMap<int, int> previous() const { return m_prev; }

private:
    void buildResultPath();

    QSet<int> m_unvisited;
    QMap<int, int> m_dist;
    QMap<int, int> m_prev;
    bool m_initialized;
};

// ==================== Prim ====================
class PrimAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit PrimAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("Prim (最小生成树)"); }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

private:
    QSet<int> m_inTree;
    QList<int> m_inTreeOrder;  // 按加入顺序记录顶点
    QList<Edge> m_treeEdges;
    int m_totalWeight;
    bool m_initialized;
};

// ==================== Kruskal ====================
class KruskalAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit KruskalAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("Kruskal (最小生成树)"); }
    bool needsStartVertex() const override { return false; }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

private:
    int find(int x);
    int findRoot(int x) const;  // 不做路径压缩的查找，用于const显示
    void unite(int a, int b);

    QList<Edge> m_sortedEdges;
    int m_edgeIndex;
    QMap<int, int> m_parent;
    QMap<int, int> m_rank;
    QList<Edge> m_treeEdges;
    int m_totalWeight;
    bool m_initialized;
};

// ==================== 拓扑排序 ====================
class TopologicalSortAlgorithm : public AlgorithmBase {
    Q_OBJECT
public:
    explicit TopologicalSortAlgorithm(Graph *graph, QObject *parent = nullptr);
    QString name() const override { return QStringLiteral("拓扑排序"); }
    bool needsStartVertex() const override { return false; }
    bool step() override;
    void reset() override;
    QStringList dataDisplayLines() const override;

private:
    QMap<int, int> m_inDegree;
    QQueue<int> m_queue;
    QList<int> m_result;
    bool m_initialized;
    bool m_hasCycle;
};

#endif // ALGORITHMS_H
