#include "graph.h"

Graph::Graph(GraphType type, QObject *parent)
    : QObject(parent), m_type(type), m_nextId(0)
{
}

int Graph::addVertex(const QString &label, const QPointF &pos)
{
    int id = m_nextId++;
    Vertex v;
    v.id = id;
    v.label = label.isEmpty() ? QString("V%1").arg(id) : label;
    v.pos = pos;
    m_vertices.insert(id, v);
    emit vertexAdded(id);
    return id;
}

void Graph::removeVertex(int id)
{
    if (!m_vertices.contains(id)) return;

    // 删除关联的边
    QList<QPair<int,int>> toRemove;
    for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
        if (it.value().from == id || it.value().to == id)
            toRemove.append(it.key());
    }
    for (auto &key : toRemove) {
        m_edges.remove(key);
        emit edgeRemoved(key.first, key.second);
    }

    m_vertices.remove(id);
    emit vertexRemoved(id);
}

void Graph::setVertexLabel(int id, const QString &label)
{
    if (!m_vertices.contains(id)) return;
    m_vertices[id].label = label;
    emit vertexChanged(id);
}

void Graph::setVertexPos(int id, const QPointF &pos)
{
    if (!m_vertices.contains(id)) return;
    m_vertices[id].pos = pos;
}

Vertex Graph::vertex(int id) const
{
    return m_vertices.value(id);
}

bool Graph::containsVertex(int id) const
{
    return m_vertices.contains(id);
}

void Graph::addEdge(int from, int to, int weight)
{
    if (!m_vertices.contains(from) || !m_vertices.contains(to)) return;
    if (from == to) return;

    Edge e;
    e.from = from;
    e.to = to;
    e.weight = weight;
    m_edges.insert({from, to}, e);

    if (m_type == Undirected) {
        Edge e2;
        e2.from = to;
        e2.to = from;
        e2.weight = weight;
        m_edges.insert({to, from}, e2);
    }
    emit edgeAdded(from, to);
}

void Graph::removeEdge(int from, int to)
{
    if (!m_edges.contains({from, to})) return;
    m_edges.remove({from, to});
    if (m_type == Undirected) {
        m_edges.remove({to, from});
    }
    emit edgeRemoved(from, to);
}

void Graph::setEdgeWeight(int from, int to, int weight)
{
    bool changed = false;
    if (m_edges.contains({from, to})) {
        m_edges[{from, to}].weight = weight;
        changed = true;
    }
    if (m_type == Undirected && m_edges.contains({to, from})) {
        m_edges[{to, from}].weight = weight;
        changed = true;
    }
    if (changed)
        emit edgeChanged(from, to);
}

bool Graph::containsEdge(int from, int to) const
{
    return m_edges.contains({from, to});
}

Edge Graph::edge(int from, int to) const
{
    return m_edges.value({from, to});
}

QList<int> Graph::vertices() const
{
    return m_vertices.keys();
}

QList<Edge> Graph::edges() const
{
    if (m_type == Undirected) {
        // 无向图去重：只返回 from < to 的边
        QList<Edge> result;
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            if (it.value().from < it.value().to)
                result.append(it.value());
        }
        return result;
    }
    return m_edges.values();
}

QList<Edge> Graph::adjacency(int vertex) const
{
    QList<Edge> result;
    for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
        if (it.value().from == vertex)
            result.append(it.value());
    }
    return result;
}

int Graph::vertexCount() const
{
    return m_vertices.size();
}

int Graph::edgeCount() const
{
    if (m_type == Undirected)
        return m_edges.size() / 2;
    return m_edges.size();
}

GraphType Graph::type() const
{
    return m_type;
}

void Graph::setType(GraphType type)
{
    if (m_type == type) return;

    if (type == Undirected) {
        // 有向→无向：为每条边补上反向边
        QList<QPair<QPair<int,int>, Edge>> toAdd;
        for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
            QPair<int,int> rev = {it.key().second, it.key().first};
            if (!m_edges.contains(rev)) {
                Edge e;
                e.from = rev.first;
                e.to = rev.second;
                e.weight = it.value().weight;
                toAdd.append({rev, e});
            }
        }
        for (auto &pair : toAdd)
            m_edges.insert(pair.first, pair.second);
    } else {
        // 无向→有向：保留双向边即可（有向图允许双向）
    }

    m_type = type;
    emit typeChanged(type);
}

void Graph::clear()
{
    m_vertices.clear();
    m_edges.clear();
    m_nextId = 0;
    emit graphCleared();
}

Graph *Graph::clone(QObject *parent) const
{
    auto *g = new Graph(m_type, parent);
    g->fromJson(toJson());
    return g;
}

QJsonObject Graph::toJson() const
{
    QJsonObject obj;
    obj["type"] = (m_type == Directed) ? "directed" : "undirected";

    QJsonArray verticesArr;
    for (auto &v : m_vertices) {
        QJsonObject vobj;
        vobj["id"] = v.id;
        vobj["label"] = v.label;
        vobj["x"] = v.pos.x();
        vobj["y"] = v.pos.y();
        verticesArr.append(vobj);
    }

    QJsonArray edgesArr;
    auto edgeList = edges(); // 使用去重后的边
    for (auto &e : edgeList) {
        QJsonObject eobj;
        eobj["from"] = e.from;
        eobj["to"] = e.to;
        eobj["weight"] = e.weight;
        edgesArr.append(eobj);
    }

    obj["vertices"] = verticesArr;
    obj["edges"] = edgesArr;
    return obj;
}

bool Graph::fromJson(const QJsonObject &json)
{
    clear();

    QString typeStr = json["type"].toString("undirected");
    m_type = (typeStr == "directed") ? Directed : Undirected;

    QJsonArray verticesArr = json["vertices"].toArray();
    for (auto val : verticesArr) {
        QJsonObject vobj = val.toObject();
        int id = vobj["id"].toInt();
        Vertex v;
        v.id = id;
        v.label = vobj["label"].toString();
        v.pos = QPointF(vobj["x"].toDouble(), vobj["y"].toDouble());
        m_vertices.insert(id, v);
        if (id >= m_nextId) m_nextId = id + 1;
    }

    QJsonArray edgesArr = json["edges"].toArray();
    for (auto val : edgesArr) {
        QJsonObject eobj = val.toObject();
        addEdge(eobj["from"].toInt(), eobj["to"].toInt(), eobj["weight"].toInt(1));
    }

    return true;
}
