#pragma once
#include <QGraphicsPathItem>
class StateItem;

class TransitionItem : public QGraphicsPathItem {
public:
    explicit TransitionItem(StateItem* src, StateItem* dst, QGraphicsItem* parent=nullptr);

    StateItem* src() const { return src_; }
    StateItem* dst() const { return dst_; }

    int priority() const { return priority_; }
    const QString& guard()  const { return guard_; }
    const QString& action() const { return action_; }
    const QString& label()  const { return label_; }

    void setPriority(int p){ priority_=p; updateLabel(); }
    void setGuard(const QString& g){ guard_=g; updateLabel(); }
    void setAction(const QString& a){ action_=a; updateLabel(); }
    void setLabel(const QString& l){ label_=l; updateLabel(); }

    void updatePath(); // recalc line & arrow

    int id() const { return id_; }    // <- NOVO

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override; // NOVO (só pra realce)

private:
    void updateLabel();

    bool isSelfLoop() const { return src_ && dst_ && src_==dst_; }
    qreal computeParallelOffset() const; // desvio para paralelas (px)

    // NOVO: organizar irmãos self-loop e geometria
    int selfLoopIndexAndCount(int& total) const;     // índice do loop entre seus irmãos
    void selfLoopGeometry(QPointF& p0, QPointF& p1, QPointF& p2, QPointF& p3) const;

    // util p/ Bezier
    static QPointF cubicPoint(const QPointF& p0, const QPointF& p1,
                              const QPointF& p2, const QPointF& p3, qreal t);
    static QPointF cubicTangent(const QPointF& p0, const QPointF& p1,
                                const QPointF& p2, const QPointF& p3, qreal t);

    StateItem* src_{};
    StateItem* dst_{};
    int priority_{1};
    QString guard_{"true"};
    QString action_{};
    QString label_{};
    QGraphicsSimpleTextItem* text_{nullptr};

    // Pontos efetivos (borda → borda)
    QPointF startPt_;
    QPointF endPt_;

    QPolygonF headPoly_;        // <-- NOVO: triângulo da cabeça para pintar separado

    int id_{0};
    static int s_nextId_;
};
