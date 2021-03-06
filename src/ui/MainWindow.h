#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <FilesEncrypt.h>
#include <QTableWidgetItem>
#include <QFuture>
#include <QFutureWatcher>
#include <QSettings>
#include <QHBoxLayout>
#include <QLabel>
#include "Progress.h"
#include "ChooseKey.h"
#include "network/UpdateManager.h"
#include "SettingsWindow.h"
#include <functional>
#include "FilesListModel.h"

#define QPAIR_CRYPT_DEF QPair<QString, EncryptDecrypt_s>

namespace Ui {
class MainWindow;
}

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
        void displayKey(bool forceAsk = true);
        void openSettings();
        void closeSettings();
        void openSelectedRowInDir();
        void on_action_newKey_triggered();

        void on_action_saveKey_triggered();

    private:
        Ui::MainWindow *ui;
        Progress* m_progress = NULL;
        ChooseKey* m_choose_key;
        UpdateManager m_update;
        QLabel *m_statusBarLabel;
        SettingsWindow* m_settings_window;
        FilesEncrypt* m_filesEncrypt = NULL;
        QFuture<void> m_future_guessEncrypted;
        QFutureWatcher<void> m_future_guessEncrypted_watcher;
        QSettings* m_settings;
        QMenu* m_addWhateverMenu;
        QMenu* m_listRowMenu;
        bool m_encrypting = false;
        FilesListModel m_filesListModel;
        static QPAIR_CRYPT_DEF guessEncrypted(QString const& file);
        finfo_s encrypt(QString const &file, EncryptDecrypt action) const;
        void guessEncryptedFinished(QFutureWatcher<QPAIR_CRYPT_DEF>* watcher, CryptInfos &item) const;
        void encryptFinished(CryptInfos &item) const;
        void action(EncryptDecrypt action);
        QString get_base_dir() const;
        void set_base_dir(QString const &dir);
        void showInGraphicalShell(const QString &pathIn);
        QString getCurrentDir() const;
        static QMutex s_encryptMutex;
        static QMutex ENCRYTPT_MUTEX2;
        static unsigned s_current_guess_encrypted_watchers;
        void openInExplorer(const QString& pathIn);
        void openInNautilus(const QString& pathIn);
        bool beSureKeyIsSelectedAndValid(std::function<void()> func, bool forceAskKey = false);
        void updateAvailableButtons();
        bool deleteKey();
    public Q_SLOTS:
        void updateStatusBar();
    protected:
        void closeEvent(QCloseEvent* event);
        void dragLeaveEvent(QDragLeaveEvent * event);
        void dragMoveEvent(QDragMoveEvent * event);
        void dropEvent(QDropEvent * event);
        void dragEnterEvent(QDragEnterEvent * event);

};

#endif // MAINWINDOW_H
