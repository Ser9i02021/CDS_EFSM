#pragma once
#include <QMainWindow>

class QGraphicsView;
class QGraphicsScene;
class QTableView;
class VarModel;
class InputModel;
class OutputModel;  // <-- NOVO

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

private:
    QGraphicsView*  view_  = nullptr;
    QGraphicsScene* scene_ = nullptr;

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
