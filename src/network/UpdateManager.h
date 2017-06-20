#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QString>
#include <QObject>
#include <QUrl>
#include "Version.h"
#include "network/Downloader.h"

typedef struct {
    bool newUpdate;
    Version version;
} update_t;

class UpdateManager : public QObject
{

     Q_OBJECT

    public:
        UpdateManager(QString fetchUrl, QString downloadUrl);
        inline void changeFetchUrl(QString const& url) { _fetchUrl = url; }
        inline void changeDownloadUrl(QString const& url) { _downloadUrl = url; }
        QUrl const getFetchUrl() const { return _fetchUrl; }
        void showUpdateDialogIfUpdateAvailable(bool checkBeta = false, bool warnNoUpdate = false, QWidget *parent = nullptr);
    private:
        update_t updateAvailable(bool checkBeta) const;
        void update(const Version &v, QWidget *parent = nullptr);
        QUrl _fetchUrl;
        QUrl _downloadUrl;
        mutable Downloader _nManager;
};

#endif // UPDATEMANAGER_H
