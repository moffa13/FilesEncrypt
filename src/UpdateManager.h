#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>

typedef struct {
    bool newUpdate;
    QString version;
} update_t;

class UpdateManager
{
public:
    UpdateManager(QString const& currentVersion, QString const& fetchUrl);
    inline void changeFetchUrl(QString const& url) { _fetchUrl = url; }
    QString const getFetchUrl() const { return _fetchUrl; }
    void showUpdateDialogIfUpdateAvailable(bool checkBeta = false, bool warnNoUpdate = false, QWidget *parent = nullptr);
private:
    update_t updateAvailable(bool checkBeta) const;
    void update();
    QString const _version;
    QString _fetchUrl;
    mutable QNetworkAccessManager _nManager;
};

#endif // UPDATEMANAGER_H
