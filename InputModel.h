#pragma once
#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>

// Model simples: cada linha = {name, value}
class InputModel : public QAbstractTableModel {
public:
    struct Entry { QString name; QVariant value; };

    explicit InputModel(QObject* parent=nullptr)
        : QAbstractTableModel(parent) {}

    // API
    bool addInput(const QString& name, const QVariant& value);
    bool addEmptyRow(); // para inserir e editar inline
    bool removeRowsByIndices(const QList<int>& rows);
    bool nameExists(const QString& name) const;
    QVector<Entry> entries() const { return rows_; }

    // QAbstractTableModel
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { Q_UNUSED(parent); return rows_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { Q_UNUSED(parent); return 2; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    static bool parseValue(const QString& s, QVariant& out); // bool, int, senão string
    QVector<Entry> rows_;
};
