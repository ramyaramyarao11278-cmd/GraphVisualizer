#include "algorithms.h"
#include <algorithm>
#include <climits>

// ==================== DFS ====================

DFSAlgorithm::DFSAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_initialized(false) {}

void DFSAlgorithm::reset()
{
    m_stack.clear();
    m_visitedVertices.clear();
    m_visitOrder.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
}

bool DFSAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;
        m_stack.push(m_startVertex);
        m_log.append(QStringLiteral("初始化: 起点 %1 入栈").arg(
            m_graph->vertex(m_startVertex).label));
        emit stateChanged();
        return true;
    }

    while (!m_stack.isEmpty()) {
        int u = m_stack.pop();
        if (m_visitedVertices.contains(u)) {
            continue; // 跳过已访问，继续弹栈
        }

        m_visitedVertices.insert(u);
        m_visitOrder.append(u);
        m_highlightedVertices = {u};
        m_highlightedEdges.clear();

        m_log.append(QStringLiteral("访问 %1").arg(m_graph->vertex(u).label));

        auto adj = m_graph->adjacency(u);
        for (int i = adj.size() - 1; i >= 0; --i) {
            int v = adj[i].to;
            if (!m_visitedVertices.contains(v)) {
                m_stack.push(v);
                m_highlightedEdges.append({u, v});
                m_log.append(QStringLiteral("  邻居 %1 入栈").arg(
                    m_graph->vertex(v).label));
            }
        }

        emit stateChanged();
        return true;
    }

    // 完成
    m_finished = true;
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultVertices = m_visitOrder;

    QStringList labels;
    for (int id : m_visitOrder)
        labels << m_graph->vertex(id).label;
    QString summary = QStringLiteral("DFS 完成，访问顺序: %1").arg(labels.join(QStringLiteral(" → ")));
    emit finished(summary);
    return false;
}

QStringList DFSAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 栈内容 (顶→底) ===");
    for (int i = m_stack.size() - 1; i >= 0; --i)
        lines << QStringLiteral("  %1").arg(m_graph->vertex(m_stack.at(i)).label);
    if (m_stack.isEmpty()) lines << QStringLiteral("  (空)");

    lines << QStringLiteral("");
    lines << QStringLiteral("=== 已访问顺序 ===");
    QStringList visited;
    for (int id : m_visitOrder)
        visited << m_graph->vertex(id).label;
    lines << QStringLiteral("  %1").arg(visited.isEmpty() ? QStringLiteral("(无)") : visited.join(QStringLiteral(" → ")));
    return lines;
}

// ==================== BFS ====================

BFSAlgorithm::BFSAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_initialized(false) {}

void BFSAlgorithm::reset()
{
    m_queue.clear();
    m_visitedVertices.clear();
    m_visitOrder.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
}

bool BFSAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;
        m_queue.enqueue(m_startVertex);
        m_visitedVertices.insert(m_startVertex);
        m_log.append(QStringLiteral("初始化: 起点 %1 入队").arg(
            m_graph->vertex(m_startVertex).label));
        emit stateChanged();
        return true;
    }

    if (m_queue.isEmpty()) {
        m_finished = true;
        m_highlightedVertices.clear();
        m_highlightedEdges.clear();
        m_resultVertices = m_visitOrder;

        QStringList labels;
        for (int id : m_visitOrder)
            labels << m_graph->vertex(id).label;
        emit finished(QStringLiteral("BFS 完成，访问顺序: %1").arg(labels.join(QStringLiteral(" → "))));
        return false;
    }

    int u = m_queue.dequeue();
    m_visitOrder.append(u);
    m_highlightedVertices = {u};
    m_highlightedEdges.clear();

    m_log.append(QStringLiteral("访问 %1").arg(m_graph->vertex(u).label));

    for (auto &e : m_graph->adjacency(u)) {
        int v = e.to;
        if (!m_visitedVertices.contains(v)) {
            m_visitedVertices.insert(v);
            m_queue.enqueue(v);
            m_highlightedEdges.append({u, v});
            m_resultEdges.append({u, v}); // BFS树边
            m_log.append(QStringLiteral("  邻居 %1 入队").arg(
                m_graph->vertex(v).label));
        }
    }

    emit stateChanged();
    return true;
}

