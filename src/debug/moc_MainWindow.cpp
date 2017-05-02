/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[20];
    char stringdata0[260];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 10), // "select_dir"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 11), // "select_file"
QT_MOC_LITERAL(4, 35, 17), // "addWhateverToList"
QT_MOC_LITERAL(5, 53, 5), // "items"
QT_MOC_LITERAL(6, 59, 4), // "item"
QT_MOC_LITERAL(7, 64, 11), // "resizeEvent"
QT_MOC_LITERAL(8, 76, 13), // "QResizeEvent*"
QT_MOC_LITERAL(9, 90, 5), // "event"
QT_MOC_LITERAL(10, 96, 9), // "showEvent"
QT_MOC_LITERAL(11, 106, 11), // "QShowEvent*"
QT_MOC_LITERAL(12, 118, 13), // "correctResize"
QT_MOC_LITERAL(13, 132, 23), // "on_importButton_clicked"
QT_MOC_LITERAL(14, 156, 21), // "on_decryptAll_clicked"
QT_MOC_LITERAL(15, 178, 19), // "on_cryptAll_clicked"
QT_MOC_LITERAL(16, 198, 20), // "on_invertAll_clicked"
QT_MOC_LITERAL(17, 219, 17), // "on_remove_clicked"
QT_MOC_LITERAL(18, 237, 11), // "keySelected"
QT_MOC_LITERAL(19, 249, 10) // "displayKey"

    },
    "MainWindow\0select_dir\0\0select_file\0"
    "addWhateverToList\0items\0item\0resizeEvent\0"
    "QResizeEvent*\0event\0showEvent\0QShowEvent*\0"
    "correctResize\0on_importButton_clicked\0"
    "on_decryptAll_clicked\0on_cryptAll_clicked\0"
    "on_invertAll_clicked\0on_remove_clicked\0"
    "keySelected\0displayKey"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    0,   85,    2, 0x08 /* Private */,
       4,    1,   86,    2, 0x08 /* Private */,
       4,    1,   89,    2, 0x08 /* Private */,
       7,    1,   92,    2, 0x08 /* Private */,
      10,    1,   95,    2, 0x08 /* Private */,
      12,    0,   98,    2, 0x08 /* Private */,
      13,    0,   99,    2, 0x08 /* Private */,
      14,    0,  100,    2, 0x08 /* Private */,
      15,    0,  101,    2, 0x08 /* Private */,
      16,    0,  102,    2, 0x08 /* Private */,
      17,    0,  103,    2, 0x08 /* Private */,
      18,    0,  104,    2, 0x0a /* Public */,
      19,    0,  105,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList,    5,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->select_dir(); break;
        case 1: _t->select_file(); break;
        case 2: _t->addWhateverToList((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 3: _t->addWhateverToList((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->resizeEvent((*reinterpret_cast< QResizeEvent*(*)>(_a[1]))); break;
        case 5: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 6: _t->correctResize(); break;
        case 7: _t->on_importButton_clicked(); break;
        case 8: _t->on_decryptAll_clicked(); break;
        case 9: _t->on_cryptAll_clicked(); break;
        case 10: _t->on_invertAll_clicked(); break;
        case 11: _t->on_remove_clicked(); break;
        case 12: _t->keySelected(); break;
        case 13: _t->displayKey(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
