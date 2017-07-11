#include <QFileDialog>
#include <QStandardPaths>
#include <QCloseEvent>
#include <QtDebug>
#include <QFuture>
#include <QtConcurrent>
#include <QMessageBox>
#include <QAction>
#include "Logger.h"
#include "FilesEncrypt.h"
#include "MainWindow.h"
#include "ui/ui_MainWindow.h"
#include "utilities.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <cassert>
#include <iostream>

#define BASE_DIR_PARAM_NAME "BASE_DIRECTORY"
#define UPDATE_FETCH_URL "http://www.filesencrypt.com/update/current.xml"
#define UPDATE_DOWNLOAD_URL "http://www.filesencrypt.com/update/"

QMutex MainWindow::s_encryptMutex;
unsigned MainWindow::s_current_guess_encrypted_watchers = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_update{UPDATE_FETCH_URL, UPDATE_DOWNLOAD_URL}
{
    ui->setupUi(this);

    setAcceptDrops(true);

    setWindowTitle(qApp->applicationName() + " v" + Version{qApp->applicationVersion()}.getVersionStr().c_str());

    m_settings = new QSettings;

    updateAvailableButtons();

    ui->filesList->setModel(&m_filesListModel);

    ui->filesList->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    ui->filesList->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    ui->filesList->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::Stretch);
    ui->filesList->setSelectionBehavior(QTableView::SelectRows);

    // Center item
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            qApp->desktop()->availableGeometry()
        )
    );

    m_choose_key = new ChooseKey(&m_filesEncrypt, this);
    connect(m_choose_key, SIGNAL(keyDone()), this, SLOT(keySelected()));
    m_choose_key->setWindowModality(Qt::WindowModal);

    correctResize();

    connect(ui->action_retrieveKey, SIGNAL(triggered(bool)), this, SLOT(displayKey()));
    connect(ui->action_openSettings, SIGNAL(triggered(bool)), this, SLOT(openSettings()));
    connect(ui->action_checkForUpdates, &QAction::triggered, [this]{
        m_update.showUpdateDialogIfUpdateAvailable(m_settings->value("check_beta", SettingsWindow::getDefaultSetting("check_beta")).toBool(), true, this);
    });

    m_statusBarContent = new QLabel{};
    m_statusBarContent->setAlignment(Qt::AlignLeft);
    statusBar()->addPermanentWidget(m_statusBarContent, 1);
    updateStatusBar();
    statusBar()->show();

    m_addWhateverMenu = new QMenu(this);
    QAction* dir = new QAction(tr("Importer des dossiers"), m_addWhateverMenu);
    QAction* file = new QAction(tr("Importer des fichiers"), m_addWhateverMenu);
    connect(dir, SIGNAL(triggered(bool)), this, SLOT(select_dir()));
    connect(file, SIGNAL(triggered(bool)), this, SLOT(select_file()));
    m_addWhateverMenu->addAction(dir);
    m_addWhateverMenu->addAction(file);

    m_listRowMenu = new QMenu(this);
    QAction* openDir = new QAction(tr("Ouvrir le dossier"), m_listRowMenu);
    connect(openDir, SIGNAL(triggered(bool)), this, SLOT(openSelectedRowInDir()));
    ui->filesList->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->tableWidget->acceptDrops();
    connect(ui->filesList, &QTableView::customContextMenuRequested, [this](const QPoint &p){
        Q_UNUSED(p);
        m_listRowMenu->exec(QCursor::pos());
    });
    m_listRowMenu->addAction(openDir);



    show();
    m_update.showUpdateDialogIfUpdateAvailable(m_settings->value("check_beta", SettingsWindow::getDefaultSetting("check_beta")).toBool(), false, this);
}

MainWindow::~MainWindow(){
    delete m_statusBarContent;
    delete m_progress;
    delete m_choose_key;
    delete m_filesEncrypt;
    delete m_settings;
    delete ui;
}

