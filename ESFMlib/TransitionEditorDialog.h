#pragma once
#include <QDialog>

class QSpinBox;
class QPlainTextEdit;
class QLineEdit;

class TransitionEditorDialog : public QDialog {
    Q_OBJECT
public:
    explicit TransitionEditorDialog(QWidget* parent=nullptr);
    void setValues(int priority, const QString& guard, const QString& action, const QString& label);
    int priority() const;
    QString guard() const;
    QString action() const;
    QString label() const;
private:
    QSpinBox* spPrio_{};
    QPlainTextEdit* edGuard_{};
    QPlainTextEdit* edAction_{};
    QLineEdit* edLabel_{};
};
