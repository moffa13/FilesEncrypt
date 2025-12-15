/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_retrieveKey;
    QAction *action_openSettings;
    QAction *action_checkForUpdates;
    QAction *action_saveKey;
    QAction *action_newKey;
    QAction *action_saveSessionKey;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_4;
    QTableView *filesList;
    QGridLayout *gridLayout;
    QPushButton *cryptAll;
    QPushButton *decryptAll;
    QPushButton *remove;
    QSpacerItem *horizontalSpacer;
    QPushButton *importButton;
    QVBoxLayout *verticalLayout_3;
    QMenuBar *menubar;
    QMenu *menuOptions;
    QMenu *menuFichier;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(734, 300);
        MainWindow->setMinimumSize(QSize(500, 300));
        action_retrieveKey = new QAction(MainWindow);
        action_retrieveKey->setObjectName("action_retrieveKey");
        action_openSettings = new QAction(MainWindow);
        action_openSettings->setObjectName("action_openSettings");
        action_checkForUpdates = new QAction(MainWindow);
        action_checkForUpdates->setObjectName("action_checkForUpdates");
        action_saveKey = new QAction(MainWindow);
        action_saveKey->setObjectName("action_saveKey");
        action_newKey = new QAction(MainWindow);
        action_newKey->setObjectName("action_newKey");
        action_saveSessionKey = new QAction(MainWindow);
        action_saveSessionKey->setObjectName("action_saveSessionKey");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_4 = new QVBoxLayout(centralwidget);
        verticalLayout_4->setObjectName("verticalLayout_4");
        filesList = new QTableView(centralwidget);
        filesList->setObjectName("filesList");

        verticalLayout_4->addWidget(filesList);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        cryptAll = new QPushButton(centralwidget);
        cryptAll->setObjectName("cryptAll");

        gridLayout->addWidget(cryptAll, 3, 1, 1, 1);

        decryptAll = new QPushButton(centralwidget);
        decryptAll->setObjectName("decryptAll");

        gridLayout->addWidget(decryptAll, 3, 2, 1, 1);

        remove = new QPushButton(centralwidget);
        remove->setObjectName("remove");

        gridLayout->addWidget(remove, 3, 3, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 3, 0, 1, 1);

        importButton = new QPushButton(centralwidget);
        importButton->setObjectName("importButton");

        gridLayout->addWidget(importButton, 3, 4, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");

        gridLayout->addLayout(verticalLayout_3, 0, 0, 1, 1);


        verticalLayout_4->addLayout(gridLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 734, 21));
        menuOptions = new QMenu(menubar);
        menuOptions->setObjectName("menuOptions");
        menuFichier = new QMenu(menubar);
        menuFichier->setObjectName("menuFichier");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFichier->menuAction());
        menubar->addAction(menuOptions->menuAction());
        menuOptions->addAction(action_retrieveKey);
        menuOptions->addAction(action_checkForUpdates);
        menuOptions->addAction(action_openSettings);
        menuFichier->addAction(action_newKey);
        menuFichier->addAction(action_saveKey);
        menuFichier->addAction(action_saveSessionKey);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Encrypter", nullptr));
        action_retrieveKey->setText(QCoreApplication::translate("MainWindow", "Obtenir la cl\303\251", nullptr));
        action_openSettings->setText(QCoreApplication::translate("MainWindow", "Param\303\250tres", nullptr));
        action_checkForUpdates->setText(QCoreApplication::translate("MainWindow", "V\303\251rifier les mises \303\240 jour", nullptr));
        action_saveKey->setText(QCoreApplication::translate("MainWindow", "Sauvegarder la cl\303\251", nullptr));
        action_newKey->setText(QCoreApplication::translate("MainWindow", "Nouvelle cl\303\251", nullptr));
        action_saveSessionKey->setText(QCoreApplication::translate("MainWindow", "Cr\303\251er la cl\303\251 de session", nullptr));
        cryptAll->setText(QCoreApplication::translate("MainWindow", "Chiffrer", nullptr));
        decryptAll->setText(QCoreApplication::translate("MainWindow", "D\303\251chiffrer", nullptr));
        remove->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        importButton->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        menuOptions->setTitle(QCoreApplication::translate("MainWindow", "Options", nullptr));
        menuFichier->setTitle(QCoreApplication::translate("MainWindow", "Fichier", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
