#include "StateItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>
#include <QInputDialog>
#include <QLineEdit>
#include <QPen>
#include <QBrush>
#include "TransitionItem.h"

namespace { constexpr qreal R = 50.0; }

StateItem::StateItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent), name_(name)
{
    setRect(-R, -R, 2*R, 2*R);
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setPen(QPen(Qt::black, 1.5));
    // setBrush(QBrush(QColor(240, 240, 255)));
    setBrush(baseBrush_); // <- usa cor base

    label_ = new QGraphicsTextItem(name_, this);
    label_->setDefaultTextColor(Qt::black);
    label_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    updateLabelPos();
}

void StateItem::setActive(bool v){
    active_ = v;
    update();
}

void StateItem::setName(const QString& s){
    name_ = s;
    label_->setPlainText(name_);
    updateLabelPos();
}

void StateItem::setInitial(bool v){
    initial_ = v;
    // borda azul quando inicial, preta caso contrário
    setPen(QPen(v ? QColor(30,80,200) : Qt::black, v ? 2.2 : 1.5));
    update();
}

void StateItem::setFinal(bool v){
    final_ = v;
    update(); // o círculo duplo é desenhado em paint()
}

void StateItem::updateLabelPos(){
    const QRectF r = rect();
    const QRectF b = label_->boundingRect();
    label_->setPos(r.center().x() - b.width()/2.0,
                   r.center().y() - b.height()/2.0);
}

QVariant StateItem::itemChange(GraphicsItemChange change, const QVariant& value){
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        updateLabelPos();
        if (scene()){
            const auto all = scene()->items();
            for (QGraphicsItem* gi : all)
                if (auto t = dynamic_cast<TransitionItem*>(gi))
                    t->updatePath();
        }
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

void StateItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e){
    QWidget* parentWidget = nullptr;
    if (scene() && !scene()->views().isEmpty())
        parentWidget = scene()->views().first();

    bool ok = false;
    QString newName = QInputDialog::getText(
        parentWidget,
        QObject::tr("Renomear estado"),
        QObject::tr("Nome:"),
        QLineEdit::Normal,
        name_,
        &ok
    );

    if (ok && !newName.trimmed().isEmpty()){
        setName(newName.trimmed());
    }
    QGraphicsEllipseItem::mouseDoubleClickEvent(e);
}

// Desenha o estado; se for "final", desenha um anel interno
void StateItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w){
    // desenho controlado (em vez do paint da base) para colorir "ativo"
    p->setPen(pen());
    p->setBrush(active_ ? QBrush(QColor(255,250,200)) : brush());
    p->drawEllipse(rect());

    // estado final: círculo interno
    if (final_){
        QPen ringPen = pen();
        ringPen.setWidthF(std::max(1.0, pen().widthF() * 0.8));
        p->setPen(ringPen);
        p->setBrush(Qt::NoBrush);
        constexpr qreal inset = 6.0;
        QRectF inner = rect().adjusted(inset, inset, -inset, -inset);
        p->drawEllipse(inner);
    }

    // desenhar o rótulo normalmente
    Q_UNUSED(o); Q_UNUSED(w);
}
