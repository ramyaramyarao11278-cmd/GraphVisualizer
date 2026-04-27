#ifndef ALGORITHMBASE_H
#define ALGORITHMBASE_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QSet>
#include <QStringList>
#include "graph.h"

class AlgorithmBase : public QObject {
    Q_OBJECT
public:
    explicit AlgorithmBase(Graph *graph, QObject *parent = nullptr);
    virtual ~AlgorithmBase();

    virtual QString name() const = 0;
    virtual bool needsStartVertex() const { return true; }
    virtual void setStartVertex(int id) { m_startVertex = id; }

    virtual bool step() = 0;
    virtual void reset() = 0;

    // 当前步高亮（红色）
    QList<int> highlightedVertices() const { return m_highlightedVertices; }
    QList<QPair<int,int>> highlightedEdges() const { return m_highlightedEdges; }

    // 已访问顶点（绿色）
    QSet<int> visitedVertices() const { return m_visitedVertices; }

    // 结果边集合（算法完成后持久高亮，蓝色）
    QList<QPair<int,int>> resultEdges() const { return m_resultEdges; }
    // 结果顶点（算法完成后持久高亮）
    QList<int> resultVertices() const { return m_resultVertices; }

    virtual QStringList dataDisplayLines() const { return {}; }
    QStringList log() const { return m_log; }

    bool isFinished() const { return m_finished; }

signals:
    void stateChanged();
    void finished(const QString &summary);

protected:
    Graph *m_graph;
    int m_startVertex;
    bool m_finished;

    // 当前步
    QList<int> m_highlightedVertices;
    QList<QPair<int,int>> m_highlightedEdges;

    // 已访问
    QSet<int> m_visitedVertices;

    // 最终结果
    QList<QPair<int,int>> m_resultEdges;
    QList<int> m_resultVertices;

    QStringList m_log;
};

#endif // ALGORITHMBASE_H
