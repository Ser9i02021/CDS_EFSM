#pragma once
#include <QGraphicsScene>

class StateItem;
class TransitionItem;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum class Mode { Select, AddTransition };

    explicit DiagramScene(QObject* parent=nullptr);

    void setMode(Mode m);
    Mode mode() const { return mode_; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    StateItem* stateAt(const QPointF& p) const;

    Mode mode_{Mode::Select};
    QGraphicsLineItem* tempLine_{nullptr};
    StateItem* pendingSrc_{nullptr};
};
