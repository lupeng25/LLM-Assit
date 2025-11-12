/****************************************************************************
** Meta object code from reading C++ file 'MessageManager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../MessageManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MessageManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MessageManager_t {
    QByteArrayData data[33];
    char stringdata0[535];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MessageManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MessageManager_t qt_meta_stringdata_MessageManager = {
    {
QT_MOC_LITERAL(0, 0, 14), // "MessageManager"
QT_MOC_LITERAL(1, 15, 6), // "Answer"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 4), // "word"
QT_MOC_LITERAL(4, 28, 6), // "bError"
QT_MOC_LITERAL(5, 35, 12), // "AnswerStream"
QT_MOC_LITERAL(6, 48, 18), // "ChangeButtonStatus"
QT_MOC_LITERAL(7, 67, 8), // "received"
QT_MOC_LITERAL(8, 76, 18), // "FunctionCallSignal"
QT_MOC_LITERAL(9, 95, 12), // "QJsonObject&"
QT_MOC_LITERAL(10, 108, 7), // "Content"
QT_MOC_LITERAL(11, 116, 11), // "StreamEnded"
QT_MOC_LITERAL(12, 128, 29), // "serverConnectionCheckFinished"
QT_MOC_LITERAL(13, 158, 11), // "isConnected"
QT_MOC_LITERAL(14, 170, 12), // "errorMessage"
QT_MOC_LITERAL(15, 183, 17), // "modelsListFetched"
QT_MOC_LITERAL(16, 201, 7), // "success"
QT_MOC_LITERAL(17, 209, 6), // "models"
QT_MOC_LITERAL(18, 216, 19), // "KonwledgeBaseSignal"
QT_MOC_LITERAL(19, 236, 45), // "std::map<QString,std::pair<QS..."
QT_MOC_LITERAL(20, 282, 8), // "knowBase"
QT_MOC_LITERAL(21, 291, 19), // "FollowSuggestSignal"
QT_MOC_LITERAL(22, 311, 11), // "suggestions"
QT_MOC_LITERAL(23, 323, 21), // "onFetchModelsFinished"
QT_MOC_LITERAL(24, 345, 20), // "onFetchModelsTimeout"
QT_MOC_LITERAL(25, 366, 25), // "onCheckConnectionFinished"
QT_MOC_LITERAL(26, 392, 24), // "onCheckConnectionTimeout"
QT_MOC_LITERAL(27, 417, 22), // "GetFollowUpSuggestions"
QT_MOC_LITERAL(28, 440, 26), // "onGetKnowledgeBaseFinished"
QT_MOC_LITERAL(29, 467, 20), // "onFileUploadFinished"
QT_MOC_LITERAL(30, 488, 20), // "onDeleteFileFinished"
QT_MOC_LITERAL(31, 509, 9), // "getAnswer"
QT_MOC_LITERAL(32, 519, 15) // "getStreamAnswer"

    },
    "MessageManager\0Answer\0\0word\0bError\0"
    "AnswerStream\0ChangeButtonStatus\0"
    "received\0FunctionCallSignal\0QJsonObject&\0"
    "Content\0StreamEnded\0serverConnectionCheckFinished\0"
    "isConnected\0errorMessage\0modelsListFetched\0"
    "success\0models\0KonwledgeBaseSignal\0"
    "std::map<QString,std::pair<QString,QString> >\0"
    "knowBase\0FollowSuggestSignal\0suggestions\0"
    "onFetchModelsFinished\0onFetchModelsTimeout\0"
    "onCheckConnectionFinished\0"
    "onCheckConnectionTimeout\0"
    "GetFollowUpSuggestions\0"
    "onGetKnowledgeBaseFinished\0"
    "onFileUploadFinished\0onDeleteFileFinished\0"
    "getAnswer\0getStreamAnswer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MessageManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,  109,    2, 0x06 /* Public */,
       5,    1,  114,    2, 0x06 /* Public */,
       6,    1,  117,    2, 0x06 /* Public */,
       8,    1,  120,    2, 0x06 /* Public */,
      11,    0,  123,    2, 0x06 /* Public */,
      12,    2,  124,    2, 0x06 /* Public */,
      15,    3,  129,    2, 0x06 /* Public */,
      18,    1,  136,    2, 0x06 /* Public */,
      21,    1,  139,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      23,    0,  142,    2, 0x0a /* Public */,
      24,    0,  143,    2, 0x0a /* Public */,
      25,    0,  144,    2, 0x0a /* Public */,
      26,    0,  145,    2, 0x0a /* Public */,
      27,    0,  146,    2, 0x0a /* Public */,
      28,    0,  147,    2, 0x0a /* Public */,
      29,    0,  148,    2, 0x0a /* Public */,
      30,    0,  149,    2, 0x0a /* Public */,
      31,    0,  150,    2, 0x08 /* Private */,
      32,    0,  151,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   13,   14,
    QMetaType::Void, QMetaType::Bool, QMetaType::QStringList, QMetaType::QString,   16,   17,   14,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void, QMetaType::QStringList,   22,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::QStringList,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MessageManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MessageManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Answer((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->AnswerStream((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->ChangeButtonStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->FunctionCallSignal((*reinterpret_cast< QJsonObject(*)>(_a[1]))); break;
        case 4: _t->StreamEnded(); break;
        case 5: _t->serverConnectionCheckFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 6: _t->modelsListFetched((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 7: _t->KonwledgeBaseSignal((*reinterpret_cast< std::map<QString,std::pair<QString,QString> >(*)>(_a[1]))); break;
        case 8: _t->FollowSuggestSignal((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 9: _t->onFetchModelsFinished(); break;
        case 10: _t->onFetchModelsTimeout(); break;
        case 11: _t->onCheckConnectionFinished(); break;
        case 12: _t->onCheckConnectionTimeout(); break;
        case 13: { QStringList _r = _t->GetFollowUpSuggestions();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->onGetKnowledgeBaseFinished(); break;
        case 15: _t->onFileUploadFinished(); break;
        case 16: _t->onDeleteFileFinished(); break;
        case 17: _t->getAnswer(); break;
        case 18: _t->getStreamAnswer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MessageManager::*)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::Answer)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::AnswerStream)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::ChangeButtonStatus)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::FunctionCallSignal)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::StreamEnded)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(bool , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::serverConnectionCheckFinished)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(bool , const QStringList & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::modelsListFetched)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(std::map<QString,std::pair<QString,QString>> );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::KonwledgeBaseSignal)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (MessageManager::*)(QStringList );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MessageManager::FollowSuggestSignal)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MessageManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MessageManager.data,
    qt_meta_data_MessageManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MessageManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MessageManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MessageManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MessageManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void MessageManager::Answer(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MessageManager::AnswerStream(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MessageManager::ChangeButtonStatus(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MessageManager::FunctionCallSignal(QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MessageManager::StreamEnded()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void MessageManager::serverConnectionCheckFinished(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void MessageManager::modelsListFetched(bool _t1, const QStringList & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void MessageManager::KonwledgeBaseSignal(std::map<QString,std::pair<QString,QString>> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void MessageManager::FollowSuggestSignal(QStringList _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
