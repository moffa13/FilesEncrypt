/********************************************************************************
** Form generated from reading UI file 'Progress.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROGRESS_H
#define UI_PROGRESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
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
    QLabel *timePassed;
    QSpacerItem *horizontalSpacer;
    QPushButton *pause_button;
    QPushButton *cancel_button;

    void setupUi(QWidget *Progress)
    {
        if (Progress->objectName().isEmpty())
            Progress->setObjectName("Progress");
        Progress->resize(400, 106);
        verticalLayout = new QVBoxLayout(Progress);
        verticalLayout->setObjectName("verticalLayout");
        progress = new QProgressBar(Progress);
        progress->setObjectName("progress");
        progress->setMaximum(1000);
        progress->setValue(24);

        verticalLayout->addWidget(progress);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(5);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalLayout_4->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout_4->setContentsMargins(-1, -1, -1, 0);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        speed = new QLabel(Progress);
        speed->setObjectName("speed");

        verticalLayout_2->addWidget(speed);

        file_out_of = new QLabel(Progress);
        file_out_of->setObjectName("file_out_of");

        verticalLayout_2->addWidget(file_out_of);

        threads_n = new QLabel(Progress);
        threads_n->setObjectName("threads_n");

        verticalLayout_2->addWidget(threads_n);

        timePassed = new QLabel(Progress);
        timePassed->setObjectName("timePassed");

        verticalLayout_2->addWidget(timePassed);


        horizontalLayout_4->addLayout(verticalLayout_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        pause_button = new QPushButton(Progress);
        pause_button->setObjectName("pause_button");

        horizontalLayout_4->addWidget(pause_button);

        cancel_button = new QPushButton(Progress);
        cancel_button->setObjectName("cancel_button");

        horizontalLayout_4->addWidget(cancel_button);


        verticalLayout->addLayout(horizontalLayout_4);


        retranslateUi(Progress);

        QMetaObject::connectSlotsByName(Progress);
    } // setupUi

    void retranslateUi(QWidget *Progress)
    {
        Progress->setWindowTitle(QCoreApplication::translate("Progress", "Progression", nullptr));
        speed->setText(QCoreApplication::translate("Progress", "Vitesse : 0 o", nullptr));
        file_out_of->setText(QCoreApplication::translate("Progress", "Fichier 0/0", nullptr));
        threads_n->setText(QCoreApplication::translate("Progress", "Threads : 0", nullptr));
        timePassed->setText(QCoreApplication::translate("Progress", "Temps ecoul\303\251 : 00h 00m 00s", nullptr));
        pause_button->setText(QCoreApplication::translate("Progress", "Pause", nullptr));
        cancel_button->setText(QCoreApplication::translate("Progress", "Annuler", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Progress: public Ui_Progress {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROGRESS_H