void MainWindow::updateStatusBar(){
    if(m_filesEncrypt == nullptr){
        m_statusBarContent->setText(tr("Clé non chargée"));
    }else if(!m_filesEncrypt->isFileKeyLoaded()){
        m_statusBarContent->setText(tr("Clé aes brute chargée"));
    }else if(m_filesEncrypt->isAesDecrypted()){
        m_statusBarContent->setText(tr("Clé chargée, prêt"));
    }else{
        m_statusBarContent->setText(tr("Clé chargée mais encryptée"));
    }
}

void MainWindow::openSelectedRowInDir(){
    showInGraphicalShell(getCurrentDir());
}

void MainWindow::showInGraphicalShell(const QString &pathIn){
#if defined(Q_OS_WIN)
    openInExplorer(pathIn);
#elif defined(Q_OS_LINUX)
    openInNautilus(pathIn);
#else
    QMessageBox::warning(this, "Not supported yet", "Sorry, but this functionnality is not supported yet for other than Windows and Linux Systems", QMessageBox::Ok);
#endif
}

void MainWindow::openInNautilus(const QString &pathIn){
      QStringList args;

      args << QDir::toNativeSeparators(pathIn);

      QProcess *process = new QProcess(this);
      process->start("nautilus", args);
}

void MainWindow::openInExplorer(const QString &pathIn){
      QStringList args;

      args << "/select," << QDir::toNativeSeparators(pathIn);

      QProcess *process = new QProcess(this);
      process->start("explorer.exe", args);
}

bool MainWindow::beSureKeyIsSelectedAndValid(std::function<void()> func, bool forceAskKey){

    // If a key is properly loaded (decrypted aes can be accessed)
    if(m_filesEncrypt != nullptr && m_filesEncrypt->isAesDecrypted() && (!m_filesEncrypt->isFileKeyLoaded() || !forceAskKey))
        return true;

    // Key is still encrypted
    if(m_filesEncrypt != nullptr){
        bool passOk{false};
        while(!passOk){
            bool ok;
            QString const pass{ChooseKey::askPassword(false, &ok, this)};
            if(!ok) return false;
            m_filesEncrypt->requestAesDecrypt(pass.toStdString(), &passOk);
        }
        return true;
    }else{ // There is no key
        static QMetaObject::Connection oldConnection;
        disconnect(oldConnection);
        oldConnection = connect(m_choose_key, &ChooseKey::keyDone, [this, func]{
            func();
            disconnect(oldConnection);
        });
        m_choose_key->show();
    }

    return false;
}

void MainWindow::updateAvailableButtons(){

    // No key loaded
    if(m_filesEncrypt == nullptr){
        ui->action_saveKey->setEnabled(false);
    }else{
        ui->action_saveKey->setEnabled(true);
    }

    unsigned decrypted{0};
    unsigned encrypted{0};

    // If no entries, all buttons must be greyed out
    if(m_filesListModel.rowCount() == 0){
        ui->remove->setEnabled(false);
        ui->cryptAll->setEnabled(false);
        ui->decryptAll->setEnabled(false);
        return;
    }

    ui->remove->setEnabled(true);

    // if there are workers guessing the entries's state, some entries might have a NOT_FINISHED state so we can't change the buttons state
    if(s_current_guess_encrypted_watchers > 0) return;

    for(auto const& dir : m_filesListModel.getDirs()){
        // If at least one entry is partially encrypted, all the buttons have to be enabled so we can return after
        if(*dir.state == PARTIAL){
            ui->cryptAll->setEnabled(true);
            ui->decryptAll->setEnabled(true);
            return;
        }else if(*dir.state == ENCRYPT){
            encrypted++;
        }else if(*dir.state == DECRYPT){
            decrypted++;
        }else{
            assert(false && "This function must be called only when the state of each entry is known");
        }
    }

    // All are encrypted
    if(decrypted == 0){
        ui->cryptAll->setEnabled(false);
        ui->decryptAll->setEnabled(true);
    }else if(encrypted == 0){
        // All are decrypted
        ui->cryptAll->setEnabled(true);
        ui->decryptAll->setEnabled(false);
    }else{
        // Some are encrypted, some are decrypted, no partial
        ui->cryptAll->setEnabled(true);
        ui->decryptAll->setEnabled(true);
    }

}

