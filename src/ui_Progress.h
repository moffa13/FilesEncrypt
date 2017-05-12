/********************************************************************************
** Form generated from reading UI file 'progress.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROGRESS_H
#define UI_PROGRESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Progress
{
public:
    QVBoxLayout *verticalLayout;
    QProgressBar *progress;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_2;
    QLabel *speed;
    QLabel *file_out_of;
    QLabel *threads_n;
    QSpacerItem *horizontalSpacer;
    QPushButton *pause_button;
    QPushButton *cancel_button;

    void setupUi(QWidget *Progress)
    {
        if (Progress->objectName().isEmpty())
            Progress->setObjectName(QStringLiteral("Progress"));
        Progress->resize(400, 105);
        verticalLayout = new QVBoxLayout(Progress);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        progress = new QProgressBar(Progress);
        progress->setObjectName(QStringLiteral("progress"));
        progress->setMaximum(1000);
        progress->setValue(24);

        verticalLayout->addWidget(progress);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(5);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout_4->setContentsMargins(-1, -1, -1, 0);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        speed = new QLabel(Progress);
        speed->setObjectName(QStringLiteral("speed"));

        verticalLayout_2->addWidget(speed);

        file_out_of = new QLabel(Progress);
        file_out_of->setObjectName(QStringLiteral("file_out_of"));

        verticalLayout_2->addWidget(file_out_of);

        threads_n = new QLabel(Progress);
        threads_n->setObjectName(QStringLiteral("threads_n"));

        verticalLayout_2->addWidget(threads_n);


        horizontalLayout_4->addLayout(verticalLayout_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        pause_button = new QPushButton(Progress);
        pause_button->setObjectName(QStringLiteral("pause_button"));

        horizontalLayout_4->addWidget(pause_button);

        cancel_button = new QPushButton(Progress);
        cancel_button->setObjectName(QStringLiteral("cancel_button"));

        horizontalLayout_4->addWidget(cancel_button);


        verticalLayout->addLayout(horizontalLayout_4);


        retranslateUi(Progress);

        QMetaObject::connectSlotsByName(Progress);
    } // setupUi

    void retranslateUi(QWidget *Progress)
    {
        Progress->setWindowTitle(QApplication::translate("Progress", "Progression", Q_NULLPTR));
        speed->setText(QApplication::translate("Progress", "Vitesse : 0 o", Q_NULLPTR));
        file_out_of->setText(QApplication::translate("Progress", "Fichier 0/0", Q_NULLPTR));
        threads_n->setText(QApplication::translate("Progress", "Threads : 0", Q_NULLPTR));
        pause_button->setText(QApplication::translate("Progress", "Pause", Q_NULLPTR));
        cancel_button->setText(QApplication::translate("Progress", "Annuler", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Progress: public Ui_Progress {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROGRESS_H
