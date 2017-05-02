#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

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
    void on_checkBox_toggled(bool checked);
Q_SIGNALS:
    void closed();
private:
    Ui::SettingsWindow *ui;
    bool m_refuseClose;
protected:
    void closeEvent(QCloseEvent*);
};

#endif // SETTINGSWINDOW_H
