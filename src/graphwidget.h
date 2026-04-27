#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include "graph.h"

class VertexItem;
class EdgeItem;

class GraphWidget : public QGraphicsView {
    Q_OBJECT
public:
    enum InteractionMode { SelectMode, AddEdgeMode };

    explicit GraphWidget(Graph *graph, QWidget *parent = nullptr);

    void setInteractionMode(InteractionMode mode);
    InteractionMode interactionMode() const { return m_mode; }

    // 高亮控制
    void highlightVertices(const QList<int> &ids, const QColor &color);
    void highlightEdges(const QList<QPair<int,int>> &edges, const QColor &color);
    void clearHighlights();
    void clearAllToGray();

    // 获取图元
    VertexItem *vertexItem(int id) const;
    EdgeItem *edgeItem(int from, int to) const;

    void rebuildFromGraph();

signals:
    void vertexDoubleClicked(int id);
    void edgeDoubleClicked(int from, int to);
    void statusMessage(const QString &msg);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onVertexAdded(int id);
    void onVertexRemoved(int id);
    void onVertexChanged(int id);
    void onEdgeAdded(int from, int to);
    void onEdgeRemoved(int from, int to);
    void onGraphCleared();
    void onVertexPositionChanged(int id, QPointF pos);

private:
    VertexItem *findVertexAt(const QPointF &scenePos);
    void updateEdgesForVertex(int id);
    void deleteSelected();

    Graph *m_graph;
    QGraphicsScene *m_scene;
    QMap<int, VertexItem*> m_vertexItems;
    QMap<QPair<int,int>, EdgeItem*> m_edgeItems;
    InteractionMode m_mode;
    int m_edgeStartVertex;
};

#endif // GRAPHWIDGET_H
