/****************************************************************************
** Meta object code from reading C++ file 'Crypt.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Crypt.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Crypt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Crypt_t {
    QByteArrayData data[5];
    char stringdata0[56];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Crypt_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Crypt_t qt_meta_stringdata_Crypt = {
    {
QT_MOC_LITERAL(0, 0, 5), // "Crypt"
QT_MOC_LITERAL(1, 6, 19), // "aes_decrypt_updated"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 8), // "progress"
QT_MOC_LITERAL(4, 36, 19) // "aes_encrypt_updated"

    },
    "Crypt\0aes_decrypt_updated\0\0progress\0"
    "aes_encrypt_updated"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Crypt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void Crypt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Crypt *_t = static_cast<Crypt *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->aes_decrypt_updated((*reinterpret_cast< qint32(*)>(_a[1]))); break;
        case 1: _t->aes_encrypt_updated((*reinterpret_cast< qint32(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Crypt::*_t)(qint32 );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Crypt::aes_decrypt_updated)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Crypt::*_t)(qint32 );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Crypt::aes_encrypt_updated)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject Crypt::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Crypt.data,
      qt_meta_data_Crypt,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Crypt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Crypt::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Crypt.stringdata0))
        return static_cast<void*>(const_cast< Crypt*>(this));
    return QObject::qt_metacast(_clname);
}

int Crypt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Crypt::aes_decrypt_updated(qint32 _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Crypt::aes_encrypt_updated(qint32 _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
