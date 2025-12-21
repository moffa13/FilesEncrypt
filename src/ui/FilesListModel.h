#ifndef FILESLISTMODEL_H
#define FILESLISTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "defines.h"


class FilesListModel : public QAbstractTableModel
{

    Q_OBJECT

    public:
        FilesListModel();
        int rowCount(const QModelIndex &parent = QModelIndex{}) const;
        int columnCount(const QModelIndex &parent = QModelIndex{}) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        void update(QString const& name);
        void update(QString const& name, CryptInfos c);
        void insert(const QString &key, const CryptInfos &infos);
        void remove(const QString &key);
        QString removeLast();
        QString get(int index) const;
        CryptInfos& getDir(const QString &key);
private:
        QMap<QString, CryptInfos> m_dirs;
        long long getIndex(QString const& name) const;
};

#endif // FILESLISTMODEL_H
