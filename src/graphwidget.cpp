#include "graphwidget.h"
#include "vertexitem.h"
#include "edgeitem.h"
#include <QMouseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>

GraphWidget::GraphWidget(Graph *graph, QWidget *parent)
    : QGraphicsView(parent)
    , m_graph(graph)
    , m_mode(SelectMode)
    , m_edgeStartVertex(-1)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-2000, -2000, 4000, 4000);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QLinearGradient bg(0, 0, 0, 800);
    bg.setColorAt(0, QColor("#F8FAFF"));
    bg.setColorAt(1, QColor("#EEF2FF"));
    setBackgroundBrush(bg);
    setStyleSheet("QGraphicsView { border: none; }");

    // 连接Graph信号
    connect(m_graph, &Graph::vertexAdded, this, &GraphWidget::onVertexAdded);
    connect(m_graph, &Graph::vertexRemoved, this, &GraphWidget::onVertexRemoved);
    connect(m_graph, &Graph::vertexChanged, this, &GraphWidget::onVertexChanged);
    connect(m_graph, &Graph::edgeAdded, this, &GraphWidget::onEdgeAdded);
    connect(m_graph, &Graph::edgeRemoved, this, &GraphWidget::onEdgeRemoved);
    connect(m_graph, &Graph::graphCleared, this, &GraphWidget::onGraphCleared);
}

void GraphWidget::setInteractionMode(InteractionMode mode)
{
    m_mode = mode;
    m_edgeStartVertex = -1;

    // 切换模式时更新所有顶点的拖拽状态
    bool draggable = (mode == SelectMode);
    for (auto *item : m_vertexItems) {
        item->setDraggable(draggable);
    }

    if (mode == SelectMode) {
        setCursor(Qt::ArrowCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void GraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_mode != SelectMode) return;

    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *item = m_scene->itemAt(scenePos, transform());

    if (!item) {
        // 双击空白处添加顶点
        m_graph->addVertex("", scenePos);
        return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_mode == AddEdgeMode && event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        // 查找点击位置的顶点
        VertexItem *vItem = findVertexAt(scenePos);

        if (vItem) {
            if (m_edgeStartVertex < 0) {
                // 选择起点
                m_edgeStartVertex = vItem->vertexId();
                vItem->setColor(Qt::yellow);
                emit statusMessage(QStringLiteral("已选起点 %1，请点击终点").arg(vItem->label()));
            } else {
                // 选择终点
                int to = vItem->vertexId();
                if (to != m_edgeStartVertex) {
                    if (m_graph->containsEdge(m_edgeStartVertex, to)) {
                        emit statusMessage(QStringLiteral("该边已存在"));
                    } else {
                        bool ok;
                        int weight = QInputDialog::getInt(this, QStringLiteral("边权值"),
                            QStringLiteral("请输入权值:"), 1, 1, 9999, 1, &ok);
                        if (ok) {
                            m_graph->addEdge(m_edgeStartVertex, to, weight);
                        }
                    }
                }
                // 恢复起点颜色并重置
                if (m_vertexItems.contains(m_edgeStartVertex))
                    m_vertexItems[m_edgeStartVertex]->resetColor();
                m_edgeStartVertex = -1;
                emit statusMessage(QStringLiteral("添加边模式: 点击选择起点"));
            }
        } else {
            // 点击空白处取消选择
            if (m_edgeStartVertex >= 0) {
                if (m_vertexItems.contains(m_edgeStartVertex))
                    m_vertexItems[m_edgeStartVertex]->resetColor();
                m_edgeStartVertex = -1;
                emit statusMessage(QStringLiteral("已取消，点击选择起点"));
            }
        }
        return; // 添加边模式下拦截所有点击
    }
    QGraphicsView::mousePressEvent(event);
}

VertexItem *GraphWidget::findVertexAt(const QPointF &scenePos)
{
    QList<QGraphicsItem*> items = m_scene->items(scenePos, Qt::IntersectsItemShape,
                                                  Qt::DescendingOrder, transform());
    for (auto *item : items) {
        if (item->type() == VertexItem::Type)
            return static_cast<VertexItem*>(item);
    }
    return nullptr;
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        deleteSelected();
    }
    QGraphicsView::keyPressEvent(event);
}

void GraphWidget::deleteSelected()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    // 先收集要删除的ID，再执行删除（避免迭代中修改）
    QList<int> verticesToRemove;
    QList<QPair<int,int>> edgesToRemove;

    for (auto *item : selected) {
        if (item->type() == VertexItem::Type) {
            verticesToRemove.append(static_cast<VertexItem*>(item)->vertexId());
        } else if (item->type() == EdgeItem::Type) {
            auto *eItem = static_cast<EdgeItem*>(item);
            edgesToRemove.append({eItem->fromVertex()->vertexId(),
                                  eItem->toVertex()->vertexId()});
        }
    }

    for (auto &e : edgesToRemove)
        m_graph->removeEdge(e.first, e.second);
    for (int id : verticesToRemove)
        m_graph->removeVertex(id);
}