QStringList BFSAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 队列 (前→后) ===");
    QStringList queueLabels;
    for (int i = 0; i < m_queue.size(); ++i)
        queueLabels << m_graph->vertex(m_queue.at(i)).label;
    lines << QStringLiteral("  %1").arg(queueLabels.isEmpty() ? QStringLiteral("(空)") : queueLabels.join(QStringLiteral(", ")));

    lines << QStringLiteral("");
    lines << QStringLiteral("=== 已访问顺序 ===");
    QStringList visited;
    for (int id : m_visitOrder)
        visited << m_graph->vertex(id).label;
    lines << QStringLiteral("  %1").arg(visited.isEmpty() ? QStringLiteral("(无)") : visited.join(QStringLiteral(" → ")));
    return lines;
}

// ==================== Dijkstra ====================

DijkstraAlgorithm::DijkstraAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_initialized(false) {}

void DijkstraAlgorithm::reset()
{
    m_unvisited.clear();
    m_dist.clear();
    m_prev.clear();
    m_visitedVertices.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
}

bool DijkstraAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;
        for (int v : m_graph->vertices()) {
            m_dist[v] = (v == m_startVertex) ? 0 : INT_MAX;
            m_prev[v] = -1;
            m_unvisited.insert(v);
        }
        m_log.append(QStringLiteral("初始化: dist[%1]=0, 其余=∞").arg(
            m_graph->vertex(m_startVertex).label));
        emit stateChanged();
        return true;
    }

    if (m_unvisited.isEmpty()) {
        m_finished = true;
        buildResultPath();
        emit finished(QStringLiteral("Dijkstra 完成"));
        return false;
    }

    int u = -1;
    int minDist = INT_MAX;
    for (int v : m_unvisited) {
        if (m_dist[v] < minDist) {
            minDist = m_dist[v];
            u = v;
        }
    }

    if (u == -1 || minDist == INT_MAX) {
        m_finished = true;
        buildResultPath();
        emit finished(QStringLiteral("Dijkstra 完成 (部分顶点不可达)"));
        return false;
    }

    m_unvisited.remove(u);
    m_visitedVertices.insert(u);
    m_highlightedVertices = {u};
    m_highlightedEdges.clear();

    m_log.append(QStringLiteral("选取 %1 (dist=%2)").arg(
        m_graph->vertex(u).label).arg(m_dist[u]));

    for (auto &e : m_graph->adjacency(u)) {
        int v = e.to;
        if (m_unvisited.contains(v)) {
            int newDist = m_dist[u] + e.weight;
            if (newDist < m_dist[v]) {
                m_dist[v] = newDist;
                m_prev[v] = u;
                m_highlightedEdges.append({u, v});
                m_log.append(QStringLiteral("  更新 dist[%1] = %2").arg(
                    m_graph->vertex(v).label).arg(newDist));
            }
        }
    }

    emit stateChanged();
    return true;
}

void DijkstraAlgorithm::buildResultPath()
{
    // 根据prev数组构建所有最短路径树边
    m_resultEdges.clear();
    m_resultVertices.clear();
    for (int v : m_graph->vertices()) {
        if (m_prev[v] != -1) {
            m_resultEdges.append({m_prev[v], v});
            if (!m_resultVertices.contains(m_prev[v]))
                m_resultVertices.append(m_prev[v]);
            if (!m_resultVertices.contains(v))
                m_resultVertices.append(v);
        }
    }
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
}

