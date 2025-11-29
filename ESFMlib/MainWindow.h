#pragma once
#include <QMainWindow>

class QGraphicsView;
class DiagramScene; // <-- em vez de QGraphicsScene
class QTableView;
class VarModel;
class InputModel;
class OutputModel;  // <-- NOVO
class QAction;
class StateItem;
class TransitionItem;

class StateItem;   // forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void saveModel();     // <-- NOVO
    void openModel();

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

    void stepOnce();
    void editSelectedTransition();

private:
    bool hasInitialState() const;
    void makeInitialIfNone(StateItem* s);
    StateItem* findInitial() const;

    // helpers do Step
    bool buildJsContext(class QJSEngine& eng);               // carrega X,I,O
    static bool jsToBool(const class QJSValue& v, bool& ok); // conversões
    static QString qjsToString(const class QJSValue& v);

    // Helpers p/ seleção (NOVO)
    TransitionItem* selectedTransition() const;
    void updateActionsEnabled();

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

    QAction* actEditTransition_ = nullptr;

    // helpers de (de)serialização
    QJsonObject toJson() const;                 // <-- NOVO
    bool loadFromJsonObject(const QJsonObject&);// <-- NOVO
    static QJsonValue encodeJsonValue(const QString& s); // str->json tipado
    static QString    decodeJsonValue(const QJsonValue& v); // json->str

    void clearSceneAndTables();
};
