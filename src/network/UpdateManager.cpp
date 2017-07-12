#include "UpdateManager.h"
#include <QMessageBox>
#include <QEventLoop>
#include <QDomDocument>
#include <QDomNode>
#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QProcess>
#include "ui/SettingsWindow.h"
#include "Version.h"

UpdateManager::UpdateManager(QString fetchUrl, QString downloadUrl) : _fetchUrl{std::move(fetchUrl)}, _downloadUrl{std::move(downloadUrl)}, _nManager{_fetchUrl}{}

void UpdateManager::showUpdateDialogIfUpdateAvailable(bool checkBeta, bool warnNoUpdate, QWidget* parent){
    update_t updInfos{updateAvailable(checkBeta)};

    if(updInfos.newUpdate){
        auto response = QMessageBox::information(
            parent,
            tr("Mise à jour"),
            tr("Une mise à jour est disponible (v %1), voulez-vous l'installer ?").arg(updInfos.version.getVersionStr().c_str()),
            QMessageBox::Yes | QMessageBox::No
        );

        if(response == QMessageBox::Yes){
            update(updInfos.version, parent);
        }
    }else if (!updInfos.newUpdate && warnNoUpdate) {
        QMessageBox::information(
            parent,
            tr("Mise à jour"),
            tr("Vous possédez déjà la dernière mise à jour."),
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
    disconnect(connect1);
    disconnect(connect2);

    connect1 = QObject::connect(&_nManager, &Downloader::downloaded, [&upd, this, &loop, checkBeta](QByteArray const& res){

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

        disconnect(connect1);


    });

    connect2 = QObject::connect(&_nManager, &Downloader::error, [&loop, this](){
        loop.quit();
        disconnect(connect2);
    });

    _nManager.download();

    loop.exec();

    return upd;
}

void UpdateManager::update(Version const& v, QWidget* parent){

    Downloader *downloader = nullptr;

    //TODO: x64 or x86 folder download
    //TODO: ubuntu folder
    //TODO: debian, OSX, ...
    //TODO: re-add release to github

#ifdef Q_PROCESSOR_X86_64
    QString arch_folder{"x64"};
#else
    QString arch_folder{"x86"};
#endif


#if defined(Q_OS_WIN)
    downloader = new Downloader{_downloadUrl.toString() + v.getVersionStr().c_str() + "/" + arch_folder + "/windows/" + qApp->applicationName() + ".exe"};
#elif defined(Q_OS_LINUX)
    downloader = new Downloader{_downloadUrl.toString() + v.getVersionStr().c_str() + "/" + qApp->applicationName() + "-linux-" + arch_folder + ".AppImage"};
#else
    QMessageBox::critical(parent, tr("Erreur de mise à jour"), tr("Les mises à jour ne sont pas encore possibles sur ce système"), QMessageBox::Ok);
    return;
#endif

    connect(downloader, &Downloader::downloaded, [downloader, parent](QByteArray const& res){
        QFile::rename(qApp->applicationFilePath(), qApp->applicationFilePath() + ".old");
        QFile f{qApp->applicationFilePath()};
        if(!f.open(QFile::ReadWrite)){
            QMessageBox::critical(
                parent,
                tr("Erreur de télechargement"),
                tr("Nous n'avons pas pu installer la mise à jour. Impossible d'écrire dans le fichier"),
                QMessageBox::Ok
            );
        }
        f.write(res);
        f.setPermissions(f.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
        f.close();

        QStringList update_done_arg;
        update_done_arg << "update_done";
        QProcess::startDetached(qApp->applicationFilePath(), update_done_arg);

        downloader->deleteLater();
        qApp->exit(0);
    });

    connect(downloader, &Downloader::error, [downloader, parent](){
         QMessageBox::critical(
             parent,
             tr("Erreur de télechargement"),
             tr("Nous n'avons pas pu installer la mise à jour. Veuillez réessayer plus tard"),
             QMessageBox::Ok
         );
         downloader->deleteLater();
    });
    downloader->download();
}
