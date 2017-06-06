#include "UpdateManager.h"
#include <QMessageBox>

UpdateManager::UpdateManager(const QString &currentVersion, QString const& fetchUrl) : _version(currentVersion), _fetchUrl(fetchUrl){

}

void UpdateManager::showUpdateDialogIfUpdateAvailable(bool checkBeta, bool warnNoUpdate, QWidget* parent){
    update_t updInfos{updateAvailable(checkBeta)};

    if(updInfos.newUpdate){
        auto response = QMessageBox::information(
            parent,
            QObject::tr("Mise à jour"),
            QObject::tr(qPrintable(QString{"Une mise à jour est disponible (v " + updInfos.version + "), voulez-vous l'installer ?"})),
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
    return upd;
}

void UpdateManager::update(){

}