QStringList DijkstraAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 距离表 ===");
    lines << QStringLiteral("  顶点\t距离\t前驱\t已确定");
    for (int v : m_graph->vertices()) {
        QString distStr = (m_dist.value(v, INT_MAX) == INT_MAX)
            ? QStringLiteral("∞") : QString::number(m_dist[v]);
        QString prevStr = (m_prev.value(v, -1) == -1)
            ? QStringLiteral("-") : m_graph->vertex(m_prev[v]).label;
        QString visitedStr = m_visitedVertices.contains(v)
            ? QStringLiteral("是") : QStringLiteral("否");
        lines << QStringLiteral("  %1\t%2\t%3\t%4").arg(
            m_graph->vertex(v).label, distStr, prevStr, visitedStr);
    }
    return lines;
}

// ==================== Prim ====================

PrimAlgorithm::PrimAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_totalWeight(0), m_initialized(false) {}

void PrimAlgorithm::reset()
{
    m_inTree.clear();
    m_inTreeOrder.clear();
    m_treeEdges.clear();
    m_totalWeight = 0;
    m_visitedVertices.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
}

bool PrimAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;
        if (m_startVertex < 0 && !m_graph->vertices().isEmpty())
            m_startVertex = m_graph->vertices().first();
        m_inTree.insert(m_startVertex);
        m_inTreeOrder.append(m_startVertex);
        m_visitedVertices.insert(m_startVertex);
        m_highlightedVertices = {m_startVertex};
        m_log.append(QStringLiteral("初始化: %1 加入生成树").arg(
            m_graph->vertex(m_startVertex).label));
        emit stateChanged();
        return true;
    }

    if (m_inTree.size() >= m_graph->vertexCount()) {
        m_finished = true;
        m_highlightedVertices.clear();
        m_highlightedEdges.clear();
        // 结果：所有树边
        for (auto &e : m_treeEdges) {
            m_resultEdges.append({e.from, e.to});
        }
        m_resultVertices = m_inTree.values();
        emit finished(QStringLiteral("Prim 完成，总权值: %1").arg(m_totalWeight));
        return false;
    }

    Edge bestEdge{-1, -1, INT_MAX};
    for (int u : m_inTree) {
        for (auto &e : m_graph->adjacency(u)) {
            if (!m_inTree.contains(e.to) && e.weight < bestEdge.weight) {
                bestEdge = e;
            }
        }
    }

    if (bestEdge.from == -1) {
        m_finished = true;
        for (auto &e : m_treeEdges)
            m_resultEdges.append({e.from, e.to});
        m_resultVertices = m_inTree.values();
        emit finished(QStringLiteral("Prim 完成 (图不连通)，总权值: %1").arg(m_totalWeight));
        return false;
    }

    m_inTree.insert(bestEdge.to);
    m_inTreeOrder.append(bestEdge.to);
    m_visitedVertices.insert(bestEdge.to);
    m_treeEdges.append(bestEdge);
    m_totalWeight += bestEdge.weight;

    m_highlightedVertices = {bestEdge.to};
    m_highlightedEdges = {{bestEdge.from, bestEdge.to}};

    m_log.append(QStringLiteral("选边 %1→%2 (权=%3)").arg(
        m_graph->vertex(bestEdge.from).label,
        m_graph->vertex(bestEdge.to).label).arg(bestEdge.weight));

    emit stateChanged();
    return true;
}

QStringList PrimAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 最小生成树边 ===");
    for (auto &e : m_treeEdges) {
        lines << QStringLiteral("  %1 — %2  权值:%3").arg(
            m_graph->vertex(e.from).label,
            m_graph->vertex(e.to).label).arg(e.weight);
    }
    lines << QStringLiteral("");
    lines << QStringLiteral("总权值: %1").arg(m_totalWeight);
    lines << QStringLiteral("");
    lines << QStringLiteral("=== 已加入顶点 ===");
    QStringList inTree;
    for (int id : m_inTreeOrder) inTree << m_graph->vertex(id).label;
    lines << QStringLiteral("  %1").arg(inTree.join(QStringLiteral(" → ")));
    return lines;
}

// ==================== Kruskal ====================

KruskalAlgorithm::KruskalAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_edgeIndex(0), m_totalWeight(0), m_initialized(false) {}

