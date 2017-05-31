#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <FilesEncrypt.h>
#include <QTableWidgetItem>
#include <QFuture>
#include <QFutureWatcher>
#include <QSettings>
#include "Progress.h"
#include "ChooseKey.h"
#include "SettingsWindow.h"

#define QPAIR_CRYPT_DEF QPair<QString, EncryptDecrypt>

namespace Ui {
class MainWindow;
}

typedef struct CryptInfos CryptInfos;
struct CryptInfos{
    QMap<QString, EncryptDecrypt*> files;
    bool isFile;
    QTableWidgetItem* encryptedItem;
    QTableWidgetItem* sizeItem;
    QTableWidgetItem* nameItem;
    EncryptDecrypt* state;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
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
    void on_remove_clicked();
    void keySelected();
    void displayKey();
    void openSettings();
    void closeSettings();
    void openSelectedRowInDir();
private:
    Ui::MainWindow *ui;
    Progress* m_progress = NULL;
    ChooseKey* m_choose_key;
    SettingsWindow* m_settings_window;
    QMap<QString, CryptInfos> m_dirs;
    FilesEncrypt* m_filesEncrypt = NULL;
    QFuture<void> m_future_guessEncrypted;
    QFutureWatcher<void> m_future_guessEncrypted_watcher;
    QSettings* m_settings;
    QMenu* m_addWhateverMenu;
    QMenu* m_listRowMenu;
    static QPAIR_CRYPT_DEF guessEncrypted(QString const& file);
    finfo_s encrypt(QString const &file, EncryptDecrypt action, EncryptDecrypt* current_action);
    void guessEncryptedFinished(QFutureWatcher<QPAIR_CRYPT_DEF>* watcher, CryptInfos const &item);
    void encryptFinished(CryptInfos const &item, EncryptDecrypt action);
    void action(EncryptDecrypt action);
    QString get_base_dir() const;
    void set_base_dir(QString const &dir);
    void showInGraphicalShell(const QString &pathIn);
    QString getCurrentDir() const;
    void openInExplorer(const QString& pathIn);
protected:
    void closeEvent(QCloseEvent* event);

};

#endif // MAINWINDOW_H
