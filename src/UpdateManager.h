#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

typedef struct {
    bool newUpdate;
    QString version;
} update_t;

class UpdateManager
{
public:
    UpdateManager(QString const& currentVersion, const QString &fetchUrl);
    void showUpdateDialogIfUpdateAvailable(bool checkBeta = false, bool warnNoUpdate = false, QObject *parent = nullptr);
private:
    update_t updateAvailable(bool checkBeta);
    void update();
    QString _version;
    QString _fetchUrl;
};

#endif // UPDATEMANAGER_H
