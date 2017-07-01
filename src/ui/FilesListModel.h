#ifndef FILESLISTMODEL_H
#define FILESLISTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "defines.h"

class FilesListModel : public QAbstractTableModel
{
    public:
        FilesListModel();
        int rowCount(const QModelIndex &parent = QModelIndex{}) const;
        int columnCount(const QModelIndex &parent = QModelIndex{}) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        void update(QString const& name);
        void insert(const QString &key, const CryptInfos &infos);
        void remove(const QString &key);
        CryptInfos& getDir(const QString &key);
        inline QMap<QString, CryptInfos> const& getDirs() const { return m_dirs; }
    private:
        QMap<QString, CryptInfos> m_dirs;
        __int64 getIndex(QString const& name) const;
};

#endif // FILESLISTMODEL_H
