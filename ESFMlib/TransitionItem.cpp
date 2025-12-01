#include "TransitionItem.h"
#include "TransitionEditorDialog.h"
#include "StateItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QLineF>
#include <QPainterPath>
#include <QtMath>
#include <cmath>
#include <QGraphicsView>   // <- necessário para scene()->views().first()
#include <QWidget>         // opcional, só para o tipo QWidget*
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>


int TransitionItem::s_nextId_ = 0;

TransitionItem::TransitionItem(StateItem* s, StateItem* d, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), src_(s), dst_(d)
{
    id_ = ++s_nextId_;                    // <- NOVO
    setZValue(-1); // atrás dos estados
    setPen(QPen(Qt::black, 1.3));
    setBrush(Qt::black); // preenche a cabeça da seta
    setFlags(QGraphicsItem::ItemIsSelectable);
    text_ = new QGraphicsSimpleTextItem(this);
    text_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    updatePath();
    updateLabel();
    text_->setZValue(10);

}

// ===== utilidades Bezier =====
QPointF TransitionItem::cubicPoint(const QPointF& p0, const QPointF& p1,
                                   const QPointF& p2, const QPointF& p3, qreal t)
{
    const qreal u = 1.0 - t;
    return u*u*u*p0 + 3*u*u*t*p1 + 3*u*t*t*p2 + t*t*t*p3;
}

QPointF TransitionItem::cubicTangent(const QPointF& p0, const QPointF& p1,
                                     const QPointF& p2, const QPointF& p3, qreal t)
{
    // derivada de Bezier cúbica
    return 3*std::pow(1-t,2)*(p1-p0) + 6*(1-t)*t*(p2-p1) + 3*t*t*(p3-p2);
}

qreal TransitionItem::computeParallelOffset() const {
    if (!scene() || !src_ || !dst_) return 0.0;

    // Defina o par canônico (independente da direção)
    StateItem* a = src_;
    StateItem* b = dst_;
    StateItem* lo = (a < b) ? a : b;
    StateItem* hi = (a < b) ? b : a;

    // Colete irmãos nas DUAS direções entre o mesmo par {lo,hi}
    QVector<const TransitionItem*> dirLoHi; // lo -> hi  (direção "canônica")
    QVector<const TransitionItem*> dirHiLo; // hi -> lo  (direção oposta)

    for (QGraphicsItem* gi : scene()->items()){
        if (auto t = dynamic_cast<TransitionItem*>(gi)){
            StateItem* ta = t->src();
            StateItem* tb = t->dst();
            StateItem* tlo = (ta < tb) ? ta : tb;
            StateItem* thi = (ta < tb) ? tb : ta;
            if (tlo == lo && thi == hi){
                if (ta == lo && tb == hi) dirLoHi.push_back(t);
                else                      dirHiLo.push_back(t);
            }
        }
    }

    auto byId = [](const TransitionItem* x, const TransitionItem* y){
        return x->id() < y->id();
    };
    std::sort(dirLoHi.begin(), dirLoHi.end(), byId);
    std::sort(dirHiLo.begin(), dirHiLo.end(), byId);

    const bool hasBothDirs = (!dirLoHi.isEmpty() && !dirHiLo.isEmpty());
    const bool isLoHi = (src_ == lo && dst_ == hi);

    const qreal base = 40.0;

    if (hasBothDirs) {
        // Caso BIDIRECIONAL: cada direção num lado.
        // Use offset POSITIVO (0.5, 1.5, 2.5, ...) — o sinal “espelha” sozinho
        // porque o normal n inverte quando a direção inverte.
        int idx = isLoHi ? dirLoHi.indexOf(this) : dirHiLo.indexOf(this);
        qreal magnitude = base * (idx + 0.5); // 0.5, 1.5, 2.5, ...
        return magnitude; // SEM sinal aqui!
    } else {
        // Caso UNIDIRECIONAL: distribui centrado (… -1, 0, +1 …)
        const auto& v = isLoHi ? dirLoHi : dirHiLo; // só um deles tem elementos
        if (v.size() <= 1) return 0.0;
        int idx = v.indexOf(this);
        return base * (idx - (v.size()-1)/2.0);
    }
}