void MainWindow::displayKey(bool forceAsk){

    if(!beSureKeyIsSelectedAndValid([this]{displayKey(false);}, forceAsk && (m_filesEncrypt != nullptr && m_filesEncrypt->isFileKeyLoaded()))) return;

    const auto aes = m_filesEncrypt->getAES();
    QString key;
    for(quint8 i(0); i < 32; ++i){
        key += QString{"%1"}.arg(*(aes + i), 2, 16, QChar{'0'}).toUpper() + " ";
    }

    QMessageBox keyBox{this};
    keyBox.setDefaultButton(QMessageBox::Ok);
    keyBox.setText(tr("Votre clé est ") + key);
    keyBox.setWindowTitle("Your key");
    keyBox.setStyleSheet("*{ messagebox-text-interaction-flags: 5 }");
    keyBox.exec();
}

void MainWindow::openSettings(){
    m_settings_window = new SettingsWindow(this);
    connect(m_settings_window, SIGNAL(closed()), this, SLOT(closeSettings()));
    m_settings_window->setWindowModality(Qt::WindowModal);
    m_settings_window->show();
}

void MainWindow::closeSettings(){
    m_settings_window->deleteLater();
}

void MainWindow::keySelected(){
    updateStatusBar();
    if(m_filesEncrypt == nullptr) return;
    connect(m_filesEncrypt, SIGNAL(keyDecrypted()), this, SLOT(updateStatusBar()));
    connect(m_filesEncrypt, SIGNAL(keyEncrypted()), this, SLOT(updateStatusBar()));
    delete m_progress;
    m_progress = new Progress(&m_filesEncrypt, this);
    m_progress->setFixedSize(m_progress->size());
    updateAvailableButtons();
}

void MainWindow::closeEvent(QCloseEvent *event){
    QMainWindow::closeEvent(event);
    if(FilesEncrypt::getPendingCrypt() != 0){
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
    EVP_cleanup();
    QApplication::quit();
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event){
    event->accept();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event){
    event->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event){
    event->accept();
}

/**
 * Called when a/some file(s)/directory(ies) is/are dropped in the mainwindow
 * Adds the entries in the list and process them
 * @brief MainWindow::dropEvent
 * @param event
 */
void MainWindow::dropEvent(QDropEvent *event){
    event->accept();
    if(event->mimeData()->hasUrls()){
        auto urls = event->mimeData()->urls();
        for(QUrl const& url : urls){
            QString urlStr{url.path()};
#ifdef Q_OS_WIN
            urlStr = urlStr.remove(0, 1);
#endif
            addWhateverToList(urlStr);
        }
    }
}

void MainWindow::correctResize(){
    ui->filesList->setColumnWidth(3, ui->filesList->width() * 0.65);
}

void MainWindow::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event);
    correctResize();
}

void MainWindow::showEvent(QShowEvent *event){
    Q_UNUSED(event);
    correctResize();
}

void MainWindow::addWhateverToList(QStringList const& items){
    foreach(const auto& item, items){
        addWhateverToList(item);
    }
}

void MainWindow::guessEncryptedFinished(QFutureWatcher<QPAIR_CRYPT_DEF>* watcher, CryptInfos &item) const{
    // Retrieve results
    QList<QPAIR_CRYPT_DEF> res{watcher->future().results()};

    foreach(auto const i, res){

        EncryptDecrypt_light& fInfo{item.files[i.first]};

        fInfo.offsetBeforeContent = i.second.offsetBeforeContent;
        *(fInfo.state) = i.second.state;
    }

    encryptFinished(item);
}

void MainWindow::encryptFinished(CryptInfos &item) const{

    auto length = item.isFile ? 1 : item.files.size();

    unsigned crypted = 0;
    unsigned uncrypted = 0;

    if(!item.isFile){
        for(auto const& i : item.files){
            if(*(i.state) == EncryptDecrypt::ENCRYPT){
                ++crypted;
            }else if(*(i.state) == EncryptDecrypt::DECRYPT){
                ++uncrypted;
            }
        }
    }else{
        if(*item.state == EncryptDecrypt::ENCRYPT){
            crypted = 1;
        }else if(*item.state == EncryptDecrypt::DECRYPT){
            uncrypted = 1;
        }
    }


    if(crypted == length){
        item.stateStr = tr("Oui");
        *item.state = ENCRYPT;
    }else if(uncrypted == length){
        item.stateStr = tr("Non");
        *item.state = DECRYPT;
    }else{
        item.stateStr = "-";
        *item.state = PARTIAL;
    }
}

