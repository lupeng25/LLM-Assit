/****************************************************************************
** Meta object code from reading C++ file 'LLMClientManager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../LLMClientManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LLMClientManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LLMClientManager_t {
    QByteArrayData data[19];
    char stringdata0[283];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LLMClientManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LLMClientManager_t qt_meta_stringdata_LLMClientManager = {
    {
QT_MOC_LITERAL(0, 0, 16), // "LLMClientManager"
QT_MOC_LITERAL(1, 17, 13), // "clientChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 15), // "MessageManager*"
QT_MOC_LITERAL(4, 48, 6), // "client"
QT_MOC_LITERAL(5, 55, 24), // "connectionCheckSucceeded"
QT_MOC_LITERAL(6, 80, 21), // "connectionCheckFailed"
QT_MOC_LITERAL(7, 102, 12), // "errorMessage"
QT_MOC_LITERAL(8, 115, 20), // "modelsFetchSucceeded"
QT_MOC_LITERAL(9, 136, 6), // "models"
QT_MOC_LITERAL(10, 143, 17), // "modelsFetchFailed"
QT_MOC_LITERAL(11, 161, 11), // "clientError"
QT_MOC_LITERAL(12, 173, 7), // "context"
QT_MOC_LITERAL(13, 181, 29), // "handleConnectionCheckFinished"
QT_MOC_LITERAL(14, 211, 11), // "isConnected"
QT_MOC_LITERAL(15, 223, 23), // "handleModelsListFetched"
QT_MOC_LITERAL(16, 247, 7), // "success"
QT_MOC_LITERAL(17, 255, 15), // "retryConnection"
QT_MOC_LITERAL(18, 271, 11) // "retryModels"

    },
    "LLMClientManager\0clientChanged\0\0"
    "MessageManager*\0client\0connectionCheckSucceeded\0"
    "connectionCheckFailed\0errorMessage\0"
    "modelsFetchSucceeded\0models\0"
    "modelsFetchFailed\0clientError\0context\0"
    "handleConnectionCheckFinished\0isConnected\0"
    "handleModelsListFetched\0success\0"
    "retryConnection\0retryModels"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LLMClientManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x06 /* Public */,
       5,    0,   67,    2, 0x06 /* Public */,
       6,    1,   68,    2, 0x06 /* Public */,
       8,    1,   71,    2, 0x06 /* Public */,
      10,    1,   74,    2, 0x06 /* Public */,
      11,    2,   77,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    2,   82,    2, 0x08 /* Private */,
      15,    3,   87,    2, 0x08 /* Private */,
      17,    0,   94,    2, 0x08 /* Private */,
      18,    0,   95,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QStringList,    9,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   12,    7,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   14,    7,
    QMetaType::Void, QMetaType::Bool, QMetaType::QStringList, QMetaType::QString,   16,    9,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void LLMClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LLMClientManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->clientChanged((*reinterpret_cast< MessageManager*(*)>(_a[1]))); break;
        case 1: _t->connectionCheckSucceeded(); break;
        case 2: _t->connectionCheckFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->modelsFetchSucceeded((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 4: _t->modelsFetchFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->clientError((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 6: _t->handleConnectionCheckFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 7: _t->handleModelsListFetched((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 8: _t->retryConnection(); break;
        case 9: _t->retryModels(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< MessageManager* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LLMClientManager::*)(MessageManager * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::clientChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LLMClientManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::connectionCheckSucceeded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LLMClientManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::connectionCheckFailed)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LLMClientManager::*)(const QStringList & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::modelsFetchSucceeded)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LLMClientManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::modelsFetchFailed)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LLMClientManager::*)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMClientManager::clientError)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LLMClientManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LLMClientManager.data,
    qt_meta_data_LLMClientManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LLMClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LLMClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LLMClientManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LLMClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void LLMClientManager::clientChanged(MessageManager * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LLMClientManager::connectionCheckSucceeded()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LLMClientManager::connectionCheckFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LLMClientManager::modelsFetchSucceeded(const QStringList & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LLMClientManager::modelsFetchFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LLMClientManager::clientError(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
