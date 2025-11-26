#pragma once
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPainter>                  // para a paint override
#include <QStyleOptionGraphicsItem>  // idem

class StateItem : public QGraphicsEllipseItem {
public:
    explicit StateItem(const QString& name, QGraphicsItem* parent = nullptr);

    QString name() const { return name_; }
    void setName(const QString& s);

    // NOVO: inicial/final
    bool isInitial() const { return initial_; }
    void setInitial(bool v);

    bool isFinal() const { return final_; }
    void setFinal(bool v);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override; // NOVO

private:
    void updateLabelPos();

    QString name_;
    QGraphicsTextItem* label_ = nullptr;
    bool initial_ = false;   // NOVO
    bool final_   = false;   // NOVO
};
