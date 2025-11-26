#include "MainWindow.h"
#include "StateItem.h"
#include "VarModel.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QToolBar>
#include <QAction>
#include <QKeySequence>
#include <QRandomGenerator>
#include <QPainter>
#include <QDockWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>
#include "OutputModel.h"
#include "InputModel.h"   // <-- NOVO
#include "VarModel.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Cena e view
    scene_ = new QGraphicsScene(this);
    view_  = new QGraphicsView(scene_, this);
    view_->setRenderHint(QPainter::Antialiasing, true);
    setCentralWidget(view_);

    // Estado inicial
    auto* s1 = new StateItem("S1");
    scene_->addItem(s1);
    s1->setPos(0, 0);
    scene_->setSceneRect(-200, -150, 400, 300);

    // Toolbar
    auto tb = addToolBar("Main");

    // Novo Estado
    auto actNew = tb->addAction("Novo Estado");
    connect(actNew, &QAction::triggered, this, [this](){
        static int count = 1;
        auto* s = new StateItem(QString("S%1").arg(++count));
        scene_->addItem(s);
        const qreal x = QRandomGenerator::global()->bounded(-100, 101);
        const qreal y = QRandomGenerator::global()->bounded(-75, 76);
        s->setPos(x, y);
    });

    // Excluir Estado (Delete)
    auto actDelete = new QAction("Excluir Estado", this);
    actDelete->setShortcut(QKeySequence::Delete);
    actDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(actDelete);
    tb->addAction(actDelete);
    connect(actDelete, &QAction::triggered, this, &MainWindow::deleteSelected);

    // Marcar Inicial (Ctrl+I)
    auto actInitial = new QAction("Marcar Inicial", this);
    actInitial->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    actInitial->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(actInitial);
    tb->addAction(actInitial);
    connect(actInitial, &QAction::triggered, this, &MainWindow::markSelectedAsInitial);

    // Alternar Final (Ctrl+F)
    auto actFinal = new QAction("Alternar Final", this);
    actFinal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    actFinal->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(actFinal);
    tb->addAction(actFinal);
    connect(actFinal, &QAction::triggered, this, &MainWindow::toggleSelectedFinal);

    // ===== NOVO: Dock de Variáveis =====
    auto dockVars = new QDockWidget("Variáveis (modelo EFSM)", this);
    auto pane = new QWidget;
    auto vlay = new QVBoxLayout(pane);
    vlay->setContentsMargins(6,6,6,6);

    // botões adicionar/excluir
    auto hlay = new QHBoxLayout;
    auto btnAdd = new QPushButton("Adicionar");
    auto btnDel = new QPushButton("Excluir");
    hlay->addWidget(btnAdd);
    hlay->addWidget(btnDel);
    hlay->addStretch(1);

    // tabela
    varModel_ = new VarModel(this);
    varTable_ = new QTableView;
    varTable_->setModel(varModel_);
    varTable_->horizontalHeader()->setStretchLastSection(true);
    varTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    varTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    varTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    vlay->addLayout(hlay);
    vlay->addWidget(varTable_);

    pane->setLayout(vlay);
    dockVars->setWidget(pane);
    addDockWidget(Qt::RightDockWidgetArea, dockVars);

    // conexões dos botões
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::addVariable);
    connect(btnDel, &QPushButton::clicked, this, &MainWindow::deleteSelectedVariables);

    // ===== Dock de Inputs =====
    auto dockInputs = new QDockWidget("Inputs (modelo EFSM)", this);
    auto paneIn = new QWidget;
    auto vlayIn = new QVBoxLayout(paneIn);
    vlayIn->setContentsMargins(6,6,6,6);

    // botões adicionar/excluir
    auto hlayIn = new QHBoxLayout;
    auto btnAddIn = new QPushButton("Adicionar");
    auto btnDelIn = new QPushButton("Excluir");
    hlayIn->addWidget(btnAddIn);
    hlayIn->addWidget(btnDelIn);
    hlayIn->addStretch(1);

    // tabela
    inputModel_ = new InputModel(this);
    inputTable_ = new QTableView;
    inputTable_->setModel(inputModel_);
    inputTable_->horizontalHeader()->setStretchLastSection(true);
    inputTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    inputTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    vlayIn->addLayout(hlayIn);
    vlayIn->addWidget(inputTable_);

    paneIn->setLayout(vlayIn);
    dockInputs->setWidget(paneIn);
    addDockWidget(Qt::RightDockWidgetArea, dockInputs);

    // conexões
    connect(btnAddIn, &QPushButton::clicked, this, &MainWindow::addInput);
    connect(btnDelIn, &QPushButton::clicked, this, &MainWindow::deleteSelectedInputs);

    // ===== Dock de Outputs =====
    auto dockOutputs = new QDockWidget("Outputs (modelo EFSM)", this);
    auto paneOut = new QWidget;
    auto vlayOut = new QVBoxLayout(paneOut);
    vlayOut->setContentsMargins(6,6,6,6);

    // botões adicionar/excluir
    auto hlayOut = new QHBoxLayout;
    auto btnAddOut = new QPushButton("Adicionar");
    auto btnDelOut = new QPushButton("Excluir");
    hlayOut->addWidget(btnAddOut);
    hlayOut->addWidget(btnDelOut);
    hlayOut->addStretch(1);

    // tabela
    outputModel_ = new OutputModel(this);
    outputTable_ = new QTableView;
    outputTable_->setModel(outputModel_);
    outputTable_->horizontalHeader()->setStretchLastSection(true);
    outputTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    outputTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    outputTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    vlayOut->addLayout(hlayOut);
    vlayOut->addWidget(outputTable_);

    paneOut->setLayout(vlayOut);
    dockOutputs->setWidget(paneOut);
    addDockWidget(Qt::RightDockWidgetArea, dockOutputs);

    // conexões
    connect(btnAddOut, &QPushButton::clicked, this, &MainWindow::addOutput);
    connect(btnDelOut, &QPushButton::clicked, this, &MainWindow::deleteSelectedOutputs);


}

