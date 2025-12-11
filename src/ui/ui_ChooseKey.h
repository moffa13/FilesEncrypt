/********************************************************************************
** Form generated from reading UI file 'ChooseKey.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSEKEY_H
#define UI_CHOOSEKEY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChooseKey
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *key;
    QPushButton *select;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QPushButton *newKey;
    QPushButton *choose;

    void setupUi(QWidget *ChooseKey)
    {
        if (ChooseKey->objectName().isEmpty())
            ChooseKey->setObjectName("ChooseKey");
        ChooseKey->resize(400, 83);
        verticalLayout = new QVBoxLayout(ChooseKey);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        key = new QLineEdit(ChooseKey);
        key->setObjectName("key");

        horizontalLayout_2->addWidget(key);

        select = new QPushButton(ChooseKey);
        select->setObjectName("select");

        horizontalLayout_2->addWidget(select);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(ChooseKey);
        pushButton->setObjectName("pushButton");

        horizontalLayout->addWidget(pushButton);

        newKey = new QPushButton(ChooseKey);
        newKey->setObjectName("newKey");

        horizontalLayout->addWidget(newKey);

        choose = new QPushButton(ChooseKey);
        choose->setObjectName("choose");

        horizontalLayout->addWidget(choose);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ChooseKey);

        QMetaObject::connectSlotsByName(ChooseKey);
    } // setupUi

    void retranslateUi(QWidget *ChooseKey)
    {
        ChooseKey->setWindowTitle(QCoreApplication::translate("ChooseKey", "S\303\251lectionner une cl\303\251", nullptr));
        select->setText(QCoreApplication::translate("ChooseKey", "Parcourir", nullptr));
        pushButton->setText(QCoreApplication::translate("ChooseKey", "Saisir une cl\303\251 AES", nullptr));
        newKey->setText(QCoreApplication::translate("ChooseKey", "Cr\303\251er une nouvelle cl\303\251", nullptr));
        choose->setText(QCoreApplication::translate("ChooseKey", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChooseKey: public Ui_ChooseKey {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSEKEY_H
