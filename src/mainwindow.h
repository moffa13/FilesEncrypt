#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <FilesEncrypt.h>
#include <QTableWidgetItem>
#include <QFuture>
#include <QFutureWatcher>
#include "Progress.h"
#include "ChooseKey.h"

namespace Ui {
class MainWindow;
}

typedef struct CryptInfos CryptInfos;
struct CryptInfos{
    QStringList files;
    QTableWidgetItem* item;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void select_dir();
    void select_file();
    void addWhateverToList(QStringList const& items);
    void addWhateverToList(QString const& item);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void correctResize();
    void on_importButton_clicked();
    void on_decryptAll_clicked();
    void on_cryptAll_clicked();
    void on_invertAll_clicked();

    void on_remove_clicked();

public Q_SLOTS:
    void keySelected();
private Q_SLOTS:
    void displayKey();
private:
    Ui::MainWindow *ui;
    Progress* m_progress = NULL;
    ChooseKey* m_choose_key;
    QMap<QString, CryptInfos> m_dirs;
    FilesEncrypt* m_filesEncrypt = NULL;
    QFuture<void> m_future_guessEncrypted;
    QFutureWatcher<void> m_future_guessEncrypted_watcher;
    static EncryptDecrypt guessEncrypted(QFileInfo const& file);
    void encrypt(QString const &file, EncryptDecrypt action);
    void guessEncryptedFinished(QFutureWatcher<EncryptDecrypt>* watcher, QTableWidgetItem* item);
    void encryptFinished(QTableWidgetItem* item, EncryptDecrypt action);
    void action(EncryptDecrypt action);


protected:
    void closeEvent(QCloseEvent* event);

};

#endif // MAINWINDOW_H
