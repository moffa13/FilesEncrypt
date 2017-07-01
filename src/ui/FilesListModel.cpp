#include "FilesListModel.h"

FilesListModel::FilesListModel(){}

int FilesListModel::rowCount(const QModelIndex &parent) const{
    return m_dirs.size();
}

int FilesListModel::columnCount(const QModelIndex &parent) const{
    return 4;
}

QVariant FilesListModel::data(const QModelIndex &index, int role) const{
    if (role == Qt::DisplayRole){
        int a = index.row();
        auto b = m_dirs.values();
        auto const& vals = m_dirs.values();
        auto const& infos{vals[index.row()]};
        switch(index.column()){
            case 0:
                return infos.type;
            case 1:
                return infos.size;
            case 2:
                return infos.stateStr;
            case 3:
                return infos.name;
        }
    }
    return QVariant{};
}

QVariant FilesListModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role == Qt::DisplayRole){
        if(orientation == Qt::Horizontal){
            switch(section){
                case 0:
                    return QString{"Type"};
                case 1:
                    return QString{"Taille"};
                case 2:
                    return QString{"Encrypté"};
                case 3:
                    return QString{"Nom"};
            }
        }
    }
    return QVariant{};
}

void FilesListModel::update(const QString &name){
    auto row{getIndex(name)};
    emit dataChanged(index(row, 0), index(row, 3));
}

void FilesListModel::insert(QString const& key, CryptInfos const& infos){
    beginInsertRows(QModelIndex{}, rowCount(), rowCount());
    m_dirs.insert(key, infos);
    endInsertRows();
}

void FilesListModel::remove(const QString &key){
    beginRemoveRows(QModelIndex{}, getIndex(key), getIndex(key));
    m_dirs.remove(key);
    endRemoveRows();
}

CryptInfos &FilesListModel::getDir(const QString &key){
    return m_dirs[key];
}

long long FilesListModel::getIndex(const QString &name) const{
    return std::distance(m_dirs.begin(), m_dirs.lowerBound(name));
}
