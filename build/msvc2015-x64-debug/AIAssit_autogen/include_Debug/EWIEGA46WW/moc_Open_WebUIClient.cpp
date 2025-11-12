/****************************************************************************
** Meta object code from reading C++ file 'Open_WebUIClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../Open_WebUIClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Open_WebUIClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Open_WebUIClient_t {
    QByteArrayData data[11];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Open_WebUIClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Open_WebUIClient_t qt_meta_stringdata_Open_WebUIClient = {
    {
QT_MOC_LITERAL(0, 0, 16), // "Open_WebUIClient"
QT_MOC_LITERAL(1, 17, 21), // "onFetchModelsFinished"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 20), // "onFetchModelsTimeout"
QT_MOC_LITERAL(4, 61, 25), // "onCheckConnectionFinished"
QT_MOC_LITERAL(5, 87, 24), // "onCheckConnectionTimeout"
QT_MOC_LITERAL(6, 112, 26), // "onGetKnowledgeBaseFinished"
QT_MOC_LITERAL(7, 139, 20), // "onFileUploadFinished"
QT_MOC_LITERAL(8, 160, 20), // "onDeleteFileFinished"
QT_MOC_LITERAL(9, 181, 9), // "getAnswer"
QT_MOC_LITERAL(10, 191, 15) // "getStreamAnswer"

    },
    "Open_WebUIClient\0onFetchModelsFinished\0"
    "\0onFetchModelsTimeout\0onCheckConnectionFinished\0"
    "onCheckConnectionTimeout\0"
    "onGetKnowledgeBaseFinished\0"
    "onFileUploadFinished\0onDeleteFileFinished\0"
    "getAnswer\0getStreamAnswer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Open_WebUIClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x0a /* Public */,
       3,    0,   60,    2, 0x0a /* Public */,
       4,    0,   61,    2, 0x0a /* Public */,
       5,    0,   62,    2, 0x0a /* Public */,
       6,    0,   63,    2, 0x0a /* Public */,
       7,    0,   64,    2, 0x0a /* Public */,
       8,    0,   65,    2, 0x0a /* Public */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    0,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
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

void Open_WebUIClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Open_WebUIClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onFetchModelsFinished(); break;
        case 1: _t->onFetchModelsTimeout(); break;
        case 2: _t->onCheckConnectionFinished(); break;
        case 3: _t->onCheckConnectionTimeout(); break;
        case 4: _t->onGetKnowledgeBaseFinished(); break;
        case 5: _t->onFileUploadFinished(); break;
        case 6: _t->onDeleteFileFinished(); break;
        case 7: _t->getAnswer(); break;
        case 8: _t->getStreamAnswer(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject Open_WebUIClient::staticMetaObject = { {
    QMetaObject::SuperData::link<MessageManager::staticMetaObject>(),
    qt_meta_stringdata_Open_WebUIClient.data,
    qt_meta_data_Open_WebUIClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Open_WebUIClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Open_WebUIClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Open_WebUIClient.stringdata0))
        return static_cast<void*>(this);
    return MessageManager::qt_metacast(_clname);
}

int Open_WebUIClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MessageManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
