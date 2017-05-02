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
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    show();

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(1);
    ui->tableWidget->insertColumn(2);
    QStringList type;
    type << "Type" << "Encrypté" << "Nom";

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    ui->tableWidget->setHorizontalHeaderLabels(type);
    ui->tableWidget->update();

    //m_progress->show();

    m_choose_key = new ChooseKey(&m_filesEncrypt, this);
    connect(m_choose_key, SIGNAL(keyDone()), this, SLOT(keySelected()));
    m_choose_key->setWindowModality(Qt::WindowModal);

    m_choose_key->show();


    correctResize();

    //menuBar()->show();

    connect(ui->actionObtenir_la_cl, SIGNAL(triggered(bool)), this, SLOT(displayKey()));


    //connect(m_choose_key, SIGNAL())


}

void MainWindow::displayKey(){
    bool passOk = false;
    while(!passOk){
        bool ok;
        QString const pass(ChooseKey::askPassword(false, &ok, this));
        if(!ok) return;
        m_filesEncrypt->requestAesDecrypt(pass.toStdString(), &passOk);
    }

    const auto aes = m_filesEncrypt->getAES();
    QString key;
    for(unsigned i(0); i < 32; ++i){
        key += QString::number((int)(*(aes + i))) + " ";
    }

    QMessageBox::information(this, "Clé", "Votre clé est " + key, QMessageBox::Ok);

}

void MainWindow::keySelected(){
    m_progress = new Progress(&m_filesEncrypt);
    m_progress->setFixedSize(m_progress->size());
}

void MainWindow::closeEvent(QCloseEvent *event){
    QMainWindow::closeEvent(event);
    QApplication::quit();

}

void MainWindow::correctResize(){
    ui->tableWidget->setColumnWidth(2, ui->tableWidget->width() * 0.7);
}

void MainWindow::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event);
    correctResize();
}

void MainWindow::showEvent(QShowEvent *event){
    Q_UNUSED(event);
    correctResize();
}

MainWindow::~MainWindow()
{
    if(m_progress != NULL){
        delete m_progress;
    }
    delete m_choose_key;
    if(m_filesEncrypt != NULL)
        delete m_filesEncrypt;
    delete ui;
}

void MainWindow::addWhateverToList(QStringList const& items){
    foreach(auto item, items){
        addWhateverToList(item);
    }
}

void MainWindow::guessEncryptedFinished(QFutureWatcher<EncryptDecrypt>* watcher, QTableWidgetItem* item){
    // Retrieve results
    QList<EncryptDecrypt> res = watcher->future().results();

    size_t length = res.length();
    unsigned crypted = 0;
    unsigned uncrypted = 0;

    foreach(auto i, res){
        if(i == EncryptDecrypt::ENCRYPT){
            ++crypted;
        }else if(i == EncryptDecrypt::DECRYPT){
            ++uncrypted;
        }
    }

    if(crypted == length){
        item->setText("Oui");
    }else if(uncrypted == length){
        item->setText("Non");
    }else{
        item->setText("-");
    }
}

void MainWindow::encryptFinished(QTableWidgetItem* item, EncryptDecrypt action){
    switch(action){
        case EncryptDecrypt::ENCRYPT:
            item->setText("Oui");
            break;
        case EncryptDecrypt::DECRYPT:
            item->setText("Non");
            break;
    }
}

EncryptDecrypt MainWindow::guessEncrypted(QFileInfo const& file){
    QFile f(file.absoluteFilePath());
    f.open(QFile::ReadOnly);
    char* begin = reinterpret_cast<char*>(malloc(512));
    // Only read a few bytes
    f.read(begin, 512);
    EncryptDecrypt res = FilesEncrypt::guessEncrypted(QByteArray(begin, 512));
    if(res == EncryptDecrypt::ENCRYPT){
        Logger::info("File " + file.absoluteFilePath() + " is encrypted");
    }
    free(begin);
    return res;
}

void MainWindow::encrypt(QString const &file, EncryptDecrypt action){
    QFile f(file);
    if(f.open(QFile::ReadWrite)){
        m_filesEncrypt->encryptFile(&f, action);
    }else{
        Logger::error("Can't open file " + file);
    }
    f.close();
}

