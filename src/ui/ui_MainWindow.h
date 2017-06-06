/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_retrieveKey;
    QAction *action_openSettings;
    QAction *action_checkForUpdates;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_4;
    QTableWidget *tableWidget;
    QGridLayout *gridLayout;
    QPushButton *cryptAll;
    QPushButton *decryptAll;
    QPushButton *remove;
    QSpacerItem *horizontalSpacer;
    QPushButton *importButton;
    QVBoxLayout *verticalLayout_3;
    QMenuBar *menubar;
    QMenu *menuOptions;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(734, 300);
        MainWindow->setMinimumSize(QSize(500, 300));
        action_retrieveKey = new QAction(MainWindow);
        action_retrieveKey->setObjectName(QStringLiteral("action_retrieveKey"));
        action_openSettings = new QAction(MainWindow);
        action_openSettings->setObjectName(QStringLiteral("action_openSettings"));
        action_checkForUpdates = new QAction(MainWindow);
        action_checkForUpdates->setObjectName(QStringLiteral("action_checkForUpdates"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout_4 = new QVBoxLayout(centralwidget);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        tableWidget = new QTableWidget(centralwidget);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));

        verticalLayout_4->addWidget(tableWidget);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        cryptAll = new QPushButton(centralwidget);
        cryptAll->setObjectName(QStringLiteral("cryptAll"));

        gridLayout->addWidget(cryptAll, 3, 1, 1, 1);

        decryptAll = new QPushButton(centralwidget);
        decryptAll->setObjectName(QStringLiteral("decryptAll"));

        gridLayout->addWidget(decryptAll, 3, 2, 1, 1);

        remove = new QPushButton(centralwidget);
        remove->setObjectName(QStringLiteral("remove"));

        gridLayout->addWidget(remove, 3, 3, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 3, 0, 1, 1);

        importButton = new QPushButton(centralwidget);
        importButton->setObjectName(QStringLiteral("importButton"));

        gridLayout->addWidget(importButton, 3, 4, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));

        gridLayout->addLayout(verticalLayout_3, 0, 0, 1, 1);


        verticalLayout_4->addLayout(gridLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 734, 22));
        menuOptions = new QMenu(menubar);
        menuOptions->setObjectName(QStringLiteral("menuOptions"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuOptions->menuAction());
        menuOptions->addAction(action_retrieveKey);
        menuOptions->addAction(action_openSettings);
        menuOptions->addAction(action_checkForUpdates);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Encrypter", Q_NULLPTR));
        action_retrieveKey->setText(QApplication::translate("MainWindow", "Obtenir la cl\303\251", Q_NULLPTR));
        action_openSettings->setText(QApplication::translate("MainWindow", "Param\303\250tres", Q_NULLPTR));
        action_checkForUpdates->setText(QApplication::translate("MainWindow", "V\303\251rifier les mises \303\240 jour", Q_NULLPTR));
        cryptAll->setText(QApplication::translate("MainWindow", "Crypter", Q_NULLPTR));
        decryptAll->setText(QApplication::translate("MainWindow", "D\303\251crypter", Q_NULLPTR));
        remove->setText(QApplication::translate("MainWindow", "Supprimer", Q_NULLPTR));
        importButton->setText(QApplication::translate("MainWindow", "Ajouter", Q_NULLPTR));
        menuOptions->setTitle(QApplication::translate("MainWindow", "Options", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
