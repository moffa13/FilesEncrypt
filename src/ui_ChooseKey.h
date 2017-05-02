/********************************************************************************
** Form generated from reading UI file 'ChooseKey.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSEKEY_H
#define UI_CHOOSEKEY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
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
    QPushButton *newKey;
    QPushButton *choose;

    void setupUi(QWidget *ChooseKey)
    {
        if (ChooseKey->objectName().isEmpty())
            ChooseKey->setObjectName(QStringLiteral("ChooseKey"));
        ChooseKey->resize(400, 83);
        verticalLayout = new QVBoxLayout(ChooseKey);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        key = new QLineEdit(ChooseKey);
        key->setObjectName(QStringLiteral("key"));

        horizontalLayout_2->addWidget(key);

        select = new QPushButton(ChooseKey);
        select->setObjectName(QStringLiteral("select"));

        horizontalLayout_2->addWidget(select);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        newKey = new QPushButton(ChooseKey);
        newKey->setObjectName(QStringLiteral("newKey"));

        horizontalLayout->addWidget(newKey);

        choose = new QPushButton(ChooseKey);
        choose->setObjectName(QStringLiteral("choose"));

        horizontalLayout->addWidget(choose);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ChooseKey);

        QMetaObject::connectSlotsByName(ChooseKey);
    } // setupUi

    void retranslateUi(QWidget *ChooseKey)
    {
        ChooseKey->setWindowTitle(QApplication::translate("ChooseKey", "S\303\251lectionner une cl\303\251", Q_NULLPTR));
        select->setText(QApplication::translate("ChooseKey", "Parcourir", Q_NULLPTR));
        newKey->setText(QApplication::translate("ChooseKey", "Cr\303\251er une nouvelle cl\303\251", Q_NULLPTR));
        choose->setText(QApplication::translate("ChooseKey", "OK", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ChooseKey: public Ui_ChooseKey {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSEKEY_H