void GraphWidget::onVertexAdded(int id)
{
    Vertex v = m_graph->vertex(id);
    auto *item = new VertexItem(id, v.label);
    item->setPos(v.pos);
    item->setDraggable(m_mode == SelectMode);
    m_scene->addItem(item);
    m_vertexItems.insert(id, item);

    connect(item, &VertexItem::positionChanged,
            this, &GraphWidget::onVertexPositionChanged);
    connect(item, &VertexItem::doubleClicked, [this](int vid) {
        if (m_mode != SelectMode) return;
        bool ok;
        QString label = QInputDialog::getText(this, QStringLiteral("编辑标签"),
            QStringLiteral("顶点标签:"), QLineEdit::Normal,
            m_graph->vertex(vid).label, &ok);
        if (ok && !label.isEmpty()) {
            m_graph->setVertexLabel(vid, label);
        }
    });
}

void GraphWidget::onVertexRemoved(int id)
{
    if (m_vertexItems.contains(id)) {
        m_scene->removeItem(m_vertexItems[id]);
        delete m_vertexItems[id];
        m_vertexItems.remove(id);
    }
}

void GraphWidget::onVertexChanged(int id)
{
    if (m_vertexItems.contains(id)) {
        Vertex v = m_graph->vertex(id);
        m_vertexItems[id]->setLabel(v.label);
    }
}

void GraphWidget::onEdgeAdded(int from, int to)
{
    if (!m_vertexItems.contains(from) || !m_vertexItems.contains(to)) return;
    if (m_edgeItems.contains({from, to})) return;

    auto *item = new EdgeItem(m_vertexItems[from], m_vertexItems[to],
                              m_graph->edge(from, to).weight,
                              m_graph->type() == Directed);
    m_scene->addItem(item);
    m_edgeItems.insert({from, to}, item);

    connect(item, &EdgeItem::doubleClicked, [this](int f, int t) {
        if (m_mode != SelectMode) return;
        bool ok;
        int w = QInputDialog::getInt(this, QStringLiteral("编辑权值"),
            QStringLiteral("权值:"), m_graph->edge(f, t).weight, 1, 9999, 1, &ok);
        if (ok) {
            m_graph->setEdgeWeight(f, t, w);
            if (m_edgeItems.contains({f, t}))
                m_edgeItems[{f, t}]->setWeight(w);
            if (m_graph->type() == Undirected && m_edgeItems.contains({t, f}))
                m_edgeItems[{t, f}]->setWeight(w);
        }
    });
}

void GraphWidget::onEdgeRemoved(int from, int to)
{
    if (m_edgeItems.contains({from, to})) {
        m_scene->removeItem(m_edgeItems[{from, to}]);
        delete m_edgeItems[{from, to}];
        m_edgeItems.remove({from, to});
    }
}

void GraphWidget::onGraphCleared()
{
    m_scene->clear();
    m_vertexItems.clear();
    m_edgeItems.clear();
}

void GraphWidget::onVertexPositionChanged(int id, QPointF pos)
{
    m_graph->setVertexPos(id, pos);
    updateEdgesForVertex(id);
}

void GraphWidget::updateEdgesForVertex(int id)
{
    for (auto it = m_edgeItems.begin(); it != m_edgeItems.end(); ++it) {
        EdgeItem *eItem = it.value();
        if (eItem->fromVertex()->vertexId() == id ||
            eItem->toVertex()->vertexId() == id) {
            eItem->updatePosition();
        }
    }
}

void GraphWidget::highlightVertices(const QList<int> &ids, const QColor &color)
{
    for (int id : ids) {
        if (m_vertexItems.contains(id))
            m_vertexItems[id]->setColor(color);
    }
}

void GraphWidget::highlightEdges(const QList<QPair<int,int>> &edges, const QColor &color)
{
    for (auto &e : edges) {
        if (m_edgeItems.contains(e))
            m_edgeItems[e]->setColor(color);
        // 无向图也高亮反向边
        if (m_edgeItems.contains({e.second, e.first}))
            m_edgeItems[{e.second, e.first}]->setColor(color);
    }
}

void GraphWidget::clearHighlights()
{
    for (auto *item : m_vertexItems)
        item->resetColor();
    for (auto *item : m_edgeItems)
        item->resetColor();
}

void GraphWidget::clearAllToGray()
{
    for (auto *item : m_vertexItems)
        item->setColor(QColor(180, 180, 180)); // 未访问灰色
    for (auto *item : m_edgeItems)
        item->resetColor();
}

VertexItem *GraphWidget::vertexItem(int id) const
{
    return m_vertexItems.value(id, nullptr);
}

EdgeItem *GraphWidget::edgeItem(int from, int to) const
{
    return m_edgeItems.value({from, to}, nullptr);
}

void GraphWidget::rebuildFromGraph()
{
    onGraphCleared();
    for (int id : m_graph->vertices()) {
        onVertexAdded(id);
    }
    for (auto &e : m_graph->edges()) {
        onEdgeAdded(e.from, e.to);
    }
}
