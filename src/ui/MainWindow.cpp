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
#define CURRENT_VERSION "0.1beta"
#define UPDATE_FETCH_URL "http://www.filesencrypt.com/update/current.xml"

QMutex MainWindow::ENCRYTPT_MUTEX;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_update{CURRENT_VERSION, UPDATE_FETCH_URL}
{
    ui->setupUi(this);

    show();

    m_settings = new QSettings;

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
        m_update.showUpdateDialogIfUpdateAvailable(false, true, this);
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
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, [this](const QPoint &p){
        Q_UNUSED(p);
        m_listRowMenu->exec(QCursor::pos());
    });
    m_listRowMenu->addAction(openDir);
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

    if(!beSureKeyIsSelectedAndValid([this]{displayKey(false);}, forceAsk)) return;

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
    EVP_cleanup();
    QApplication::quit();
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

MainWindow::~MainWindow(){
    delete m_progress;
    delete m_choose_key;
    delete m_filesEncrypt;
    delete ui;
}

void MainWindow::addWhateverToList(QStringList const& items){
    foreach(const auto& item, items){
        addWhateverToList(item);
    }
}

void MainWindow::guessEncryptedFinished(QFutureWatcher<QPAIR_CRYPT_DEF>* watcher, CryptInfos const &item) const{
    // Retrieve results
    QList<QPAIR_CRYPT_DEF> res{watcher->future().results()};

    size_t length = res.length();
    unsigned crypted = 0;
    unsigned uncrypted = 0;

    foreach(auto i, res){
        *(item.files[i.first]) = i.second;
        if(i.second == EncryptDecrypt::ENCRYPT){
            ++crypted;
        }else if(i.second == EncryptDecrypt::DECRYPT){
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

    *item.state = action;

    switch(action){
        case EncryptDecrypt::ENCRYPT:
        item.encryptedItem->setText("Oui");
            break;
        case EncryptDecrypt::DECRYPT:
        item.encryptedItem->setText("Non");
            break;
    }
}

QPAIR_CRYPT_DEF MainWindow::guessEncrypted(QString const& file){
    QFileInfo fInfo(file);
    QFile f(fInfo.absoluteFilePath());
    f.open(QFile::ReadOnly);
    auto res = FilesEncrypt::guessEncrypted(f);
    if(res.state == EncryptDecrypt::ENCRYPT){
        Logger::info("File " + fInfo.absoluteFilePath() + " is encrypted");
    }
    return QPAIR_CRYPT_DEF{file, res.state};
}

finfo_s MainWindow::encrypt(QString const &file, EncryptDecrypt action, EncryptDecrypt* current_action) const{
    QFile f(file);
    finfo_s res;
    res.success = false;
    if(f.open(QFile::ReadWrite)){
        res = m_filesEncrypt->encryptFile(&f, action);
        *current_action = action;
        f.close();
    }else{
        Logger::error("Can't open file " + file);
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
        QMap<QString, EncryptDecrypt*> filesAndState;

        // Create the watcher
        QFutureWatcher<QPAIR_CRYPT_DEF>* watcher = new QFutureWatcher<QPAIR_CRYPT_DEF>;

        // Store items to re-use them later
        infos.encryptedItem = encryptedVal;
        infos.nameItem  = nameItem;
        infos.sizeItem = sizeItem;
        infos.typeItem = typeItem;
        infos.isFile = false;
        infos.watcher = nullptr;
        infos.recursiveWatcher = nullptr;

        // Method called when we know the crypt state of each file
        connect(watcher, &QFutureWatcher<QPAIR_CRYPT_DEF>::finished, [this, watcher, item](){

            if(!watcher->isCanceled()){
                auto &infos{m_dirs[item]};
                guessEncryptedFinished(watcher, infos);
                infos.watcher = nullptr;
            }

            watcher->deleteLater();
        });

        if(info.isDir()){


            QFutureWatcher<FilesAndSize>* watcherRecursiveFilesDiscover = new QFutureWatcher<FilesAndSize>;
            infos.recursiveWatcher = watcherRecursiveFilesDiscover;

            connect(watcherRecursiveFilesDiscover, &QFutureWatcher<FilesAndSize>::finished, [this, item, watcher, watcherRecursiveFilesDiscover](){

                if(!watcherRecursiveFilesDiscover->isCanceled()){
                    CryptInfos &infos{m_dirs[item]};
                    FilesAndSize res{watcherRecursiveFilesDiscover->result()};
                    infos.sizeItem->setText(utilities::speed_to_human(res.size));
                    QStringList &files = res.files;
                    foreach(auto const& file, files){
                        infos.files[file] = new EncryptDecrypt{NOT_FINISHED};
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

        }else{
            infos.isFile = true;
            // Add the single file to the list
            filesAndState[info.absoluteFilePath()] = new EncryptDecrypt{NOT_FINISHED};
            // Show the type
            typeItem->setText("Fichier");
            sizeItem->setText(utilities::speed_to_human(info.size()));

            QFuture<QPAIR_CRYPT_DEF> future = QtConcurrent::mapped(filesAndState.keys(), &MainWindow::guessEncrypted);
            watcher->setFuture(future);
        }

        infos.files = filesAndState;
        infos.state = new EncryptDecrypt(NOT_FINISHED);

        // Attach the watcher
        infos.watcher = watcher;

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

        // For each file in the dir, or one file
        for(QMap<QString, EncryptDecrypt*>::const_iterator it = item.files.begin(); it != item.files.end(); ++it) {
            ++items_number;
            QFileInfo f{it.key()};
            QFile ff{it.key()};
            ff.open(QFile::ReadOnly);
            EncryptDecrypt_s state = FilesEncrypt::guessEncrypted(ff);
            // If current state != from what you'd do

            if(*it.value() != action && f.isWritable()){
                if(action == EncryptDecrypt::ENCRYPT){
                    max += f.size();
                }else{
                    max += f.size() - state.offsetBeforeContent;
                }
            }else{
                ++item_does_not_need_action;
            }
            QCoreApplication::processEvents();
        }
    }

    // If at least one op is necessary
    if(item_does_not_need_action != items_number){

        // Show the progress bar and set the max
        m_progress->setFileMax(items_number - item_does_not_need_action);
        m_progress->setMax(max);
        m_progress->show();

        for(QMap<QString, CryptInfos>::iterator it = m_dirs.begin(); it != m_dirs.end(); ++it){

            const QString& key{it.key()};
            CryptInfos& item{it.value()};

            if(*item.state != action){ // Check again and avoid to do any action if it's not needed

                item.encryptedItem->setText("En cours...");

                QFutureWatcher<void>* watcher = new QFutureWatcher<void>;

                QStringList const *l = new QStringList{item.files.keys()};

                connect(watcher, &QFutureWatcher<void>::finished, [this, action, watcher, &item, l](){
                    delete l;
                    encryptFinished(item, action);
                    m_encrypting = false;
                    watcher->deleteLater();
                });

                std::function<void(QString const &)> func = [this, action, &item, key, l](QString const &file){

                    QMutexLocker{&ENCRYTPT_MUTEX}; // Lock this

                    EncryptDecrypt *current_state = item.files[file];
                    if(*current_state != action){

                        finfo_s state = encrypt(file, action, current_state);

                        if(state.success){
                            // Because the filename changed, we delete the concerned file and recreate it with the appropriate name
                            item.files.remove(file);
                            item.files.insert(state.name, current_state);
                            if(item.isFile){
                                item.nameItem->setText(state.name);
                            }
                        }
                    }
                };

                QFuture<void> future = QtConcurrent::map(*l, func);
                watcher->setFuture(future);
                m_progress->encryptionStarted();
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
        }
        if(c.watcher != nullptr){
            c.watcher->cancel();
        }

        m_dirs.remove(getCurrentDir());
        delete c.encryptedItem;
        for(QMap<QString, EncryptDecrypt*>::const_iterator it{c.files.begin()}; it != c.files.end(); ++it){
            delete it.value();
        }
        delete c.nameItem;
        delete c.sizeItem;
        delete c.typeItem;
        delete c.state;

        ui->tableWidget->removeRow(row);
    }
}