void KruskalAlgorithm::reset()
{
    m_sortedEdges.clear();
    m_edgeIndex = 0;
    m_parent.clear();
    m_rank.clear();
    m_treeEdges.clear();
    m_totalWeight = 0;
    m_visitedVertices.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
}

int KruskalAlgorithm::find(int x)
{
    if (m_parent[x] != x)
        m_parent[x] = find(m_parent[x]);
    return m_parent[x];
}

int KruskalAlgorithm::findRoot(int x) const
{
    while (m_parent.value(x, x) != x)
        x = m_parent.value(x, x);
    return x;
}

void KruskalAlgorithm::unite(int a, int b)
{
    int ra = find(a), rb = find(b);
    if (ra == rb) return;
    if (m_rank[ra] < m_rank[rb]) std::swap(ra, rb);
    m_parent[rb] = ra;
    if (m_rank[ra] == m_rank[rb]) m_rank[ra]++;
}

bool KruskalAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;
        m_sortedEdges = m_graph->edges();
        std::sort(m_sortedEdges.begin(), m_sortedEdges.end(),
            [](const Edge &a, const Edge &b) { return a.weight < b.weight; });

        for (int v : m_graph->vertices()) {
            m_parent[v] = v;
            m_rank[v] = 0;
        }
        m_log.append(QStringLiteral("初始化: 按权值排序 %1 条边，初始化并查集").arg(m_sortedEdges.size()));
        emit stateChanged();
        return true;
    }

    int n = m_graph->vertexCount();
    while (m_edgeIndex < m_sortedEdges.size()) {
        Edge e = m_sortedEdges[m_edgeIndex++];
        int fu = find(e.from), fv = find(e.to);

        if (fu != fv) {
            unite(e.from, e.to);
            m_treeEdges.append(e);
            m_totalWeight += e.weight;

            m_visitedVertices.insert(e.from);
            m_visitedVertices.insert(e.to);
            m_highlightedVertices = {e.from, e.to};
            m_highlightedEdges = {{e.from, e.to}};

            m_log.append(QStringLiteral("选边 %1—%2 (权=%3) ✓").arg(
                m_graph->vertex(e.from).label,
                m_graph->vertex(e.to).label).arg(e.weight));
            emit stateChanged();

            if (m_treeEdges.size() >= n - 1) {
                m_finished = true;
                m_highlightedVertices.clear();
                m_highlightedEdges.clear();
                for (auto &te : m_treeEdges)
                    m_resultEdges.append({te.from, te.to});
                m_resultVertices = m_visitedVertices.values();
                emit finished(QStringLiteral("Kruskal 完成，总权值: %1").arg(m_totalWeight));
                return false;
            }
            return true;
        } else {
            m_log.append(QStringLiteral("跳过 %1—%2 (权=%3) 会形成环").arg(
                m_graph->vertex(e.from).label,
                m_graph->vertex(e.to).label).arg(e.weight));
        }
    }

    m_finished = true;
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    for (auto &te : m_treeEdges)
        m_resultEdges.append({te.from, te.to});
    m_resultVertices = m_visitedVertices.values();
    emit finished(QStringLiteral("Kruskal 完成，总权值: %1").arg(m_totalWeight));
    return false;
}

QStringList KruskalAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 已选边 ===");
    for (auto &e : m_treeEdges) {
        lines << QStringLiteral("  %1 — %2  权值:%3").arg(
            m_graph->vertex(e.from).label,
            m_graph->vertex(e.to).label).arg(e.weight);
    }
    lines << QStringLiteral("");
    lines << QStringLiteral("总权值: %1").arg(m_totalWeight);

    lines << QStringLiteral("");
    lines << QStringLiteral("=== 并查集 ===");
    QMap<int, QStringList> groups;
    for (int v : m_graph->vertices()) {
        int root = findRoot(v);
        groups[root] << m_graph->vertex(v).label;
    }
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        lines << QStringLiteral("  集合: {%1}").arg(it.value().join(QStringLiteral(", ")));
    }
    return lines;
}

