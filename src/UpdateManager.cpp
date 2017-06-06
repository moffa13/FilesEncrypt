#include "UpdateManager.h"
#include <QMessageBox>

UpdateManager::UpdateManager(const QString &currentVersion, const QString& fetchUrl) : _version(currentVersion), _fetchUrl(fetchUrl){

}

void UpdateManager::showUpdateDialogIfUpdateAvailable(bool checkBeta, bool warnNoUpdate, QObject* parent){
    update_t updInfos{updateAvailable(checkBeta)};

    if(updInfos.newUpdate){
        auto response = QMessageBox::information(
            parent,
            tr("Mise à jour"),
            tr(QString{"Une mise a jour est disponible (v " + updInfos.version + "), voulez-vous l'installer ?"}),
            QMessageBox::Yes | QMessageBox::No
        );

        if(response == QMessageBox::Yes){
            update();
        }
    }else if (!updInfos.newUpdate && warnNoUpdate) {
        QMessageBox::information(
            parent,
            tr("Mise à jour"),
            tr(QString{"Vous possédez déjà la dernière mise à jour."}),
            QMessageBox::Ok
        );
    }
}

update_t UpdateManager::updateAvailable(bool checkBeta){
    update_t upd;
    upd.newUpdate = false;
    return upd;
}

void UpdateManager::update(){

}
