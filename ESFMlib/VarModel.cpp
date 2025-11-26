#include "VarModel.h"
#include <QColor>
#include <algorithm>


bool VarModel::nameExists(const QString& name) const {
    for (const auto& e : rows_) if (e.name == name) return true;
    return false;
}

bool VarModel::addVar(const QString& name, const QVariant& value){
    if (name.trimmed().isEmpty() || nameExists(name)) return false;
    const int r = rows_.size();
    beginInsertRows(QModelIndex(), r, r);
    rows_.push_back({name, value});
    endInsertRows();
    return true;
}

bool VarModel::addEmptyRow(){
    const int r = rows_.size();
    beginInsertRows(QModelIndex(), r, r);
    rows_.push_back({"", QVariant{}}); // usuário edita depois
    endInsertRows();
    return true;
}

bool VarModel::removeRowsByIndices(const QList<int>& rows){
    if (rows.isEmpty()) return false;
    // remover do fim para o começo para não invalidar índices
    auto sorted = rows; std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    bool any=false;
    for (int r : sorted) {
        if (r<0 || r>=rows_.size()) continue;
        beginRemoveRows(QModelIndex(), r, r);
        rows_.removeAt(r);
        endRemoveRows();
        any = true;
    }
    return any;
}

QVariant VarModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row()<0 || index.row()>=rows_.size()) return {};
    const auto& e = rows_[index.row()];

    if (role==Qt::DisplayRole || role==Qt::EditRole){
        if (index.column()==0) return e.name;

        // Mostrar bool como "true"/"false", demais como string
        #if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            if (e.value.typeId() == QMetaType::Bool)
        #else
            if (e.value.type() == QVariant::Bool)
        #endif
        {
            return e.value.toBool() ? "true" : "false";
        }
        return e.value.toString();
    }
    return {};
}


QVariant VarModel::headerData(int section, Qt::Orientation o, int role) const {
    if (role!=Qt::DisplayRole) return {};
    if (o==Qt::Horizontal) return section==0 ? "Nome" : "Valor";
    return section+1;
}

Qt::ItemFlags VarModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    auto f = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return f;
}

bool VarModel::parseValue(const QString& s, QVariant& out){
    const QString t = s.trimmed();
    if (t.compare("true", Qt::CaseInsensitive)==0)  { out = true;  return true; }
    if (t.compare("false", Qt::CaseInsensitive)==0) { out = false; return true; }
    bool ok=false; qlonglong v = t.toLongLong(&ok);
    if (ok) { out = static_cast<qlonglong>(v); return true; }
    // aceita string livre (se quiser restringir a bool/int, troque esta linha por 'return false;')
    out = t;
    return true;
}

bool VarModel::setData(const QModelIndex& index, const QVariant& value, int role){
    if (!index.isValid() || role!=Qt::EditRole) return false;
    auto &e = rows_[index.row()];
    if (index.column()==0){
        const QString newName = value.toString().trimmed();
        if (newName.isEmpty()) return false;
        if (newName != e.name && nameExists(newName)) return false;
        e.name = newName;
    } else {
        QVariant val;
        if (!parseValue(value.toString(), val)) return false;
        e.value = val;
    }
    emit dataChanged(index, index);
    return true;
}

bool VarModel::removeRows(int row, int count, const QModelIndex& parent){
    if (row<0 || count<=0 || row+count>rows_.size()) return false;
    beginRemoveRows(parent, row, row+count-1);
    for (int i=0;i<count;i++) rows_.removeAt(row);
    endRemoveRows();
    return true;
}