// ==================== 拓扑排序 ====================

TopologicalSortAlgorithm::TopologicalSortAlgorithm(Graph *graph, QObject *parent)
    : AlgorithmBase(graph, parent), m_initialized(false), m_hasCycle(false) {}

void TopologicalSortAlgorithm::reset()
{
    m_inDegree.clear();
    m_queue.clear();
    m_result.clear();
    m_visitedVertices.clear();
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    m_resultEdges.clear();
    m_resultVertices.clear();
    m_log.clear();
    m_initialized = false;
    m_finished = false;
    m_hasCycle = false;
}

bool TopologicalSortAlgorithm::step()
{
    if (!m_initialized) {
        m_initialized = true;

        if (m_graph->type() != Directed) {
            m_finished = true;
            m_log.append(QStringLiteral("错误: 拓扑排序仅适用于有向图"));
            emit finished(QStringLiteral("拓扑排序仅适用于有向图"));
            return false;
        }

        for (int v : m_graph->vertices())
            m_inDegree[v] = 0;
        for (auto &e : m_graph->edges())
            m_inDegree[e.to]++;

        for (int v : m_graph->vertices()) {
            if (m_inDegree[v] == 0)
                m_queue.enqueue(v);
        }

        m_log.append(QStringLiteral("初始化: 计算入度，入度为0的顶点入队"));
        emit stateChanged();
        return true;
    }

    if (m_queue.isEmpty()) {
        m_finished = true;
        m_highlightedVertices.clear();
        m_highlightedEdges.clear();

        if (m_result.size() < m_graph->vertexCount()) {
            m_hasCycle = true;
            emit finished(QStringLiteral("拓扑排序失败: 图中存在环！"));
        } else {
            m_resultVertices = m_result;
            QStringList labels;
            for (int id : m_result) labels << m_graph->vertex(id).label;
            emit finished(QStringLiteral("拓扑排序完成: %1").arg(labels.join(QStringLiteral(" → "))));
        }
        return false;
    }

    int u = m_queue.dequeue();
    m_result.append(u);
    m_visitedVertices.insert(u);
    m_highlightedVertices = {u};
    m_highlightedEdges.clear();

    m_log.append(QStringLiteral("输出 %1 (入度=0)").arg(m_graph->vertex(u).label));

    for (auto &e : m_graph->adjacency(u)) {
        m_inDegree[e.to]--;
        m_highlightedEdges.append({u, e.to});
        if (m_inDegree[e.to] == 0) {
            m_queue.enqueue(e.to);
            m_log.append(QStringLiteral("  %1 入度变为0，入队").arg(
                m_graph->vertex(e.to).label));
        }
    }

    emit stateChanged();
    return true;
}

QStringList TopologicalSortAlgorithm::dataDisplayLines() const
{
    QStringList lines;
    lines << QStringLiteral("=== 入度表 ===");
    for (int v : m_graph->vertices()) {
        lines << QStringLiteral("  %1: 入度=%2").arg(
            m_graph->vertex(v).label).arg(m_inDegree.value(v, 0));
    }

    lines << QStringLiteral("");
    lines << QStringLiteral("=== 队列 ===");
    QStringList qLabels;
    for (int i = 0; i < m_queue.size(); ++i)
        qLabels << m_graph->vertex(m_queue.at(i)).label;
    lines << QStringLiteral("  %1").arg(qLabels.isEmpty() ? QStringLiteral("(空)") : qLabels.join(QStringLiteral(", ")));

    lines << QStringLiteral("");
    lines << QStringLiteral("=== 拓扑序列 ===");
    QStringList rLabels;
    for (int id : m_result) rLabels << m_graph->vertex(id).label;
    lines << QStringLiteral("  %1").arg(rLabels.isEmpty() ? QStringLiteral("(无)") : rLabels.join(QStringLiteral(" → ")));

    if (m_hasCycle) {
        lines << QStringLiteral("");
        lines << QStringLiteral("!! 检测到环 !!");
    }
    return lines;
}
