/********************************************************************************
** Form generated from reading UI file 'TrollDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TROLLDIALOG_H
#define UI_TROLLDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_TrollDialog
{
public:
    QLabel *label;

    void setupUi(QDialog *TrollDialog)
    {
        if (TrollDialog->objectName().isEmpty())
            TrollDialog->setObjectName(QStringLiteral("TrollDialog"));
        TrollDialog->resize(364, 132);
        label = new QLabel(TrollDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(170, 60, 47, 13));

        retranslateUi(TrollDialog);

        QMetaObject::connectSlotsByName(TrollDialog);
    } // setupUi

    void retranslateUi(QDialog *TrollDialog)
    {
        TrollDialog->setWindowTitle(QApplication::translate("TrollDialog", "Dialog", 0));
        label->setText(QApplication::translate("TrollDialog", ":(", 0));
    } // retranslateUi

};

namespace Ui {
    class TrollDialog: public Ui_TrollDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TROLLDIALOG_H