void MainWindow::addWhateverToList(QString const& item){
    if(!m_dirs.contains(item) && !item.isEmpty()){

        // Add a row
        int rCount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rCount);

        CryptInfos infos;

        QFileInfo info(item);

        // Make the files list
        QStringList* files;

        // Create the watcher
        QFutureWatcher<EncryptDecrypt>* watcher = new QFutureWatcher<EncryptDecrypt>;

        QTableWidgetItem* encryptedVal = new QTableWidgetItem("...");
        infos.item = encryptedVal;

        if(info.isDir()){

              // Add the files to the list
              files = new QStringList(FilesEncrypt::getFilesFromDirRecursive(QDir(item)));

              // Show the type
              ui->tableWidget->setItem(rCount, 0, new QTableWidgetItem("Dossier"));

        }else{

            // Add the single file to the list
            files = new QStringList(info.absoluteFilePath());

            // Show the type
            ui->tableWidget->setItem(rCount, 0, new QTableWidgetItem("Fichier"));

        }

        infos.files = *files;

        connect(watcher, &QFutureWatcher<EncryptDecrypt>::finished, [this, watcher, files, encryptedVal](){
            guessEncryptedFinished(watcher, encryptedVal);
            watcher->deleteLater();
        });

        QFuture<EncryptDecrypt> future = QtConcurrent::mapped(*files, &MainWindow::guessEncrypted);
        watcher->setFuture(future);
        ui->tableWidget->setItem(rCount, 1, encryptedVal);
        ui->tableWidget->setItem(rCount, 2, new QTableWidgetItem(item));
        m_dirs.insert(item, infos);
    }
}

void MainWindow::select_file()
{
    QStringList list = QFileDialog::getOpenFileNames(
        this,
        "Sélectionner des fichiers",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
    );

    addWhateverToList(list);
}

void MainWindow::select_dir()
{
    QString name = QFileDialog::getExistingDirectory(
        this,
        "Sélectionner des dossiers",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
    );

    addWhateverToList(name);
}

void MainWindow::on_importButton_clicked()
{
    QMenu menu(ui->importButton);
    QAction* dir = new QAction("Importer des dossiers");
    QAction* file = new QAction("Importer des fichiers");
    connect(dir, SIGNAL(triggered(bool)), this, SLOT(select_dir()));
    connect(file, SIGNAL(triggered(bool)), this, SLOT(select_file()));
    menu.addAction(dir);
    menu.addAction(file);
    menu.exec(QCursor::pos());
}

void MainWindow::on_decryptAll_clicked()
{
    action(EncryptDecrypt::DECRYPT);
}

void MainWindow::on_cryptAll_clicked()
{
    action(EncryptDecrypt::ENCRYPT);
}

void MainWindow::on_invertAll_clicked()
{

}

void MainWindow::action(EncryptDecrypt action){

    qint64 max(0);
    qint64 items_number(0);
    qint64 item_already_opposite_as_we_want(0);

    bool passOk = false;
    if(!m_filesEncrypt->isAesUncrypted()){
        while(!passOk){
            bool ok;
            QString const pass(ChooseKey::askPassword(false, &ok, this));
            if(!ok) return;
            passOk = m_filesEncrypt->requestAesDecrypt(pass.toStdString());
        }
    }

    foreach(auto const& item, m_dirs.values()){
        foreach (auto s, item.files) {
            ++items_number;
            QFile f(s);
            f.open(QFile::ReadOnly);
            // If current state != from what you'd do
            if(m_filesEncrypt->guessEncrypted(f) != action){
                if(action == EncryptDecrypt::ENCRYPT){
                    max += f.size();
                }else{
                    max += f.size() - (18 + 18 + 16);
                }
            }else{
                ++item_already_opposite_as_we_want;
            }
            QCoreApplication::processEvents();
        }
    }

    // If at least one op is necessary
    if(item_already_opposite_as_we_want != items_number){
        // Show the progress bar and set the max
        m_progress->setMax(max);
        m_progress->show();
    }


    foreach(auto const& item, m_dirs.values()){

        item.item->setText("En cours...");

        QFutureWatcher<void>* watcher = new QFutureWatcher<void>;

        connect(watcher, &QFutureWatcher<void>::finished, [this, action, watcher, item](){
            encryptFinished(item.item, action);
            watcher->deleteLater();
        });

        std::function<void(QString const &)> func = [this, action](QString const &file){
            encrypt(file, action);
        };

        QFuture<void> future = QtConcurrent::map(item.files, func);
        watcher->setFuture(future);

    }

}

void MainWindow::on_remove_clicked()
{
    int row = ui->tableWidget->currentRow();
    if(row >= 0){
        m_dirs.remove(ui->tableWidget->item(row, 2)->text());
        ui->tableWidget->removeRow(row);
    }
}
