#include "graphgenerator.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QSet>
#include <QHash>
#include <algorithm>
#include <climits>

namespace GraphGenerator {

namespace {

double autoRadius(int n) {
    return std::max(280.0, 30.0 * n / (2.0 * M_PI));
}

QList<QPointF> circularLayout(int n, QPointF center, double radius) {
    QList<QPointF> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        double theta = 2.0 * M_PI * i / n - M_PI / 2.0;
        pts.append(center + QPointF(radius * std::cos(theta),
                                     radius * std::sin(theta)));
    }
    return pts;
}

int pickWeight(const WeightConfig &w) {
    if (!w.randomWeights) return 1;
    int lo = std::min(w.minWeight, w.maxWeight);
    int hi = std::max(w.minWeight, w.maxWeight);
    return QRandomGenerator::global()->bounded(lo, hi + 1);
}

QList<int> createVertices(Graph *g, int n, const LayoutConfig &lc) {
    double r = lc.radius > 0 ? lc.radius : autoRadius(n);
    auto pts = circularLayout(n, lc.center, r);
    QList<int> ids;
    ids.reserve(n);
    for (int i = 0; i < n; ++i)
        ids.append(g->addVertex(QString::number(i), pts[i]));
    return ids;
}

qint64 edgeKey(int a, int b) {
    if (a > b) std::swap(a, b);
    return (qint64(a) << 32) | quint32(b);
}

} // anon

// ====================== Erdős-Rényi G(n, p) ======================

void generateErdosRenyiGnp(Graph *g, int n, double p,
                            const WeightConfig &wc, const LayoutConfig &lc) {
    g->clear();
    if (n <= 0) return;
    p = qBound(0.0, p, 1.0);

    auto ids = createVertices(g, n, lc);
    auto *rng = QRandomGenerator::global();

    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (rng->generateDouble() < p)
                g->addEdge(ids[i], ids[j], pickWeight(wc));
}

// ====================== Erdős-Rényi G(n, m) ======================

void generateErdosRenyiGnm(Graph *g, int n, int m,
                            const WeightConfig &wc, const LayoutConfig &lc) {
    g->clear();
    if (n <= 0) return;

    qint64 maxE = qint64(n) * (n - 1) / 2;
    m = int(std::min<qint64>(std::max<qint64>(m, 0), maxE));

    auto ids = createVertices(g, n, lc);
    auto *rng = QRandomGenerator::global();

    if (qint64(m) > maxE / 2) {
        QList<QPair<int,int>> all;
        all.reserve(int(maxE));
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                all.append({i, j});
        for (int k = 0; k < m; ++k) {
            int r = rng->bounded(k, int(all.size()));
            std::swap(all[k], all[r]);
            g->addEdge(ids[all[k].first], ids[all[k].second], pickWeight(wc));
        }
    } else {
        QSet<qint64> seen;
        while (int(seen.size()) < m) {
            int a = rng->bounded(n);
            int b = rng->bounded(n);
            if (a == b) continue;
            qint64 k = edgeKey(a, b);
            if (seen.contains(k)) continue;
            seen.insert(k);
            if (a > b) std::swap(a, b);
            g->addEdge(ids[a], ids[b], pickWeight(wc));
        }
    }
}

// ====================== Barabási-Albert ======================

void generateBarabasiAlbert(Graph *g, int n, int m, int m0,
                             const WeightConfig &wc, const LayoutConfig &lc) {
    g->clear();
    if (n <= 0 || m <= 0) return;
    if (m0 <= 0) m0 = m + 1;
    if (m0 < 2)  m0 = 2;
    if (m0 < m)  m0 = m;
    if (m0 > n)  m0 = n;

    auto ids = createVertices(g, n, lc);
    auto *rng = QRandomGenerator::global();

    for (int i = 0; i < m0; ++i)
        for (int j = i + 1; j < m0; ++j)
            g->addEdge(ids[i], ids[j], pickWeight(wc));

    QList<int> bag;
    bag.reserve(n * m * 2);
    for (int i = 0; i < m0; ++i)
        for (int k = 0; k < m0 - 1; ++k)
            bag.append(i);

    for (int i = m0; i < n; ++i) {
        QSet<int> targets;
        int tries = 0;
        while (targets.size() < m && tries < 10000) {
            int t = bag[rng->bounded(int(bag.size()))];
            targets.insert(t);
            ++tries;
        }
        for (int t : targets) {
            g->addEdge(ids[i], ids[t], pickWeight(wc));
            bag.append(i);
            bag.append(t);
        }
    }
}

