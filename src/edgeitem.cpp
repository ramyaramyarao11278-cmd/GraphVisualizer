#include "edgeitem.h"
#include "vertexitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <QFont>

EdgeItem::EdgeItem(VertexItem *from, VertexItem *to, int weight, bool directed,
                   QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
    , m_from(from)
    , m_to(to)
    , m_weight(weight)
    , m_directed(directed)
    , m_color(QColor(80, 80, 80))
    , m_penWidth(2.0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(1); // 边在顶点之下
    setAcceptHoverEvents(true);
    updatePosition();
}

void EdgeItem::setWeight(int w)
{
    m_weight = w;
    update();
}

void EdgeItem::setColor(const QColor &color)
{
    m_color = color;
    m_penWidth = (color == QColor(80, 80, 80)) ? 2.0 : 3.5;
    update();
}

void EdgeItem::resetColor()
{
    m_color = QColor(80, 80, 80);
    m_penWidth = 2.0;
    update();
}

void EdgeItem::updatePosition()
{
    if (!m_from || !m_to) return;
    setLine(QLineF(m_from->pos(), m_to->pos()));
}

QRectF EdgeItem::boundingRect() const
{
    qreal extra = m_penWidth + 20;
    return QGraphicsLineItem::boundingRect().adjusted(-extra, -extra, extra, extra);
}

QPainterPath EdgeItem::shape() const
{
    QPainterPath path;
    QPainterPathStroker stroker;
    stroker.setWidth(12);
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    return stroker.createStroke(path);
}

void EdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_from || !m_to) return;
    QPointF p1 = m_from->pos();
    QPointF p2 = m_to->pos();

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(m_color, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    if (isSelected()) {
        pen.setColor(Qt::yellow);
        pen.setWidthF(3.5);
    }
    painter->setPen(pen);

    // 计算边缘连接点（圆边缘而非圆心）
    QLineF fullLine(p1, p2);
    qreal len = fullLine.length();
    if (len < 1) return;

    QPointF dir = (p2 - p1) / len;
    QPointF edgeStart = p1 + dir * VertexItem::Radius;
    QPointF edgeEnd = p2 - dir * VertexItem::Radius;

    painter->drawLine(edgeStart, edgeEnd);

    // 有向图画箭头
    if (m_directed) {
        drawArrow(painter, edgeStart, edgeEnd);
    }

    // 绘制权值
    QPointF mid = (p1 + p2) / 2.0;
    // 偏移以避免与边重叠
    QPointF normal(-dir.y(), dir.x());
    QPointF textPos = mid + normal * 15;

    painter->setPen(QColor(200, 60, 60));
    QFont font("Microsoft YaHei", 9, QFont::Bold);
    painter->setFont(font);
    QRectF textRect(textPos.x() - 15, textPos.y() - 10, 30, 20);
    painter->drawText(textRect, Qt::AlignCenter, QString::number(m_weight));
}

void EdgeItem::drawArrow(QPainter *painter, const QPointF &from, const QPointF &to)
{
    qreal arrowSize = 12;
    QLineF line(from, to);
    double angle = std::atan2(-line.dy(), line.dx());

    QPointF arrowP1 = to + QPointF(std::cos(angle + M_PI + M_PI / 6) * arrowSize,
                                    -std::sin(angle + M_PI + M_PI / 6) * arrowSize);
    QPointF arrowP2 = to + QPointF(std::cos(angle + M_PI - M_PI / 6) * arrowSize,
                                    -std::sin(angle + M_PI - M_PI / 6) * arrowSize);

    QPolygonF arrowHead;
    arrowHead << to << arrowP1 << arrowP2;
    painter->setBrush(m_color);
    painter->drawPolygon(arrowHead);
}

void EdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_from && m_to)
        emit doubleClicked(m_from->vertexId(), m_to->vertexId());
}
