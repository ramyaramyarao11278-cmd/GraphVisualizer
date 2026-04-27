#include "vertexitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QFont>
#include <QCursor>
#include <QPropertyAnimation>

VertexItem::VertexItem(int id, const QString &label, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_id(id)
    , m_label(label)
    , m_color(QColor(100, 149, 237)) // cornflower blue
    , m_dragging(false)
    , m_draggable(true)
    , m_colorAnim(nullptr)
{
    setRect(-Radius, -Radius, 2 * Radius, 2 * Radius);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(10); // 顶点在边之上
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
}

void VertexItem::setLabel(const QString &label)
{
    m_label = label;
    update();
}

void VertexItem::setColor(const QColor &color)
{
    if (m_color == color) return;
    delete m_colorAnim;
    m_colorAnim = new QPropertyAnimation(this, "color", this);
    m_colorAnim->setDuration(250);
    m_colorAnim->setStartValue(m_color);
    m_colorAnim->setEndValue(color);
    m_colorAnim->start();
}

void VertexItem::setColorDirect(const QColor &color)
{
    m_color = color;
    update();
}

void VertexItem::resetColor()
{
    delete m_colorAnim;
    m_colorAnim = nullptr;
    m_color = QColor(100, 149, 237);
    update();
}

void VertexItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // 外发光
    painter->setPen(Qt::NoPen);
    QColor glow = m_color;
    glow.setAlpha(35);
    painter->setBrush(glow);
    painter->drawEllipse(rect().adjusted(-4, -4, 4, 4));

    // 阴影
    painter->setBrush(QColor(0, 0, 0, 30));
    painter->drawEllipse(rect().translated(2, 3));

    // 圆形主体
    QRadialGradient gradient(QPointF(-Radius * 0.25, -Radius * 0.35), Radius * 1.6);
    gradient.setColorAt(0, m_color.lighter(145));
    gradient.setColorAt(0.6, m_color);
    gradient.setColorAt(1, m_color.darker(120));
    painter->setBrush(gradient);

    if (isSelected()) {
        painter->setPen(QPen(QColor("#FBBF24"), 3));
    } else {
        painter->setPen(QPen(m_color.darker(140), 1.5));
    }
    painter->drawEllipse(rect());

    // 高光
    painter->setPen(Qt::NoPen);
    QRadialGradient highlight(QPointF(-Radius * 0.2, -Radius * 0.3), Radius * 0.7);
    highlight.setColorAt(0, QColor(255, 255, 255, 90));
    highlight.setColorAt(1, QColor(255, 255, 255, 0));
    painter->setBrush(highlight);
    painter->drawEllipse(rect().adjusted(2, 2, -2, -2));

    // 标签
    painter->setPen(Qt::white);
    QFont font("Microsoft YaHei UI", 10, QFont::Bold);
    painter->setFont(font);
    painter->drawText(rect(), Qt::AlignCenter, m_label);
}

void VertexItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_draggable) {
        m_dragging = true;
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

void VertexItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragging && m_draggable) {
        setPos(event->scenePos());
        emit positionChanged(m_id, pos());
    }
}

void VertexItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_dragging = false;
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void VertexItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubleClicked(m_id);
}