void MainWindow::deleteSelected(){
    if (!scene_) return;
    const auto items = scene_->selectedItems();
    for (QGraphicsItem* gi : items) {
        if (auto st = dynamic_cast<StateItem*>(gi)) {
            scene_->removeItem(st);
            delete st;
        }
    }
}

void MainWindow::markSelectedAsInitial(){
    if (!scene_) return;
    const auto sel = scene_->selectedItems();
    if (sel.size() != 1) return;
    auto* s = dynamic_cast<StateItem*>(sel.front());
    if (!s) return;

    for (QGraphicsItem* gi : scene_->items())
        if (auto st = dynamic_cast<StateItem*>(gi))
            st->setInitial(false);
    s->setInitial(true);
}

void MainWindow::toggleSelectedFinal(){
    if (!scene_) return;
    const auto sel = scene_->selectedItems();
    if (sel.size() != 1) return;
    if (auto* s = dynamic_cast<StateItem*>(sel.front()))
        s->setFinal(!s->isFinal());
}

// ===== NOVOS slots =====
void MainWindow::addVariable(){
    // Pergunta nome
    bool ok=false;
    const QString name = QInputDialog::getText(this, "Nova variável", "Nome:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || name.isEmpty()) return;
    if (varModel_->nameExists(name)) {
        // já existe: opcionalmente, pode mostrar um aviso; por simplicidade, apenas retorna
        return;
    }

    // Pergunta valor (aceita true/false, número, ou string livre)
    ok=false;
    const QString valueStr = QInputDialog::getText(this, "Nova variável", "Valor:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QVariant parsed;
    // reaproveita o setData do model: insere linha vazia e edita
    if (!varModel_->addVar(name, QVariant())) return;
    // define o valor (usa a mesma lógica do editor de célula)
    const int row = varModel_->rowCount()-1;
    varModel_->setData(varModel_->index(row, 1), valueStr, Qt::EditRole);
}

void MainWindow::deleteSelectedVariables(){
    if (!varTable_) return;
    const auto sel = varTable_->selectionModel()->selectedRows();
    QList<int> rows;
    rows.reserve(sel.size());
    for (const auto& idx : sel) rows.push_back(idx.row());
    varModel_->removeRowsByIndices(rows);
}

void MainWindow::addInput(){
    bool ok=false;
    const QString name = QInputDialog::getText(this, "Novo input", "Nome:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || name.isEmpty()) return;
    if (inputModel_->nameExists(name)) return;

    ok=false;
    const QString valueStr = QInputDialog::getText(this, "Novo input", "Valor:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    if (!inputModel_->addInput(name, QVariant())) return;
    const int row = inputModel_->rowCount()-1;
    inputModel_->setData(inputModel_->index(row, 1), valueStr, Qt::EditRole);
}

void MainWindow::deleteSelectedInputs(){
    if (!inputTable_) return;
    const auto sel = inputTable_->selectionModel()->selectedRows();
    QList<int> rows;
    rows.reserve(sel.size());
    for (const auto& idx : sel) rows.push_back(idx.row());
    inputModel_->removeRowsByIndices(rows);
}

void MainWindow::addOutput(){
    bool ok=false;
    const QString name = QInputDialog::getText(this, "Novo output", "Nome:", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || name.isEmpty()) return;
    if (outputModel_->nameExists(name)) return;

    ok=false;
    const QString valueStr = QInputDialog::getText(this, "Novo output", "Valor:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    if (!outputModel_->addOutput(name, QVariant())) return;
    const int row = outputModel_->rowCount()-1;
    outputModel_->setData(outputModel_->index(row, 1), valueStr, Qt::EditRole);
}

void MainWindow::deleteSelectedOutputs(){
    if (!outputTable_) return;
    const auto sel = outputTable_->selectionModel()->selectedRows();
    QList<int> rows;
    rows.reserve(sel.size());
    for (const auto& idx : sel) rows.push_back(idx.row());
    outputModel_->removeRowsByIndices(rows);
}
