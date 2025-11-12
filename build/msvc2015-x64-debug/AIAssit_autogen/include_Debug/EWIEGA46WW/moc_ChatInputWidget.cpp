/****************************************************************************
** Meta object code from reading C++ file 'ChatInputWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../ChatInputWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChatInputWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ChatInputWidget_t {
    QByteArrayData data[20];
    char stringdata0[277];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ChatInputWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ChatInputWidget_t qt_meta_stringdata_ChatInputWidget = {
    {
QT_MOC_LITERAL(0, 0, 15), // "ChatInputWidget"
QT_MOC_LITERAL(1, 16, 9), // "MessageUp"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 15), // "ChatSendMessage"
QT_MOC_LITERAL(4, 43, 7), // "message"
QT_MOC_LITERAL(5, 51, 11), // "ModelSelect"
QT_MOC_LITERAL(6, 63, 6), // "iModel"
QT_MOC_LITERAL(7, 70, 19), // "KnowledgeBaseSelect"
QT_MOC_LITERAL(8, 90, 13), // "knowledgeName"
QT_MOC_LITERAL(9, 104, 15), // "addButtonSignal"
QT_MOC_LITERAL(10, 120, 16), // "InitFileFinished"
QT_MOC_LITERAL(11, 137, 4), // "file"
QT_MOC_LITERAL(12, 142, 16), // "RemoveFileSignal"
QT_MOC_LITERAL(13, 159, 20), // "RemoveAllFilesSignal"
QT_MOC_LITERAL(14, 180, 8), // "fileList"
QT_MOC_LITERAL(15, 189, 15), // "onOptionChanged"
QT_MOC_LITERAL(16, 205, 5), // "index"
QT_MOC_LITERAL(17, 211, 21), // "onAttachButtonClicked"
QT_MOC_LITERAL(18, 233, 16), // "onAddFileClicked"
QT_MOC_LITERAL(19, 250, 26) // "onKnowledgeBaseItemClicked"

    },
    "ChatInputWidget\0MessageUp\0\0ChatSendMessage\0"
    "message\0ModelSelect\0iModel\0"
    "KnowledgeBaseSelect\0knowledgeName\0"
    "addButtonSignal\0InitFileFinished\0file\0"
    "RemoveFileSignal\0RemoveAllFilesSignal\0"
    "fileList\0onOptionChanged\0index\0"
    "onAttachButtonClicked\0onAddFileClicked\0"
    "onKnowledgeBaseItemClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ChatInputWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       5,    1,   72,    2, 0x06 /* Public */,
       7,    1,   75,    2, 0x06 /* Public */,
       9,    0,   78,    2, 0x06 /* Public */,
      10,    1,   79,    2, 0x06 /* Public */,
      12,    1,   82,    2, 0x06 /* Public */,
      13,    1,   85,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    1,   88,    2, 0x08 /* Private */,
      17,    0,   91,    2, 0x08 /* Private */,
      18,    0,   92,    2, 0x08 /* Private */,
      19,    0,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QStringList,   14,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ChatInputWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ChatInputWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->MessageUp((*reinterpret_cast< ChatSendMessage(*)>(_a[1]))); break;
        case 1: _t->ModelSelect((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->KnowledgeBaseSelect((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->addButtonSignal(); break;
        case 4: _t->InitFileFinished((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->RemoveFileSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->RemoveAllFilesSignal((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 7: _t->onOptionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onAttachButtonClicked(); break;
        case 9: _t->onAddFileClicked(); break;
        case 10: _t->onKnowledgeBaseItemClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ChatInputWidget::*)(ChatSendMessage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::MessageUp)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::ModelSelect)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::KnowledgeBaseSelect)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::addButtonSignal)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::InitFileFinished)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::RemoveFileSignal)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ChatInputWidget::*)(const QStringList & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatInputWidget::RemoveAllFilesSignal)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ChatInputWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ChatInputWidget.data,
    qt_meta_data_ChatInputWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ChatInputWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChatInputWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ChatInputWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ChatInputWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ChatInputWidget::MessageUp(ChatSendMessage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ChatInputWidget::ModelSelect(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ChatInputWidget::KnowledgeBaseSelect(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ChatInputWidget::addButtonSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ChatInputWidget::InitFileFinished(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ChatInputWidget::RemoveFileSignal(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ChatInputWidget::RemoveAllFilesSignal(const QStringList & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
