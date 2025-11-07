/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "seqlistBuild",
        "",
        "seqlistInsert",
        "seqlistErase",
        "seqlistClear",
        "linklistBuild",
        "linklistInsert",
        "linklistErase",
        "linklistClear",
        "stackBuild",
        "stackPush",
        "stackPop",
        "stackClear",
        "btBuild",
        "btClear",
        "btPreorder",
        "btInorder",
        "btPostorder",
        "animateBTOrder",
        "const int*",
        "order",
        "n",
        "title",
        "bstBuild",
        "bstFind",
        "bstErase",
        "bstClear",
        "huffmanBuild",
        "huffmanClear",
        "avlBuild",
        "avlInsert",
        "avlClear",
        "onZoomIn",
        "onZoomOut",
        "onZoomFit",
        "onZoomReset",
        "saveDoc",
        "openDoc",
        "exportPNG",
        "insertDSLExample",
        "runDSL",
        "runNLI",
        "dumpBTLevel",
        "QList<int>",
        "ds::BTNode*",
        "root",
        "nullSentinel",
        "dumpPreorder",
        "r",
        "QList<int>&",
        "out",
        "collectLeafWeights"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'seqlistBuild'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'seqlistInsert'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'seqlistErase'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'seqlistClear'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'linklistBuild'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'linklistInsert'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'linklistErase'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'linklistClear'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stackBuild'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stackPush'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stackPop'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stackClear'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'btBuild'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'btClear'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'btPreorder'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'btInorder'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'btPostorder'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'animateBTOrder'
        QtMocHelpers::SlotData<void(const int *, int, const QString &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 }, { QMetaType::Int, 22 }, { QMetaType::QString, 23 },
        }}),
        // Slot 'bstBuild'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bstFind'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bstErase'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bstClear'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'huffmanBuild'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'huffmanClear'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'avlBuild'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'avlInsert'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'avlClear'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomIn'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomOut'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomFit'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomReset'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveDoc'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openDoc'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'exportPNG'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'insertDSLExample'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'runDSL'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'runNLI'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'dumpBTLevel'
        QtMocHelpers::SlotData<QVector<int>(ds::BTNode *, int) const>(43, 2, QMC::AccessPrivate, 0x80000000 | 44, {{
            { 0x80000000 | 45, 46 }, { QMetaType::Int, 47 },
        }}),
        // Slot 'dumpPreorder'
        QtMocHelpers::SlotData<void(ds::BTNode *, QVector<int> &) const>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 45, 49 }, { 0x80000000 | 50, 51 },
        }}),
        // Slot 'collectLeafWeights'
        QtMocHelpers::SlotData<void(ds::BTNode *, QVector<int> &) const>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 45, 49 }, { 0x80000000 | 50, 51 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->seqlistBuild(); break;
        case 1: _t->seqlistInsert(); break;
        case 2: _t->seqlistErase(); break;
        case 3: _t->seqlistClear(); break;
        case 4: _t->linklistBuild(); break;
        case 5: _t->linklistInsert(); break;
        case 6: _t->linklistErase(); break;
        case 7: _t->linklistClear(); break;
        case 8: _t->stackBuild(); break;
        case 9: _t->stackPush(); break;
        case 10: _t->stackPop(); break;
        case 11: _t->stackClear(); break;
        case 12: _t->btBuild(); break;
        case 13: _t->btClear(); break;
        case 14: _t->btPreorder(); break;
        case 15: _t->btInorder(); break;
        case 16: _t->btPostorder(); break;
        case 17: _t->animateBTOrder((*reinterpret_cast< std::add_pointer_t<const int*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 18: _t->bstBuild(); break;
        case 19: _t->bstFind(); break;
        case 20: _t->bstErase(); break;
        case 21: _t->bstClear(); break;
        case 22: _t->huffmanBuild(); break;
        case 23: _t->huffmanClear(); break;
        case 24: _t->avlBuild(); break;
        case 25: _t->avlInsert(); break;
        case 26: _t->avlClear(); break;
        case 27: _t->onZoomIn(); break;
        case 28: _t->onZoomOut(); break;
        case 29: _t->onZoomFit(); break;
        case 30: _t->onZoomReset(); break;
        case 31: _t->saveDoc(); break;
        case 32: _t->openDoc(); break;
        case 33: _t->exportPNG(); break;
        case 34: _t->insertDSLExample(); break;
        case 35: _t->runDSL(); break;
        case 36: _t->runNLI(); break;
        case 37: { QList<int> _r = _t->dumpBTLevel((*reinterpret_cast< std::add_pointer_t<ds::BTNode*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast< QList<int>*>(_a[0]) = std::move(_r); }  break;
        case 38: _t->dumpPreorder((*reinterpret_cast< std::add_pointer_t<ds::BTNode*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>&>>(_a[2]))); break;
        case 39: _t->collectLeafWeights((*reinterpret_cast< std::add_pointer_t<ds::BTNode*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>&>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 40)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 40;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 40)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 40;
    }
    return _id;
}
QT_WARNING_POP
