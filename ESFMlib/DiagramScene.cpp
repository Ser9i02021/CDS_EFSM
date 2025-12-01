#include "DiagramScene.h"
#include "StateItem.h"
#include "TransitionItem.h"
#include "TransitionEditorDialog.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QGraphicsView>   // <- necessário para scene()->views().first()
#include <QWidget>         // opcional, só para o tipo QWidget*

DiagramScene::DiagramScene(QObject* parent) : QGraphicsScene(parent) {}

void DiagramScene::setMode(Mode m){
    mode_ = m;
    if (tempLine_) { removeItem(tempLine_); delete tempLine_; tempLine_ = nullptr; }
    pendingSrc_ = nullptr;
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent* e){
    if (mode_ == Mode::AddTransition){
        pendingSrc_ = stateAt(e->scenePos());
        if (pendingSrc_){
            tempLine_ = addLine(QLineF(e->scenePos(), e->scenePos()),
                                QPen(Qt::darkGray, 1, Qt::DashLine));
            return; // não passa para base
        }
    }
    QGraphicsScene::mousePressEvent(e);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e){
    if (tempLine_){
        auto l = tempLine_->line();
        l.setP2(e->scenePos());
        tempLine_->setLine(l);
        return;
    }
    QGraphicsScene::mouseMoveEvent(e);
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e){
    if (tempLine_){
        removeItem(tempLine_); delete tempLine_; tempLine_ = nullptr;

        StateItem* dst = stateAt(e->scenePos());

        // Fallback: se não achou um estado no ponto final, mas o usuário soltou
        // dentro (ou muito perto) do estado de origem, considere self-loop:
        if (pendingSrc_ && !dst) {
            // testa “containment” no sistema de coordenadas do StateItem
            const QPointF local = pendingSrc_->mapFromScene(e->scenePos());
            if (pendingSrc_->contains(local)) {
                dst = pendingSrc_;
            }
        }

        // >>> Agora PERMITIMOS self-loop (dst == pendingSrc_)
        if (pendingSrc_ && dst){
            auto* t = new TransitionItem(pendingSrc_, dst);
            addItem(t);
            t->updatePath();

            // Reposiciona todos os self-loops desse estado
            if (pendingSrc_ == dst) {
                for (QGraphicsItem* gi : items()){
                    if (auto tr = dynamic_cast<TransitionItem*>(gi)){
                        if (tr->src() == pendingSrc_ && tr->dst() == dst) {
                            tr->updatePath(); // recomputa ângulo/controle conforme nova contagem
                        }
                    }
                }
            }

            // Abre o editor para preencher prioridade/guarda/ação/label
            QWidget* parentWidget = nullptr;
            if (!views().isEmpty()) parentWidget = views().first();

            TransitionEditorDialog dlg(parentWidget);
            dlg.setValues(1, "true", "", "");
            if (dlg.exec()==QDialog::Accepted){
                t->setPriority(dlg.priority());
                t->setGuard(dlg.guard());
                t->setAction(dlg.action());
                t->setLabel(dlg.label());
            }
        }

        pendingSrc_ = nullptr;
        return;
    }
    QGraphicsScene::mouseReleaseEvent(e);
}


StateItem* DiagramScene::stateAt(const QPointF& p) const{
    for (auto* it : items(p)) if (auto s = dynamic_cast<StateItem*>(it)) return s;
    return nullptr;
}
