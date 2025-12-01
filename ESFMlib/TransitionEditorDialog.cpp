#include "TransitionEditorDialog.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QDialogButtonBox>

TransitionEditorDialog::TransitionEditorDialog(QWidget* parent) : QDialog(parent) {
    auto lay = new QFormLayout(this);
    edLabel_ = new QLineEdit; edLabel_->setPlaceholderText("ex.: vend");
    spPrio_  = new QSpinBox; spPrio_->setRange(0, 1'000'000); spPrio_->setValue(1);
    edGuard_ = new QPlainTextEdit;  edGuard_->setPlaceholderText("ex.: btn && credit >= price && stock > 0");
    edAction_= new QPlainTextEdit;  edAction_->setPlaceholderText("ex.: dispense := true; credit := credit - price; stock := stock - 1");

    lay->addRow("Rótulo:", edLabel_);
    lay->addRow("Prioridade:", spPrio_);
    lay->addRow("Guarda g(X,I):", edGuard_);
    lay->addRow("Ação a(X,I,O):", edAction_);

    auto bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    lay->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    setWindowTitle("Editar Transição");
    resize(480, 360);
}
void TransitionEditorDialog::setValues(int p,const QString& g,const QString& a,const QString& l){
    spPrio_->setValue(p); edGuard_->setPlainText(g); edAction_->setPlainText(a); edLabel_->setText(l);
}
int TransitionEditorDialog::priority() const { return spPrio_->value(); }
QString TransitionEditorDialog::guard()  const { return edGuard_->toPlainText(); }
QString TransitionEditorDialog::action() const { return edAction_->toPlainText(); }
QString TransitionEditorDialog::label()  const { return edLabel_->text(); }
