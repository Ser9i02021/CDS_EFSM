#pragma once
#include <QMainWindow>

class QGraphicsView;
class DiagramScene; // <-- em vez de QGraphicsScene
class QTableView;
class VarModel;
class InputModel;
class OutputModel;  // <-- NOVO

class StateItem;   // forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void deleteSelected();
    void markSelectedAsInitial();
    void toggleSelectedFinal();

    // Variáveis
    void addVariable();
    void deleteSelectedVariables();

    // Inputs
    void addInput();
    void deleteSelectedInputs();

    // Outputs (NOVO)
    void addOutput();
    void deleteSelectedOutputs();

    void stepOnce();                   // <- NOVO

private:
    bool hasInitialState() const;
    void makeInitialIfNone(StateItem* s);
    StateItem* findInitial() const;

    // helpers do Step (NOVO)
    bool buildJsContext(class QJSEngine& eng);               // carrega X,I,O
    static bool jsToBool(const class QJSValue& v, bool& ok); // conversões
    static QString qjsToString(const class QJSValue& v);

    QGraphicsView*  view_  = nullptr;
    DiagramScene*   scene_ = nullptr; // <-- trocado

    // NOVO: ponteiro do estado corrente
    StateItem* currentState_ = nullptr;

    // Variáveis
    QTableView* varTable_ = nullptr;
    VarModel*   varModel_ = nullptr;

    // Inputs
    QTableView* inputTable_ = nullptr;
    InputModel* inputModel_ = nullptr;

    // Outputs (NOVO)
    QTableView* outputTable_ = nullptr;
    OutputModel* outputModel_ = nullptr;
};
