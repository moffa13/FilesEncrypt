#include "FilesListModel.h"

FilesListModel::FilesListModel(){}

int FilesListModel::rowCount(const QModelIndex &parent) const{
	Q_UNUSED(parent);
	return m_dirs.size();
}

int FilesListModel::columnCount(const QModelIndex &parent) const{
	Q_UNUSED(parent);
	return 4;
}

QVariant FilesListModel::data(const QModelIndex &index, int role) const{
	if (role == Qt::DisplayRole){
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
					return tr("Type");
				case 1:
					return tr("Taille");
				case 2:
                    return tr("Chiffré");
				case 3:
					return tr("Nom");
			}
		}
	}
	return QVariant{};
}
void FilesListModel::update(const QString &name){
    auto row{getIndex(name)};
    Q_EMIT dataChanged(index(row, 0), index(row, 3));
}
void FilesListModel::update(const QString &name, CryptInfos c){
    auto &e = m_dirs[name];
    c.files.clear(); // Remove all recursive infos (from sub files/folders)
    e = c;

    update(name);
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
QString FilesListModel::removeLast(){
    if(m_dirs.isEmpty()) return "-----";
    auto elem = std::prev(m_dirs.constEnd()).key();
    remove(elem);
    return elem;
}

QString FilesListModel::get(int index) const
{
    return std::next(m_dirs.cbegin(), index).key();
}

long long FilesListModel::getIndex(const QString &name) const{
	return std::distance(m_dirs.begin(), m_dirs.lowerBound(name));
}
