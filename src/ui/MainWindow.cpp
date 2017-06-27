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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_update{UPDATE_FETCH_URL, UPDATE_DOWNLOAD_URL}
{
    ui->setupUi(this);

    setAcceptDrops(true);

    show();

    setWindowTitle(qApp->applicationName() + " v" + Version{qApp->applicationVersion()}.getVersionStr().c_str());

    m_settings = new QSettings;

    m_update.showUpdateDialogIfUpdateAvailable(m_settings->value("check_beta", SettingsWindow::getDefaultSetting("check_beta")).toBool(), false, this);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(1);
    ui->tableWidget->insertColumn(2);
    ui->tableWidget->insertColumn(3);
    QStringList type;
    type << "Type" << "Taille" << "Encrypté" << "Nom";

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::Stretch);
    ui->tableWidget->setHorizontalHeaderLabels(type);
    ui->tableWidget->update();

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

    m_addWhateverMenu = new QMenu(this);
    QAction* dir = new QAction("Importer des dossiers", m_addWhateverMenu);
    QAction* file = new QAction("Importer des fichiers", m_addWhateverMenu);
    connect(dir, SIGNAL(triggered(bool)), this, SLOT(select_dir()));
    connect(file, SIGNAL(triggered(bool)), this, SLOT(select_file()));
    m_addWhateverMenu->addAction(dir);
    m_addWhateverMenu->addAction(file);

    m_listRowMenu = new QMenu(this);
    QAction* openDir = new QAction("Ouvrir le dossier", m_listRowMenu);
    connect(openDir, SIGNAL(triggered(bool)), this, SLOT(openSelectedRowInDir()));
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->tableWidget->acceptDrops();
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, [this](const QPoint &p){
        Q_UNUSED(p);
        m_listRowMenu->exec(QCursor::pos());
    });
    m_listRowMenu->addAction(openDir);
}