QPAIR_CRYPT_DEF MainWindow::guessEncrypted(QString const& file){
    QFileInfo fInfo(file);
    QFile f(fInfo.absoluteFilePath());
    f.open(QFile::ReadOnly);
    auto res = FilesEncrypt::guessEncrypted(f);
    if(res.state == EncryptDecrypt::ENCRYPT){
        Logging::Logger::debug(tr("Le fichier %1 est encrypté").arg(fInfo.absoluteFilePath()));
    }
    return QPAIR_CRYPT_DEF{file, res};
}

finfo_s MainWindow::encrypt(QString const &file, EncryptDecrypt action) const{
    QFile f(file);
    finfo_s res;
    res.success = false;
    if(f.open(QFile::ReadWrite)){
        res = m_filesEncrypt->encryptFile(&f, action, m_settings->value("encrypt_filenames", SettingsWindow::getDefaultSetting("encrypt_filenames")).toBool());
        f.close();
    }else{
        Logging::Logger::error("Can't open file " + file);
    }
    return res;
}

void MainWindow::addWhateverToList(QString const& item){

    if(!item.isEmpty() && !m_filesListModel.getDirs().contains(item)){ // Pre-conditions

        CryptInfos infos; // All the informations about an entry (a complete recursive directory or a file)

        QFileInfo info(item);

        // Will be directly filled with one file if item is not a directory or with a thread if it is a directory
        QMap<QString, EncryptDecrypt_light> filesAndState;

        // Store items to re-use them later
        infos.stateStr = "...";
        infos.name  = item;
        infos.size = "...";
        infos.isFile = false;
        infos.watcher = nullptr;
        infos.recursiveWatcher = nullptr;

        if(info.isDir()){

            // Create the watchers
            QFutureWatcher<QPAIR_CRYPT_DEF>* watcher = new QFutureWatcher<QPAIR_CRYPT_DEF>;
            s_current_guess_encrypted_watchers++;
            QFutureWatcher<FilesAndSize>* watcherRecursiveFilesDiscover = new QFutureWatcher<FilesAndSize>;

            // Attach the watchers
            infos.watcher = watcher;
            infos.recursiveWatcher = watcherRecursiveFilesDiscover;

            connect(watcher, &QFutureWatcher<QPAIR_CRYPT_DEF>::finished, [this, watcher, item](){

                s_current_guess_encrypted_watchers--;

                if(!watcher->isCanceled()){
                    auto &infos{m_filesListModel.getDir(item)};
                    guessEncryptedFinished(watcher, infos);

                    m_filesListModel.update(item);

                    updateAvailableButtons();

                    infos.watcher = nullptr;
                }else{
                    Logging::Logger::debug("Encrypted guess watcher canceled");
                }

                watcher->deleteLater();
            });

            connect(watcherRecursiveFilesDiscover, &QFutureWatcher<FilesAndSize>::finished, [this, item, watcher, watcherRecursiveFilesDiscover](){

                if(!watcherRecursiveFilesDiscover->isCanceled()){
                    CryptInfos &infos{m_filesListModel.getDir(item)};
                    FilesAndSize res{watcherRecursiveFilesDiscover->result()};
                    infos.size = utilities::speed_to_human(res.size);
                    QStringList &files = res.files;
                    foreach(auto const& file, files){
                        EncryptDecrypt_light state;
                        state.offsetBeforeContent = 0; // Will be set in guessEncryptedFinished
                        state.state = new EncryptDecrypt{NOT_FINISHED};
                        infos.files[file] = state;
                    }
                    QFuture<QPAIR_CRYPT_DEF> future = QtConcurrent::mapped(infos.files.keys(), &MainWindow::guessEncrypted);
                    watcher->setFuture(future);
                    infos.recursiveWatcher = nullptr;
                    m_filesListModel.update(item);
                }

                watcherRecursiveFilesDiscover->deleteLater();
            });

            // Add the files to the list
            QFuture<FilesAndSize> future{QtConcurrent::run(FilesEncrypt::getFilesFromDirRecursive, QDir{item})};
            watcherRecursiveFilesDiscover->setFuture(future);

            infos.type = tr("Dossier");
            infos.state = new EncryptDecrypt(NOT_FINISHED);

        }else{
            infos.isFile = true;
            // Add the single file to the list

            QPAIR_CRYPT_DEF guess{guessEncrypted(item)};

            EncryptDecrypt_light state;
            state.offsetBeforeContent = guess.second.offsetBeforeContent;
            state.state = new EncryptDecrypt{guess.second.state};

            filesAndState[info.absoluteFilePath()] = state;
            infos.state = new EncryptDecrypt{guess.second.state};
            encryptFinished(infos);

            // Show the type
            infos.type = tr("Fichier");
            infos.size = utilities::speed_to_human(info.size());
        }

        infos.files = filesAndState;

        m_filesListModel.insert(item, infos);
        m_filesListModel.update(item);

        if(infos.isFile){
            updateAvailableButtons();
        }
    }
}

