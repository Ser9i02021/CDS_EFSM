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
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QMessageBox>
#include <QStatusBar>   // para statusBar()->showMessage(...)
#include <algorithm>    // std::remove_if, std::sort
#include <cmath>        // std::llround
#include "TransitionItem.h"
#include "OutputModel.h"
#include "InputModel.h"   // <-- NOVO
#include "VarModel.h"
#include "DiagramScene.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Cena e view
    scene_ = new DiagramScene(this);
    view_  = new QGraphicsView(scene_, this);
    view_->setRenderHint(QPainter::Antialiasing, true);
    setCentralWidget(view_);

    // Estado inicial
    auto* s1 = new StateItem("S1");
    scene_->addItem(s1);
    s1->setPos(0, 0);
    scene_->setSceneRect(-200, -150, 400, 300);
    // ↓ novo: se for o primeiro, vira inicial
    makeInitialIfNone(s1);
    if (s1->isInitial()) { currentState_ = s1; currentState_->setActive(true); }

    // Toolbar
    auto tb = addToolBar("Main");
    auto actTrans = tb->addAction("Transição");
    actTrans->setCheckable(true);
    connect(actTrans, &QAction::toggled, this, [this](bool on){
        scene_->setMode(on ? DiagramScene::Mode::AddTransition
                           : DiagramScene::Mode::Select);
    });

    auto actStep = tb->addAction("Step");
    actStep->setShortcut(Qt::Key_F10);
    actStep->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(actStep);
    connect(actStep, &QAction::triggered, this, &MainWindow::stepOnce);

    // Novo Estado
    auto actNew = tb->addAction("Novo Estado");
    connect(actNew, &QAction::triggered, this, [this](){
        static int count = 1;
        auto* s = new StateItem(QString("S%1").arg(++count));
        scene_->addItem(s);
        const qreal x = QRandomGenerator::global()->bounded(-100, 101);
        const qreal y = QRandomGenerator::global()->bounded(-75, 76);
        s->setPos(x, y);

        // se ainda não há inicial, este passa a ser inicial e corrente
        makeInitialIfNone(s);
        if (!currentState_ && s->isInitial()) { currentState_ = s; currentState_->setActive(true); }
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

bool MainWindow::hasInitialState() const {
    if (!scene_) return false;
    for (QGraphicsItem* gi : scene_->items())
        if (auto st = dynamic_cast<StateItem*>(gi))
            if (st->isInitial()) return true;
    return false;
}

void MainWindow::makeInitialIfNone(StateItem* s) {
    if (!s) return;
    if (!hasInitialState()) {
        s->setInitial(true);
        // não mexe em outros, pois é o primeiro
    }
}

StateItem* MainWindow::findInitial() const {
    if (!scene_) return nullptr;
    for (QGraphicsItem* gi : scene_->items())
        if (auto st = dynamic_cast<StateItem*>(gi))
            if (st->isInitial()) return st;
    return nullptr;
}

static inline QJSValue toJsValue(QJSEngine& eng, const QVariant& v){
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    if (v.typeId()==QMetaType::Bool)   return QJSValue(v.toBool());
    if (v.canConvert<long long>())     return QJSValue((double)v.toLongLong());
#else
    if (v.type()==QVariant::Bool)      return QJSValue(v.toBool());
    if (v.canConvert<qlonglong>())     return QJSValue((double)v.toLongLong());
#endif
    return QJSValue(v.toString());
}

bool MainWindow::buildJsContext(QJSEngine& eng){
    auto g = eng.globalObject();

    // Vars (X)
    if (varModel_){
        for (int r=0; r<varModel_->rowCount(); ++r){
            const QString name = varModel_->index(r,0).data().toString();
            const QString sval = varModel_->index(r,1).data().toString();
            QVariant v = sval; // parse como em setData (bool/int/string)
            // tenta bool
            if (sval.compare("true", Qt::CaseInsensitive)==0) v = true;
            else if (sval.compare("false", Qt::CaseInsensitive)==0) v = false;
            else {
                bool ok=false; qlonglong n = sval.toLongLong(&ok);
                if (ok) v = (qlonglong)n;
            }
            g.setProperty(name, toJsValue(eng, v));
        }
    }

    // Inputs (I)
    if (inputModel_){
        for (int r=0; r<inputModel_->rowCount(); ++r){
            const QString name = inputModel_->index(r,0).data().toString();
            const QString sval = inputModel_->index(r,1).data().toString();
            QVariant v = sval;
            if (sval.compare("true", Qt::CaseInsensitive)==0) v = true;
            else if (sval.compare("false", Qt::CaseInsensitive)==0) v = false;
            else { bool ok=false; qlonglong n = sval.toLongLong(&ok); if (ok) v=(qlonglong)n; }
            g.setProperty(name, toJsValue(eng, v));
        }
    }

    // Outputs (O) — disponíveis para leitura e passíveis de escrita pela ação
    if (outputModel_){
        for (int r=0; r<outputModel_->rowCount(); ++r){
            const QString name = outputModel_->index(r,0).data().toString();
            const QString sval = outputModel_->index(r,1).data().toString();
            QVariant v = sval;
            if (sval.compare("true", Qt::CaseInsensitive)==0) v = true;
            else if (sval.compare("false", Qt::CaseInsensitive)==0) v = false;
            else { bool ok=false; qlonglong n = sval.toLongLong(&ok); if (ok) v=(qlonglong)n; }
            g.setProperty(name, toJsValue(eng, v));
        }
    }
    return true;
}

bool MainWindow::jsToBool(const QJSValue& v, bool& ok){
    ok = true;
    if (v.isBool())   return v.toBool();
    if (v.isNumber()) return v.toNumber()!=0.0;
    if (v.isString()){
        const auto s = v.toString().trimmed();
        if (s.compare("true", Qt::CaseInsensitive)==0)  return true;
        if (s.compare("false", Qt::CaseInsensitive)==0) return false;
    }
    ok = false; return false;
}

QString MainWindow::qjsToString(const QJSValue& v){
    if (v.isBool())   return v.toBool() ? "true" : "false";
    if (v.isNumber()) return QString::number((qlonglong)std::llround(v.toNumber()));
    return v.toString();
}


void MainWindow::stepOnce(){
    if (!scene_) return;

    // estado corrente: se ainda não definido, assume o inicial
    if (!currentState_) {
        currentState_ = findInitial();
        if (currentState_) currentState_->setActive(true);
        if (!currentState_) return; // nada a fazer
    }

    // 1) Coletar transições saindo do estado corrente
    struct Cand { int prio; int id; TransitionItem* t; };
    QVector<Cand> candidates;

    for (QGraphicsItem* gi : scene_->items()){
        if (auto t = dynamic_cast<TransitionItem*>(gi)){
            if (t->src() == currentState_){
                candidates.push_back({ t->priority(), t->id(), t });
            }
        }
    }
    if (candidates.isEmpty()) {
        statusBar()->showMessage("Sem transições saindo do estado atual.", 1500);
        return;
    }

    // 2) Avaliar guardas g(X,I)
    QJSEngine eng;
    buildJsContext(eng);
    auto gobj = eng.globalObject();

    auto enabled = candidates;
    enabled.erase(std::remove_if(enabled.begin(), enabled.end(), [&](const Cand& c){
        QString guard = c.t->guard().trimmed();
        if (guard.isEmpty()) guard = "true";
        // Avalia a expressão
        QJSValue res = eng.evaluate(guard);
        if (res.isError()){
            // guarda inválida => trata como false e mostra status
            statusBar()->showMessage(QString("Erro na guarda: %1").arg(res.toString()), 3000);
            return true; // remove (não habilitada)
        }
        bool ok=false; bool b = jsToBool(res, ok);
        return !ok || !b; // remove se !ok ou !b
    }), enabled.end());

    if (enabled.isEmpty()) {
        statusBar()->showMessage("Nenhuma transição habilitada.", 1500);
        return;
    }

    // 3) Escolher menor prioridade (empate: menor id = mais antiga)
    std::sort(enabled.begin(), enabled.end(), [](const Cand& a, const Cand& b){
        if (a.prio != b.prio) return a.prio < b.prio;
        return a.id < b.id;
    });
    TransitionItem* chosen = enabled.front().t;

    // 4) Executar ação a(X,I,O) — atualiza vars/outputs
    QString action = chosen->action().trimmed();
    if (!action.isEmpty()){
        // converte ":=" -> "=" para o JS
        action.replace(":=", "=");
        QJSValue res = eng.evaluate(action);
        if (res.isError()){
            QMessageBox::warning(this, "Erro na ação",
                                 QString("Avaliação da ação falhou:\n%1").arg(res.toString()));
            return; // aborta step para não ficar inconsistente
        }
    }

    // Ler de volta novas valuations para Vars e Outputs e atualizar as tabelas
    if (varModel_){
        for (int r=0; r<varModel_->rowCount(); ++r){
            const QString name = varModel_->index(r,0).data().toString();
            QJSValue v = gobj.property(name);
            varModel_->setData(varModel_->index(r,1), qjsToString(v), Qt::EditRole);
        }
    }
    if (outputModel_){
        for (int r=0; r<outputModel_->rowCount(); ++r){
            const QString name = outputModel_->index(r,0).data().toString();
            QJSValue v = gobj.property(name);
            outputModel_->setData(outputModel_->index(r,1), qjsToString(v), Qt::EditRole);
        }
    }
    // (inputs tipicamente não são alterados por ações; mantemos como estão)

    // 5) Transitar p/ o estado destino
    if (currentState_) currentState_->setActive(false);
    currentState_ = chosen->dst();
    if (currentState_) currentState_->setActive(true);

    statusBar()->showMessage(
        QString("Transição: %1 → %2  [p=%3]")
            .arg(chosen->src()->name())
            .arg(chosen->dst()->name())
            .arg(chosen->priority()),
        2000
    );
}


