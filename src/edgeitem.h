#ifndef EDGEITEM_H
#define EDGEITEM_H

#include <QGraphicsLineItem>
#include <QObject>

class VertexItem;

class EdgeItem : public QObject, public QGraphicsLineItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
public:
    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    EdgeItem(VertexItem *from, VertexItem *to, int weight, bool directed,
             QGraphicsItem *parent = nullptr);

    VertexItem *fromVertex() const { return m_from; }
    VertexItem *toVertex() const { return m_to; }
    int weight() const { return m_weight; }
    void setWeight(int w);
    bool isDirected() const { return m_directed; }

    QColor color() const { return m_color; }
    void setColor(const QColor &color);
    void resetColor();

    void updatePosition();

signals:
    void doubleClicked(int from, int to);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    void drawArrow(QPainter *painter, const QPointF &from, const QPointF &to);

    VertexItem *m_from;
    VertexItem *m_to;
    int m_weight;
    bool m_directed;
    QColor m_color;
    qreal m_penWidth;
};

#endif // EDGEITEM_H
