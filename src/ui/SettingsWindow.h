#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QPointer>
#include <QSettings>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();
private slots:

Q_SIGNALS:
    void closed();
private:
    Ui::SettingsWindow *ui;
    QPointer<QSettings> m_settings;
    bool m_refuseClose;
protected:
    void closeEvent(QCloseEvent*);
};

#endif // SETTINGSWINDOW_H
