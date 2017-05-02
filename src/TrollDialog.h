#ifndef TROLLDIALOG_H
#define TROLLDIALOG_H

#include <QDialog>

namespace Ui {
class TrollDialog;
}

class TrollDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrollDialog(QWidget *parent = 0, unsigned i = 2);
    ~TrollDialog();
    void show();
    static QPoint randomQPoint(uint16_t a, uint16_t b);
private:
    Ui::TrollDialog *ui;
    unsigned m_i;
protected:
    void closeEvent(QCloseEvent*);
};

#endif // TROLLDIALOG_H
