#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QPair>
#include <QPointF>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

enum GraphType { Undirected, Directed };

struct Vertex {
    int id;
    QString label;
    QPointF pos;
};

struct Edge {
    int from;
    int to;
    int weight;
};

class Graph : public QObject {
    Q_OBJECT
public:
    explicit Graph(GraphType type = Undirected, QObject *parent = nullptr);

    // 顶点操作
    int addVertex(const QString &label = "", const QPointF &pos = QPointF());
    void removeVertex(int id);
    void setVertexLabel(int id, const QString &label);
    void setVertexPos(int id, const QPointF &pos);
    Vertex vertex(int id) const;
    bool containsVertex(int id) const;

    // 边操作
    void addEdge(int from, int to, int weight = 1);
    void removeEdge(int from, int to);
    void setEdgeWeight(int from, int to, int weight);
    bool containsEdge(int from, int to) const;
    Edge edge(int from, int to) const;

    // 查询
    QList<int> vertices() const;
    QList<Edge> edges() const;
    QList<Edge> adjacency(int vertex) const;
    int vertexCount() const;
    int edgeCount() const;

    // 图类型
    GraphType type() const;
    void setType(GraphType type);

    // 清空
    void clear();

    // 深拷贝
    Graph *clone(QObject *parent = nullptr) const;

    // JSON
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

signals:
    void vertexAdded(int id);
    void vertexRemoved(int id);
    void vertexChanged(int id);
    void edgeAdded(int from, int to);
    void edgeRemoved(int from, int to);
    void edgeChanged(int from, int to);
    void graphCleared();
    void typeChanged(GraphType type);

private:
    GraphType m_type;
    QMap<int, Vertex> m_vertices;
    QMap<QPair<int,int>, Edge> m_edges;
    int m_nextId;
};

#endif // GRAPH_H