// ====================== Watts-Strogatz ======================

void generateWattsStrogatz(Graph *g, int n, int k, double p,
                            const WeightConfig &wc, const LayoutConfig &lc) {
    g->clear();
    if (n <= 0) return;
    if (k % 2 != 0) ++k;
    k = std::min(k, (n - 1) - ((n - 1) % 2));
    if (k <= 0) { createVertices(g, n, lc); return; }
    p = qBound(0.0, p, 1.0);

    auto ids = createVertices(g, n, lc);
    auto *rng = QRandomGenerator::global();

    QSet<qint64> edges;
    for (int i = 0; i < n; ++i)
        for (int d = 1; d <= k / 2; ++d)
            edges.insert(edgeKey(i, (i + d) % n));

    QList<qint64> origEdges = edges.values();
    for (qint64 ek : origEdges) {
        if (rng->generateDouble() >= p) continue;
        int u = int(ek >> 32);
        int tries = 0;
        while (tries < 100) {
            int v = rng->bounded(n);
            if (v == u) { ++tries; continue; }
            qint64 nk = edgeKey(u, v);
            if (edges.contains(nk)) { ++tries; continue; }
            edges.remove(ek);
            edges.insert(nk);
            break;
        }
    }

    for (qint64 ek : edges) {
        int a = int(ek >> 32);
        int b = int(ek & 0xFFFFFFFF);
        g->addEdge(ids[a], ids[b], pickWeight(wc));
    }
}

// ====================== 图指标 ======================

GraphMetrics computeMetrics(const Graph *g) {
    GraphMetrics m;
    if (!g) return m;

    QList<int> verts = g->vertices();
    m.vertexCount = verts.size();
    m.edgeCount = g->edgeCount();
    if (m.vertexCount == 0) return m;

    QHash<int, int> deg;
    QHash<int, QSet<int>> nbrs;
    for (int v : verts) { deg[v] = 0; nbrs[v] = {}; }

    for (const auto &e : g->edges()) {
        deg[e.from]++;
        nbrs[e.from].insert(e.to);
        if (g->type() == Undirected) {
            deg[e.to]++;
            nbrs[e.to].insert(e.from);
        }
    }

    int totalDeg = 0;
    m.maxDegree = 0;
    m.minDegree = INT_MAX;
    for (int v : verts) {
        totalDeg += deg[v];
        m.maxDegree = std::max(m.maxDegree, deg[v]);
        m.minDegree = std::min(m.minDegree, deg[v]);
    }
    m.avgDegree = double(totalDeg) / m.vertexCount;

    if (g->type() == Undirected) {
        double totalC = 0.0;
        int counted = 0;
        for (int v : verts) {
            const auto &ns = nbrs[v];
            int kk = ns.size();
            if (kk < 2) continue;
            int tri = 0;
            QList<int> nlist = ns.values();
            for (int i = 0; i < nlist.size(); ++i)
                for (int j = i + 1; j < nlist.size(); ++j)
                    if (nbrs[nlist[i]].contains(nlist[j])) ++tri;
            totalC += double(tri) / (kk * (kk - 1) / 2.0);
            ++counted;
        }
        m.clusteringCoefficient = counted > 0 ? totalC / counted : 0.0;
    }

    QSet<int> visited;
    m.componentCount = 0;
    m.largestComponentSize = 0;
    for (int start : verts) {
        if (visited.contains(start)) continue;
        ++m.componentCount;
        QList<int> stack = {start};
        int sz = 0;
        while (!stack.isEmpty()) {
            int u = stack.takeLast();
            if (visited.contains(u)) continue;
            visited.insert(u);
            ++sz;
            for (int w : nbrs[u])
                if (!visited.contains(w)) stack.append(w);
        }
        m.largestComponentSize = std::max(m.largestComponentSize, sz);
    }

    return m;
}

} // namespace GraphGenerator