MainWindow::~MainWindow(){
    delete m_progress;
    delete m_choose_key;
    delete m_filesEncrypt;
    delete ui;
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
    if(m_filesEncrypt != nullptr && m_filesEncrypt->isAesDecrypted() && !forceAskKey)
        return true;

    // Key is encrypted but selected
    if(m_filesEncrypt != nullptr){
        bool passOk = false;
        while(!passOk){
            bool ok;
            QString const pass{ChooseKey::askPassword(false, &ok, this)};
            if(!ok) return false;
            m_filesEncrypt->requestAesDecrypt(pass.toStdString(), &passOk);
        }
        return true;
    }else{ // Key is not selected
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

void MainWindow::displayKey(bool forceAsk){

    if(!beSureKeyIsSelectedAndValid([this]{displayKey(false);}, forceAsk && (m_filesEncrypt != nullptr && m_filesEncrypt->isFileKeyLoaded()))) return;

    const auto aes = m_filesEncrypt->getAES();
    QString key;
    for(quint8 i(0); i < 32; ++i){
        key += QString{"%1"}.arg(*(aes + i), 2, 16, QChar{'0'}).toUpper() + " ";
    }

    qApp->setStyleSheet("QMessageBox { messagebox-text-interaction-flags: 5 }");
    QMessageBox::information(this, "Clé", "Votre clé est " + key, QMessageBox::Ok);
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
    m_progress = new Progress(&m_filesEncrypt, this);
    m_progress->setFixedSize(m_progress->size());
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

void MainWindow::dropEvent(QDropEvent *event){
    event->accept();
    if(event->mimeData()->hasUrls()){
        auto urls = event->mimeData()->urls();
        for(QUrl const& url : urls){
            addWhateverToList(url.path().remove(0, 1));
        }
    }
}

void MainWindow::correctResize(){
    ui->tableWidget->setColumnWidth(3, ui->tableWidget->width() * 0.65);
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

    size_t length = res.length();
    unsigned crypted = 0;
    unsigned uncrypted = 0;

    foreach(auto const i, res){

        EncryptDecrypt_light& fInfo{item.files[i.first]};

        fInfo.offsetBeforeContent = i.second.offsetBeforeContent;
        *(fInfo.state) = i.second.state;
        if(i.second.state == EncryptDecrypt::ENCRYPT){
            ++crypted;
        }else if(i.second.state == EncryptDecrypt::DECRYPT){
            ++uncrypted;
        }
    }

    if(crypted == length){
        item.encryptedItem->setText("Oui");
        *item.state = ENCRYPT;
    }else if(uncrypted == length){
        item.encryptedItem->setText("Non");
        *item.state = DECRYPT;
    }else{
        item.encryptedItem->setText("-");
        *item.state = PARTIAL;
    }
}

void MainWindow::encryptFinished(CryptInfos const &item, EncryptDecrypt action) const{

    *(item.state) = action;

    switch(action){
        case EncryptDecrypt::ENCRYPT:
            item.encryptedItem->setText("Oui");
            break;
        case EncryptDecrypt::DECRYPT:
            item.encryptedItem->setText("Non");
            break;
        default:
             item.encryptedItem->setText("-");
    }
}

QPAIR_CRYPT_DEF MainWindow::guessEncrypted(QString const& file){
    QFileInfo fInfo(file);
    QFile f(fInfo.absoluteFilePath());
    f.open(QFile::ReadOnly);
    auto res = FilesEncrypt::guessEncrypted(f);
    if(res.state == EncryptDecrypt::ENCRYPT){
        Logging::Logger::debug("File " + fInfo.absoluteFilePath() + " is encrypted");
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

    if(!m_dirs.contains(item) && !item.isEmpty()){ // Pre-conditions

        // Add a row
        int rCount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rCount);

        // Init items
        QTableWidgetItem* encryptedVal = new QTableWidgetItem("...");
        QTableWidgetItem* nameItem = new QTableWidgetItem(item);
        QTableWidgetItem* sizeItem = new QTableWidgetItem("...");
        QTableWidgetItem* typeItem = new QTableWidgetItem("...");

        ui->tableWidget->setItem(rCount, 2, encryptedVal);
        ui->tableWidget->setItem(rCount, 3, nameItem);
        ui->tableWidget->setItem(rCount, 0, typeItem);
        ui->tableWidget->setItem(rCount, 1, sizeItem);

        CryptInfos infos; // All the informations about an entry (a complete recursive directory or a file)

        QFileInfo info(item);

        // Will be directly filled with one file if item is not a directory or with a thread if it is a directory
        QMap<QString, EncryptDecrypt_light> filesAndState;

        // Store items to re-use them later
        infos.encryptedItem = encryptedVal;
        infos.nameItem  = nameItem;
        infos.sizeItem = sizeItem;
        infos.typeItem = typeItem;
        infos.isFile = false;
        infos.watcher = nullptr;
        infos.recursiveWatcher = nullptr;

        if(info.isDir()){

            // Create the watchers
            QFutureWatcher<QPAIR_CRYPT_DEF>* watcher = new QFutureWatcher<QPAIR_CRYPT_DEF>;
            QFutureWatcher<FilesAndSize>* watcherRecursiveFilesDiscover = new QFutureWatcher<FilesAndSize>;

            // Attach the watchers
            infos.watcher = watcher;
            infos.recursiveWatcher = watcherRecursiveFilesDiscover;

            connect(watcher, &QFutureWatcher<QPAIR_CRYPT_DEF>::finished, [this, watcher, item](){

                if(!watcher->isCanceled()){
                    auto &infos{m_dirs[item]};
                    guessEncryptedFinished(watcher, infos);
                    infos.watcher = nullptr;
                }

                watcher->deleteLater();
            });

            connect(watcherRecursiveFilesDiscover, &QFutureWatcher<FilesAndSize>::finished, [this, item, watcher, watcherRecursiveFilesDiscover](){

                if(!watcherRecursiveFilesDiscover->isCanceled()){
                    CryptInfos &infos{m_dirs[item]};
                    FilesAndSize res{watcherRecursiveFilesDiscover->result()};
                    infos.sizeItem->setText(utilities::speed_to_human(res.size));
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
                }

                watcherRecursiveFilesDiscover->deleteLater();
            });

            // Add the files to the list
            QFuture<FilesAndSize> future{QtConcurrent::run(FilesEncrypt::getFilesFromDirRecursive, QDir{item})};
            watcherRecursiveFilesDiscover->setFuture(future);

            typeItem->setText("Dossier");
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
            encryptFinished(infos, *infos.state);

            // Show the type
            typeItem->setText("Fichier");
            sizeItem->setText(utilities::speed_to_human(info.size()));

        }

        infos.files = filesAndState;

        m_dirs.insert(item, infos);
    }
}

void MainWindow::select_file()
{
    QStringList list = QFileDialog::getOpenFileNames(
        this,
        "Sélectionner des fichiers",
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
        "Sélectionner des dossiers",
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

    if(m_encrypting || m_dirs.isEmpty()) return;

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

    for(auto const& item : m_dirs.values()){

        bool problemWrite{false};

        // For each file in the dir, or one file
        for(QMap<QString, EncryptDecrypt_light>::const_iterator it = item.files.begin(); it != item.files.end(); ++it) {
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
                "Write protection",
                "Some files are write protected (perhaps privileges too low), they won't be encrypted/decrypted"
            );
        }
    }

    // If at least one op is necessary
    if(item_does_not_need_action != items_number){

        // Show the progress bar and set the max
        m_progress->setFileMax(items_number - item_does_not_need_action);
        m_progress->setMax(max);

        for(QMap<QString, CryptInfos>::iterator it = m_dirs.begin(); it != m_dirs.end(); ++it){

            CryptInfos& item{it.value()};

            if(*item.state != action){ // Check again and avoid to do any action if it's not needed

                item.encryptedItem->setText("En cours...");

                QFutureWatcher<void>* watcher = new QFutureWatcher<void>;

                QStringList const *l = new QStringList{item.files.keys()};

                connect(watcher, &QFutureWatcher<void>::finished, [this, action, watcher, &item, l](){
                    delete l;
                    qDebug() << "mdrmdr";
                    encryptFinished(item, action);
                    m_encrypting = false;
                    watcher->deleteLater();
                });

                std::function<void(QString const &)> func = [this, action, &item](QString const &file){

                    s_encryptMutex.lock(); // Lock this

                    EncryptDecrypt_light state = item.files[file]; // Current infos of the file

                    s_encryptMutex.unlock();

                    if(*state.state != action){

                        finfo_s encrypt_result = encrypt(file, action);

                        *state.state = encrypt_result.state;

                        if(encrypt_result.success){
                            s_encryptMutex.lock(); // Lock this
                            // Because the filename changed, we delete the concerned file and recreate it with the appropriate name
                            item.files.remove(file);
                            item.files.insert(encrypt_result.name, {state.state, encrypt_result.offsetBeforeContent});
                            if(item.isFile){
                                item.nameItem->setText(encrypt_result.name);
                            }
                            s_encryptMutex.unlock();
                        }
                    }
                };

                QFuture<void> future = QtConcurrent::map(*l, func);
                watcher->setFuture(future);
                m_progress->encryptionStarted();
                m_progress->exec();
            }
        }
    }else{
        m_encrypting = false;
    }
}

QString MainWindow::getCurrentDir() const{
    int const row{ui->tableWidget->currentRow()};
    return ui->tableWidget->item(row, 3)->text();
}

void MainWindow::on_remove_clicked(){
    int const row = ui->tableWidget->currentRow();
    if(row >= 0){
        CryptInfos c = m_dirs[getCurrentDir()];
        if(c.recursiveWatcher != nullptr){
            c.recursiveWatcher->cancel();
            c.recursiveWatcher->waitForFinished();
        }
        if(c.watcher != nullptr){
            c.watcher->cancel();
            c.watcher->waitForFinished();
        }

        m_dirs.remove(getCurrentDir());
        delete c.encryptedItem;
        for(QMap<QString, EncryptDecrypt_light>::const_iterator it{c.files.begin()}; it != c.files.end(); ++it){
            delete it.value().state;
        }
        delete c.nameItem;
        delete c.sizeItem;
        delete c.typeItem;
        delete c.state;

        ui->tableWidget->removeRow(row);
    }
}
