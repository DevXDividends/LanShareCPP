/****************************************************************************
** Meta object code from reading C++ file 'ChatWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ui/ChatWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChatWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10ChatWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto ChatWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10ChatWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ChatWindow",
        "onSendClicked",
        "",
        "onAttachClicked",
        "onContactClicked",
        "QListWidgetItem*",
        "item",
        "onCreateGroupClicked",
        "onJoinGroupClicked",
        "onRefreshUsersClicked",
        "onMessageReceived",
        "std::string",
        "from",
        "std::vector<uint8_t>",
        "encrypted",
        "uint64_t",
        "timestamp",
        "onGroupMessageReceived",
        "group",
        "onUserListReceived",
        "std::vector<std::string>",
        "users",
        "onDecryptClicked",
        "MessageBubble*",
        "bubble"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onSendClicked'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAttachClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onContactClicked'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onCreateGroupClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onJoinGroupClicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshUsersClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMessageReceived'
        QtMocHelpers::SlotData<void(const std::string &, const std::vector<uint8_t> &, uint64_t)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 11, 12 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 15, 16 },
        }}),
        // Slot 'onGroupMessageReceived'
        QtMocHelpers::SlotData<void(const std::string &, const std::string &, const std::vector<uint8_t> &, uint64_t)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 11, 18 }, { 0x80000000 | 11, 12 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 15, 16 },
        }}),
        // Slot 'onUserListReceived'
        QtMocHelpers::SlotData<void(const std::vector<std::string> &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
        // Slot 'onDecryptClicked'
        QtMocHelpers::SlotData<void(MessageBubble *)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChatWindow, qt_meta_tag_ZN10ChatWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ChatWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ChatWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ChatWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10ChatWindowE_t>.metaTypes,
    nullptr
} };

void ChatWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ChatWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onSendClicked(); break;
        case 1: _t->onAttachClicked(); break;
        case 2: _t->onContactClicked((*reinterpret_cast<std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 3: _t->onCreateGroupClicked(); break;
        case 4: _t->onJoinGroupClicked(); break;
        case 5: _t->onRefreshUsersClicked(); break;
        case 6: _t->onMessageReceived((*reinterpret_cast<std::add_pointer_t<std::string>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<std::vector<uint8_t>>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<uint64_t>>(_a[3]))); break;
        case 7: _t->onGroupMessageReceived((*reinterpret_cast<std::add_pointer_t<std::string>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<std::string>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<std::vector<uint8_t>>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<uint64_t>>(_a[4]))); break;
        case 8: _t->onUserListReceived((*reinterpret_cast<std::add_pointer_t<std::vector<std::string>>>(_a[1]))); break;
        case 9: _t->onDecryptClicked((*reinterpret_cast<std::add_pointer_t<MessageBubble*>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *ChatWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChatWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ChatWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int ChatWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