void TransitionItem::updatePath(){
    if (!src_ || !dst_) return;
    const QPointF A = src_->scenePos();
    const QPointF B = dst_->scenePos();

    // raios aproximados (estados como círculos)
    const qreal rSrc = 0.5 * std::max(src_->boundingRect().width(),  src_->boundingRect().height());
    const qreal rDst = 0.5 * std::max(dst_->boundingRect().width(),  dst_->boundingRect().height());

    QPainterPath p;
    QPointF p0, p1, p2, p3;

    if (isSelfLoop()){
        // usa a geometria distribuída (ângulos diferentes p/ cada loop)
        selfLoopGeometry(p0, p1, p2, p3);

        startPt_ = p0;
        endPt_   = p3;

        p.moveTo(p0);
        p.cubicTo(p1, p2, p3);
    } else {
        // ===== aresta curva entre dois estados distintos (como já estava)
        QLineF line(A,B);
        const qreal len = line.length();
        if (len < 1e-3) return;

        const QPointF u = (B - A) / len;
        const QPointF n = QPointF(-u.y(), u.x());

        startPt_ = A + u * rSrc;
        endPt_   = B - u * rDst;

        const qreal off = computeParallelOffset();
        const QPointF mid  = (startPt_ + endPt_) * 0.5;
        const QPointF ctrl = mid + n * off;

        p0 = startPt_;
        p1 = ctrl;          // duplica para Bezier cúbica suave
        p2 = ctrl;
        p3 = endPt_;

        p.moveTo(p0);
        p.cubicTo(p1, p2, p3);
    }


    // cabeça da seta com base na tangente em t=1
    QPointF tan1 = cubicTangent(p0,p1,p2,p3, 1.0);
    if (std::hypot(tan1.x(), tan1.y()) < 1e-6) tan1 = (p3 - p2);
    QPointF dir = tan1 / std::hypot(tan1.x(), tan1.y());
    QPointF nor(-dir.y(), dir.x());

    const qreal headLen = 14.0;
    const qreal headW   = 10.0;
    QPointF base = p3 - dir * headLen;
    QPointF h1   = base + nor * (headW * 0.5);
    QPointF h2   = base - nor * (headW * 0.5);

    // guarde o triângulo para pintar depois, separado
    headPoly_.clear();
    headPoly_ << p3 << h1 << h2;

    // Para o boundingRect/seleção, você pode manter a path "completa"
    QPainterPath all = p;
    all.addPolygon(headPoly_);
    setPath(all);

    // rótulo no meio da curva
    const QPointF midC = cubicPoint(p0,p1,p2,p3, 0.5);
    auto b = text_->boundingRect();

    if (isSelfLoop()) {
        // posiciona o texto alinhado ao centro do estado, um pouco abaixo do MEU arco
        int total = 0;
        int idx   = selfLoopIndexAndCount(total);

        const QPointF C = src_->scenePos();
        const qreal labelBelowArc = 14.0;     // distância do texto em relação ao arco
        // usamos a altura do MEU arco via midC.y(); como cada loop tem loopOut diferente,
        // midC.y() já cresce em degraus com idx
        const qreal x = C.x() - b.width()/2.0;
        const qreal y = midC.y() + labelBelowArc;

        text_->setPos(x, y);
    } else {
        text_->setPos(midC.x() - b.width()/2.0, midC.y() - b.height() - 6.0);
    }
}

void TransitionItem::updateLabel(){
    QString t;
    if (!label_.isEmpty()) t += label_ + "  ";
    t += QString("[%1] ").arg(priority_) +
         (guard_.isEmpty() ? "true" : guard_) +
         " / " +
         (action_.isEmpty() ? "/*no-op*/" : action_);
    text_->setText(t);
    // posição recalculada em updatePath()
}

void TransitionItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e){
    QWidget* parentWidget = nullptr;
    if (scene() && !scene()->views().isEmpty()) parentWidget = scene()->views().first();
    TransitionEditorDialog dlg(parentWidget);
    dlg.setValues(priority_, guard_, action_, label_);
    if (dlg.exec()==QDialog::Accepted){
        setPriority(dlg.priority());
        setGuard(dlg.guard());
        setAction(dlg.action());
        setLabel(dlg.label());
    }
    QGraphicsPathItem::mouseDoubleClickEvent(e);
}

void TransitionItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e){
    setSelected(true); // dá feedback visual
    QMenu menu;
    QAction* actEdit = menu.addAction("Editar…");
    QAction* actDel  = menu.addAction("Excluir");
    QAction* chosen  = menu.exec(e->screenPos());
    if (!chosen) return;

    if (chosen == actEdit){
        QWidget* parentWidget = nullptr;
        if (scene() && !scene()->views().isEmpty()) parentWidget = scene()->views().first();
        TransitionEditorDialog dlg(parentWidget);
        dlg.setValues(priority_, guard_, action_, label_);
        if (dlg.exec()==QDialog::Accepted){
            setPriority(dlg.priority());
            setGuard(dlg.guard());
            setAction(dlg.action());
            setLabel(dlg.label());
        }
    } else if (chosen == actDel){
        if (scene()) scene()->removeItem(this);
        delete this;
    }
    e->accept();
}

void TransitionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    // 1) curva sem brush (nada de fill)
    QPen pen = this->pen();
    if (isSelected()) pen.setWidthF(pen.widthF() + 1.0);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);        // <-- SEM preenchimento na curva
    painter->drawPath(path());             // path inclui curva + polígono, mas brush é NoBrush

    // 2) cabeça da seta preenchida
    painter->setBrush(this->brush());      // normalmente preto
    painter->drawPolygon(headPoly_);

    // Não chame QGraphicsPathItem::paint(), para não redesenhar com preenchimento.
}

int TransitionItem::selfLoopIndexAndCount(int& total) const {
    total = 0;
    if (!scene() || !isSelfLoop()) return -1;
    QVector<const TransitionItem*> siblings;
    for (QGraphicsItem* gi : scene()->items()){
        if (auto t = dynamic_cast<TransitionItem*>(gi)){
            if (t->src()==src_ && t->dst()==dst_) siblings.push_back(t);
        }
    }
    std::sort(siblings.begin(), siblings.end(),
              [](const TransitionItem* a, const TransitionItem* b){ return a->id() < b->id(); });
    total = siblings.size();
    return siblings.indexOf(this);
}

void TransitionItem::selfLoopGeometry(QPointF& p0, QPointF& p1, QPointF& p2, QPointF& p3) const {
    // centro do estado
    const QPointF C = src_->scenePos();
    const qreal r   = 0.5 * std::max(src_->boundingRect().width(),
                                     src_->boundingRect().height());

    int total = 0;
    int idx   = selfLoopIndexAndCount(total);
    // separação angular ainda simétrica (usa k)
    const qreal spacingDeg = 22.5;
    const qreal k = (idx >= 0 && total>0) ? (idx - (total-1)/2.0) : 0.0;
    const qreal theta = -90.0 + k * spacingDeg;

    // abertura do arco na borda e afastamento
    const qreal halfArcDeg = 16.0;                     // meia abertura
    // NOVO: base + passo por loop (mais externo a cada loop)
    const qreal baseOut = std::max<qreal>(0.9*r, 42.0); // raio do 1º loop
    const qreal stepOut = 32.0;                         // quanto empurrar p/ cada novo
    const qreal loopOut = baseOut + stepOut * std::max(0, idx);

    auto onCircle = [&](qreal deg, qreal radMore = 0.0){
        const qreal rad = qDegreesToRadians(deg);
        return QPointF(C.x() + (r+radMore)*std::cos(rad),
                       C.y() - (r+radMore)*std::sin(rad)); // eixo Y p/ baixo em Qt
    };

    // pontos na borda (entrada/saída) ao redor de theta
    p0 = onCircle(theta + halfArcDeg, 0.0);
    p3 = onCircle(theta - halfArcDeg, 0.0);

    // direção "para fora" do centro no ângulo theta
    const qreal radTh = qDegreesToRadians(theta);
    const QPointF normalOut(std::cos(radTh), -std::sin(radTh));
    const QPointF tangent(-normalOut.y(), normalOut.x());

    // ponto de controle "fora" do estado
    const QPointF ctrl = C + normalOut * (r + loopOut);

    // dois controles simétricos com leve tangente para dar volume
    const qreal side = 0.35 * r;
    p1 = ctrl + tangent * (+side);
    p2 = ctrl + tangent * (-side);
}
