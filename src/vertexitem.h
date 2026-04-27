#ifndef VERTEXITEM_H
#define VERTEXITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>

class QPropertyAnimation;

class VertexItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColorDirect)
public:
    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    explicit VertexItem(int id, const QString &label, QGraphicsItem *parent = nullptr);

    int vertexId() const { return m_id; }
    QString label() const { return m_label; }
    void setLabel(const QString &label);

    QColor color() const { return m_color; }
    void setColor(const QColor &color);       // 带动画
    void setColorDirect(const QColor &color);  // 无动画（供QPropertyAnimation调用）
    void resetColor();

    // 控制是否允许拖拽（添加边模式时禁用）
    void setDraggable(bool enabled) { m_draggable = enabled; }

    static constexpr qreal Radius = 22.0;

signals:
    void positionChanged(int id, QPointF pos);
    void doubleClicked(int id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int m_id;
    QString m_label;
    QColor m_color;
    bool m_dragging;
    bool m_draggable;
    QPropertyAnimation *m_colorAnim;
};

#endif // VERTEXITEM_H
