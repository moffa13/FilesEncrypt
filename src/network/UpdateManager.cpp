#include "UpdateManager.h"
#include <QMessageBox>
#include <QEventLoop>
#include <QDomDocument>
#include <QDomNode>
#include <QSettings>
#include <QApplication>
#include "ui/SettingsWindow.h"
#include "Version.h"

UpdateManager::UpdateManager(QString const& fetchUrl) : _fetchUrl{fetchUrl}, _nManager{_fetchUrl}{}

void UpdateManager::showUpdateDialogIfUpdateAvailable(bool checkBeta, bool warnNoUpdate, QWidget* parent){
    update_t updInfos{updateAvailable(checkBeta)};

    if(updInfos.newUpdate){
        auto response = QMessageBox::information(
            parent,
            QObject::tr("Mise à jour"),
            QObject::tr(qPrintable(QString{"Une mise à jour est disponible (v"} + updInfos.version.getVersionStr().c_str() + "), voulez-vous l'installer ?")),
            QMessageBox::Yes | QMessageBox::No
        );

        if(response == QMessageBox::Yes){
            update();
        }
    }else if (!updInfos.newUpdate && warnNoUpdate) {
        QMessageBox::information(
            parent,
            QObject::tr("Mise à jour"),
            QObject::tr(qPrintable(QString{"Vous possédez déjà la dernière mise à jour."})),
            QMessageBox::Ok
        );
    }
}

update_t UpdateManager::updateAvailable(bool checkBeta) const{
    update_t upd;
    upd.newUpdate = false;

    QEventLoop loop;

    static QMetaObject::Connection connect1;
    static QMetaObject::Connection connect2;

    connect1 = QObject::connect(&_nManager, &Downloader::downloaded, [&upd, this, &loop, checkBeta](QByteArray const& res){

        disconnect(connect1);

        QDomDocument dom{qApp->applicationName()};
        dom.setContent(res);

        QDomElement versioning = dom.documentElement();

        Version beta;
        Version stable;

        try{
            beta = versioning.firstChildElement("beta").toElement().attribute("version");
            stable = versioning.firstChildElement("stable").toElement().attribute("version");
        }catch(std::runtime_error const& ex){
            Q_UNUSED(ex);
            loop.quit();
            return;
        }


        Version current_version{qApp->applicationVersion()};

        if(checkBeta && current_version < beta){
            upd.newUpdate = true;
            upd.version = beta;
        }else if(!checkBeta && current_version < stable){
            upd.newUpdate = true;
            upd.version = stable;
        }

        loop.quit();


    });

    connect2 = QObject::connect(&_nManager, &Downloader::error, [&loop, this](){
        disconnect(connect2);
        loop.quit();
    });

    _nManager.download();

    loop.exec();

    return upd;
}

void UpdateManager::update(Version const& v){
    //TODO
}