void MainWindow::select_file()
{
    QStringList list = QFileDialog::getOpenFileNames(
        this,
        tr("Sélectionner des fichiers"),
        get_base_dir()
    );

    if(!list.isEmpty())
        set_base_dir(list[0]);

    addWhateverToList(list);
}

void MainWindow::select_dir()
{
    QString name = QFileDialog::getExistingDirectory(
        this,
        tr("Sélectionner des dossiers"),
        get_base_dir()
    );

    set_base_dir(name);

    addWhateverToList(name);
}

QString MainWindow::get_base_dir() const{
    return m_settings->value(BASE_DIR_PARAM_NAME, QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
}

void MainWindow::set_base_dir(QString const &dir){
    m_settings->setValue(BASE_DIR_PARAM_NAME, dir);
}

void MainWindow::on_importButton_clicked(){
    m_addWhateverMenu->exec(QCursor::pos());
}

void MainWindow::on_decryptAll_clicked(){
    action(EncryptDecrypt::DECRYPT);
}

void MainWindow::on_cryptAll_clicked(){
    action(EncryptDecrypt::ENCRYPT);
}

void MainWindow::action(EncryptDecrypt action){

    if(m_encrypting || m_filesListModel.rowCount() == 0) return;

    if(!beSureKeyIsSelectedAndValid([this, action]{
        this->action(action);
    })) return;

    m_encrypting = true;

    qint64 max{0};
    qint64 items_number{0};
    qint64 item_does_not_need_action{0};

    bool passOk{false};

    // Ask password to decrypt aes if aes has been deleted
    if(!m_filesEncrypt->isAesDecrypted()){
        while(!passOk){
            bool ok;
            QString const pass(ChooseKey::askPassword(false, &ok, this));
            if(!ok) return;
            passOk = m_filesEncrypt->requestAesDecrypt(pass.toStdString());
        }
    }

    for(auto const& item : m_filesListModel.getDirs().values()){

        bool problemWrite{false};

        // For each file in the dir, or one file
        for(QMap<QString, EncryptDecrypt_light>::const_iterator it = item.files.constBegin(); it != item.files.constEnd(); ++it) {
            ++items_number;
            QFileInfo f{it.key()};
            EncryptDecrypt_light const& state{it.value()};
            // If current state != from what you'd do

            if(*state.state != action && f.isWritable()){
                qDebug() << state.offsetBeforeContent;
                if(action == EncryptDecrypt::ENCRYPT){
                    max += f.size();
                }else{
                    max += f.size() - state.offsetBeforeContent;
                }
            }else{
                if(!f.isWritable()){
                    qDebug() << "!!!"  << it.key();
                    problemWrite = true;
                }
                ++item_does_not_need_action;
            }
            QCoreApplication::processEvents();
        }

        if(problemWrite){
            QMessageBox::warning(
                this,
                tr("Write protection"),
                tr("Some files are write protected (perhaps privileges too low), they won't be encrypted/decrypted")
            );
        }
    }

    // If at least one op is necessary
    if(item_does_not_need_action != items_number){

        // Show the progress bar and set the max
        m_progress->setFileMax(items_number - item_does_not_need_action);
        m_progress->setMax(max);

        for(QMap<QString, CryptInfos>::const_iterator it{m_filesListModel.getDirs().constBegin()}; it != m_filesListModel.getDirs().constEnd(); ++it){

            CryptInfos& item{m_filesListModel.getDir(it.key())};
            QString orig_name{it.key()};

            if(*item.state != action){ // Check again and avoid to do any action if it's not needed

                item.stateStr = tr("En cours...");

                QFutureWatcher<void>* watcher = new QFutureWatcher<void>;

                QStringList const *l = new QStringList{item.files.keys()};

                connect(watcher, &QFutureWatcher<void>::finished, [this, watcher, &item, l, orig_name](){
                    delete l;
                    encryptFinished(item);
                    m_encrypting = false;
                    if(FilesEncrypt::getPendingCrypt() == 0){
                        Crypt::setAborted(false);
                        Crypt::setPaused(false);
                        updateAvailableButtons();
                    }

                    m_filesListModel.update(orig_name);

                    watcher->deleteLater();
                });

                std::function<void(QString const &)> func = [this, action, &item](QString const &file){

                    s_encryptMutex.lock(); // Lock this

                    EncryptDecrypt_light state = item.files[file]; // Current infos of the file

                    s_encryptMutex.unlock();

                    if(*state.state != action){

                        finfo_s encrypt_result = encrypt(file, action);

                        *state.state = encrypt_result.state;

                        // If single file, also change global state
                        if(item.isFile)
                            *item.state = encrypt_result.state;

                        if(encrypt_result.success){
                            s_encryptMutex.lock(); // Lock this
                            // Because the filename changed, we delete the concerned file and recreate it with the appropriate name
                            item.files.remove(file);
                            item.files.insert(encrypt_result.name, {state.state, encrypt_result.offsetBeforeContent});
                            if(item.isFile){
                                item.name = encrypt_result.name;
                            }
                            s_encryptMutex.unlock();
                        }
                    }
                };

                QFuture<void> future = QtConcurrent::map(*l, func);
                watcher->setFuture(future);
                m_progress->encryptionStarted();
                m_progress->show();
            }
        }
    }else{
        m_encrypting = false;
    }
}

QString MainWindow::getCurrentDir() const{
    int const row{ui->filesList->currentIndex().row()};
    return m_filesListModel.getDirs().keys().at(row);
}

void MainWindow::on_remove_clicked(){
    int const row{ ui->filesList->currentIndex().row() };
    if(row >= 0){
        CryptInfos const& c{m_filesListModel.getDir(getCurrentDir())};
        if(c.recursiveWatcher != nullptr){
            c.recursiveWatcher->cancel();
            c.recursiveWatcher->waitForFinished();
        }
        if(c.watcher != nullptr){
            c.watcher->cancel();
            c.watcher->waitForFinished();
        }

        for(QMap<QString, EncryptDecrypt_light>::const_iterator it{c.files.constBegin()}; it != c.files.constEnd(); ++it){
            delete it.value().state;
        }
        delete c.state;

        m_filesListModel.remove(getCurrentDir());
        updateAvailableButtons();
    }else if(m_filesListModel.getDirs().size() > 0){
        m_filesListModel.removeLast();
    }
}

void MainWindow::on_action_newKey_triggered(){
    if(FilesEncrypt::getPendingCrypt() > 0) return;
    delete m_filesEncrypt;
    m_filesEncrypt = nullptr;
    beSureKeyIsSelectedAndValid([](){}, false);
}

void MainWindow::on_action_saveKey_triggered(){
    if(beSureKeyIsSelectedAndValid([](){}, true)){
        m_choose_key->saveAESToFile();
    }
}
