#include "mainwindow.h"
#include "dsl.h"
#include "llmclient.h"
#include <QGraphicsScene>
#include <QApplication>
#include <QMessageBox>
#include <QStyle>
#include <QIcon>
#include <QStatusBar>
#include <QTimeLine>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <cmath>
#include <memory>
#include <climits>
#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QSet>
#include <QHash>
#include <QTableWidget>
#include <QHeaderView>
#include <QPointF>

void MainWindow::animateBTOrder(const int* order, int n, const QString& title)
{
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 选择需要绘制的树根：标题包含"BST/二叉搜索树"则优先用 BST，否则用普通二叉树
    ds::BTNode* rootBT  = bt.root();
    ds::BTNode* rootBST = bst.root();
    ds::BTNode* root = nullptr;
    if (title.contains("BST", Qt::CaseInsensitive) || title.contains(QStringLiteral("二叉搜索树"))) {
        root = rootBST ? rootBST : rootBT;
    } else {
        root = rootBT ? rootBT : rootBST;
    }

    // 空序列或无树时的兜底
    if (!order || n <= 0 || !root) {
        view->resetScene();
        view->setTitle(title + QStringLiteral("（空）"));
        if (root) drawBT(root, 400, 120, 200, 0);
        timer.start(); // 保持流程一致
        return;
    }

    // 逐步高亮访问序列
    for (int i = 0; i < n; ++i) {
        const int key = order[i];
        steps.push_back([this, i, n, key, title, root]() {
            view->resetScene();
            view->setTitle(QString("%1：访问 %2（%3/%4）").arg(title).arg(key).arg(i + 1).arg(n));
            drawBT(root, 400, 120, 200, key); // 仅高亮，不移动
        });
    }
    timer.start();
}



// ================== 文件保存/打开/导出 ==================
void MainWindow::saveDoc() {
    QFileDialog dialog(this, QStringLiteral("保存为"), "", "XPR's DS Visualizer (*.xpr)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("xpr");
    dialog.setStyleSheet(
        "QFileDialog { background: #f8fafc; }"
        "QLabel { color: #334155; font-weight: 600; }"
        "QPushButton { background: #3b82f6; color: white; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background: #2563eb; }"
        "QLineEdit { border: 2px solid #e2e8f0; border-radius: 6px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        if(path.isEmpty()) return;

        // 一次性保存所有数据结构的状态，方便下次直接恢复
        QJsonObject root;
        root["kind"] = "all";

        // 顺序表
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < seq.size(); ++i) arr.push_back(seq.get(i));
            obj["values"] = arr;
            root["seqlist"] = obj;
        }
        // 单链表
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < link.size(); ++i) arr.push_back(link.get(i));
            obj["values"] = arr;
            root["linkedlist"] = obj;
        }
        // 栈
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < st.size(); ++i) arr.push_back(st.get(i));
            obj["values"] = arr;
            root["stack"] = obj;
        }
        // 普通二叉树（层序 + 哨兵）
        {
            QJsonObject obj;
            int sent = (btLastNullSentinel_ == INT_MIN ? -1 : btLastNullSentinel_);
            obj["null"] = sent;
            QJsonArray arr;
            auto v = dumpBTLevel(bt.root(), sent);
            for(int x: v) arr.push_back(x);
            obj["level"] = arr;
            root["binarytree"] = obj;
        }
        // BST（先序）
        {
            QJsonObject obj;
            QJsonArray arr;
            QVector<int> pre;
            dumpPreorder(bst.root(), pre);
            for(int x: pre) arr.push_back(x);
            obj["preorder"] = arr;
            root["bst"] = obj;
        }
        // AVL（先序）
        {
            QJsonObject obj;
            QJsonArray arr;
            QVector<int> pre;
            dumpPreorder(avl.root(), pre);
            for(int x: pre) arr.push_back(x);
            obj["preorder"] = arr;
            root["avl"] = obj;
        }
        // Huffman（权值列表）
        {
            QJsonObject obj;
            QJsonArray arr;
            if(!huffLastWeights_.isEmpty()) {
                for(int w: huffLastWeights_) arr.push_back(w);
            } else {
                QVector<int> leaves;
                collectLeafWeights(huff.root(), leaves);
                for(int w: leaves) arr.push_back(w);
            }
            obj["weights"] = arr;
            root["huffman"] = obj;
        }

        QFile f(path);
        if(f.open(QIODevice::WriteOnly)) {
            f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
            f.close();
            statusBar()->showMessage(QString("已保存全部数据结构：%1").arg(path));
        }
    }
}

void MainWindow::openDoc() {
    QFileDialog dialog(this, QStringLiteral("打开"), "", "XPR's DS Visualizer (*.xpr)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setStyleSheet(
        "QFileDialog { background: #f8fafc; }"
        "QLabel { color: #334155; font-weight: 600; }"
        "QPushButton { background: #3b82f6; color: white; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background: #2563eb; }"
        "QLineEdit { border: 2px solid #e2e8f0; border-radius: 6px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        if(path.isEmpty()) return;

        QFile f(path);
        if(!f.open(QIODevice::ReadOnly)) {
            statusBar()->showMessage("打开失败");
            return;
        }

        auto doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if(!doc.isObject()) {
            statusBar()->showMessage("文件格式错误");
            return;
        }

        // 一次性恢复所有数据结构，但不强制切换右侧模块；画布按当前模块刷新
        auto o = doc.object();
        if (o.contains("seqlist")) {
            QJsonObject s = o["seqlist"].toObject();
            seq.clear();
            for (auto v : s["values"].toArray()) seq.insert(seq.size(), v.toInt());
        }
        if (o.contains("linkedlist")) {
            QJsonObject s = o["linkedlist"].toObject();
            link.clear();
            for (auto v : s["values"].toArray()) link.insert(link.size(), v.toInt());
        }
        if (o.contains("stack")) {
            QJsonObject s = o["stack"].toObject();
            st.clear();
            for (auto v : s["values"].toArray()) st.push(v.toInt());
        }
        if (o.contains("binarytree")) {
            QJsonObject s = o["binarytree"].toObject();
            int sent = s["null"].toInt(-1);
            QVector<int> a; for (auto v : s["level"].toArray()) a.push_back(v.toInt());
            bt.clear(); bt.buildTree(a.data(), a.size(), sent);
            btLastNullSentinel_ = sent;
        }
        if (o.contains("bst")) {
            QJsonObject s = o["bst"].toObject();
            bst.clear(); for (auto v : s["preorder"].toArray()) bst.insert(v.toInt());
        }
        if (o.contains("avl")) {
            QJsonObject s = o["avl"].toObject();
            avl.clear(); for (auto v : s["preorder"].toArray()) avl.insert(v.toInt());
        }
        if (o.contains("huffman")) {
            QJsonObject s = o["huffman"].toObject();
            huff.clear();
            QVector<int> w; for (auto v : s["weights"].toArray()) w.push_back(v.toInt());
            if (!w.isEmpty()) { huff.buildFromWeights(w.data(), w.size()); huffLastWeights_ = w; }
        }

        // 依据当前右侧模块选择刷新画布到该模块的“上一次状态”
        onModuleChanged(moduleCombo ? moduleCombo->currentIndex() : 0);
        statusBar()->showMessage(QString("已打开（全部数据结构已恢复）：%1").arg(path));
    }
}

// 新增：模块切换时同步画布为对应数据结构的上一次状态（若无则显示“空”）
void MainWindow::onModuleChanged(int index) {
    // 停止动画，直接展示该模块最近一次的状态
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene();

    switch (index) {
    case 0: // 顺序表
        currentKind_ = DocKind::SeqList;
        if (seq.size() > 0) { drawSeqlist(seq); view->setTitle(QStringLiteral("顺序表")); }
        else { view->setTitle(QStringLiteral("顺序表（空）")); }
        break;
    case 1: // 单链表
        currentKind_ = DocKind::LinkedList;
        if (link.size() > 0) { drawLinklist(link); view->setTitle(QStringLiteral("单链表")); }
        else { view->setTitle(QStringLiteral("单链表（空）")); }
        break;
    case 2: // 栈
        currentKind_ = DocKind::Stack;
        if (st.size() > 0) { drawStack(st); view->setTitle(QStringLiteral("顺序栈")); }
        else { view->setTitle(QStringLiteral("顺序栈（空）")); }
        break;
    case 3: // 普通二叉树
        currentKind_ = DocKind::BinaryTree;
        if (bt.root()) { drawBT(bt.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("二叉树")); }
        else { view->setTitle(QStringLiteral("二叉树（空）")); }
        break;
    case 4: // BST
        currentKind_ = DocKind::BST;
        if (bst.root()) { drawBT(bst.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("BST")); }
        else { view->setTitle(QStringLiteral("BST（空）")); }
        break;
    case 5: // Huffman
        currentKind_ = DocKind::Huffman;
        if (huff.root()) { drawBT(huff.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("哈夫曼树")); }
        else { view->setTitle(QStringLiteral("哈夫曼树（空）")); }
        break;
    case 6: // AVL
        currentKind_ = DocKind::AVL;
        if (avl.root()) { drawBT(avl.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("AVL")); }
        else { view->setTitle(QStringLiteral("AVL（空）")); }
        break;
    case 7: // DSL（不绑定具体结构）
        currentKind_ = DocKind::None;
        view->setTitle(QStringLiteral("脚本/DSL"));
        break;
    default:
        break;
    }
}

// ================== DSL / 自然语言 ==================
void MainWindow::insertDSLExample() {
    // 弹出一个只读文档页面，详细说明 DSL 用法
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("DSL 使用说明"));
    dlg->resize(780, 620);
    dlg->setModal(false);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);

    auto* v = new QVBoxLayout(dlg);

    auto* doc = new QTextBrowser(dlg);
    doc->setOpenExternalLinks(false);
    doc->setStyleSheet(
        "QTextBrowser{border:2px solid #e2e8f0;border-radius:10px;padding:12px;background:white;}"
    );

    const QString html = QString::fromUtf8(R"HTML(
<h2 style="margin:0 0 12px 0;">DSL 使用说明</h2>
<p>规则：</p>
<ul>
  <li>每行一条命令；大小写不敏感；数字可用空格或逗号分隔。</li>
  <li>普通二叉树层序构建支持哨兵：<code>null=-1</code>，可省略（默认 -1）。</li>
  <li>命令按顺序逐条执行；一条命令的动画结束后自动进入下一条。</li>
</ul>

<h3>顺序表（Seqlist）</h3>
<pre><code>seq 1 3 5 7
seq.insert pos value
seq.erase pos
seq.clear
</code></pre>

<h3>单链表（Linklist）</h3>
<pre><code>link 2 4 6 8
link.insert pos value
link.erase pos
link.clear
</code></pre>

<h3>顺序栈（Stack）</h3>
<pre><code>stack 3 8 13
stack.push value
stack.pop
stack.clear
</code></pre>

<h3>普通二叉树（Binary Tree，层序+哨兵）</h3>
<pre><code>bt 15 6 23 4 -1 -1 7  null=-1
bt.preorder
bt.inorder
bt.postorder
bt.levelorder
bt.clear
</code></pre>

<h3>二叉搜索树（BST）</h3>
<pre><code>bst 15 6 23 4 7 17 71
bst.find x
bst.insert x
bst.erase x
bst.clear
</code></pre>

<h3>哈夫曼树（Huffman）</h3>
<pre><code>huff 5 9 12 13 16 45
huff.clear
</code></pre>

<h3>AVL 树</h3>
<pre><code>avl 10 20 30 40 50 25
avl.insert x
avl.clear
</code></pre>


)HTML");

    doc->setHtml(html);

    auto* btnBar = new QHBoxLayout;
    auto* btnClose = new QPushButton(QStringLiteral("关闭"));
    btnClose->setStyleSheet("QPushButton{background:#ef4444;color:white;}");

    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::close);

    btnBar->addStretch(1);
    btnBar->addWidget(btnClose);

    v->addWidget(doc, 1);
    v->addLayout(btnBar);

    dlg->show();
}

void MainWindow::runDSL() {
    // 读取 DSL 文本
    const QString all = dslEdit->toPlainText();

    // 统一在算法模块中做校验
    QString errorTitle, errorDialog, statusBarMsg;
    bool ok = dsl::validateDslScript(all, &errorTitle, &errorDialog, &statusBarMsg);
    if (!ok) {
        // 判断是否为“多种数据结构混用”的错误
        const bool isMultiFamilyError =
            statusBarMsg.contains(QStringLiteral("包含多种数据结构"))
            || errorDialog.contains(QStringLiteral("多种数据结构"));

        if (isMultiFamilyError) {
            // —— 多种数据结构混用：给用户一个“忽略并继续 / 取消执行”的选择 ——
            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle(errorTitle.isEmpty()
                                  ? QStringLiteral("输入有误")
                                  : errorTitle);

            QString text = errorDialog;
            if (text.isEmpty()) {
                text = QStringLiteral(
                    "检测到 DSL 同时包含多种数据结构的操作。\n\n"
                    "建议一次仅操作一种数据结构，以便观察动画效果和理解算法。\n\n"
                    "你可以选择忽略警告继续执行，或者取消本次执行。"
                );
            } else {
                text += QStringLiteral(
                    "\n\n是否忽略上述警告并继续执行？"
                );
            }
            msgBox.setText(text);

            QPushButton* ignoreButton = msgBox.addButton(
                QStringLiteral("忽略并继续执行"), QMessageBox::AcceptRole);
            QPushButton* cancelButton = msgBox.addButton(
                QStringLiteral("取消执行"), QMessageBox::RejectRole);
            msgBox.setDefaultButton(cancelButton);

            msgBox.exec();

            if (msgBox.clickedButton() == cancelButton) {
                // 用户选择取消：保持原有“校验失败并已取消”的语义
                if (!statusBarMsg.isEmpty())
                    statusBar()->showMessage(statusBarMsg);
                else
                    statusBar()->showMessage(QStringLiteral("已取消执行 DSL 脚本"));
                return;
            } else if (msgBox.clickedButton() == ignoreButton) {
                // 用户选择忽略：给一个轻量提示然后继续往下执行 DSL
                statusBar()->showMessage(QStringLiteral("已忽略“多种数据结构混用”的校验警告，正在继续执行…"));
            }
            // 不 return，继续往下解析/执行 DSL
        } else {
            // —— 其他类型错误：保持原来的“直接拦截”行为，例如单行多条指令/带分隔符 ——
            if (!errorTitle.isEmpty())
                QMessageBox::warning(this, errorTitle, errorDialog);
            else
                QMessageBox::warning(this, QStringLiteral("输入有误"), errorDialog);

            if (!statusBarMsg.isEmpty())
                statusBar()->showMessage(statusBarMsg);
            return;
        }
    }

    // 校验通过，再按行拆分（后面原有解析 + 执行动画的代码全部保持不变）
    QStringList lines = all.split('\n', Qt::SkipEmptyParts);

    // =============== 解析与执行计划 ===============
    QVector<std::function<void()>> ops;
    ops.reserve(lines.size());

    auto asNumbers = [this](const QString& s)->QVector<int>{
        return parseIntList(s); // 已有：正则抽取所有整数，支持空格/逗号
    };

    auto normalized = [](QString s){
        s = s.trimmed();
        s.replace(QRegularExpression("[\\t,]+"), " "); // 制表符/逗号 → 空格
        return s.toLower();
    };

    for (const QString& ln : lines) {
        QString s = normalized(ln);
        if (s.isEmpty()) continue;

        // ================= 顺序表 =================
        if (s.startsWith("seq.insert")) {
            // seq.insert pos val
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 3) {
                bool ok1=false, ok2=false;
                int pos = tokens.value(1).toInt(&ok1);
                int val = tokens.value(2).toInt(&ok2);
                if (ok1 && ok2) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::SeqList;
                        seqlistPosition->setValue(pos);
                        seqlistValue->setText(QString::number(val));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        seqlistInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("seq.erase")) {
            // seq.erase pos
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int pos = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::SeqList;
                        seqlistPosition->setValue(pos);
                        timer.stop(); steps.clear(); stepIndex = 0;
                        seqlistErase();
                    });
                    continue;
                }
            }
        }
        if (s == "seq.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::SeqList;
                timer.stop(); steps.clear(); stepIndex = 0;
                seqlistClear();
            });
            continue;
        }
        if (s.startsWith("seq ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i) {
                if(i) numbers+=' ';
                numbers+=QString::number(a[i]);
            }
            ops.push_back([=, this](){
                currentKind_ = DocKind::SeqList;
                seqlistInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                seqlistBuild();
            });
            continue;
        }

        // ================= 单链表 =================
        if (s.startsWith("link.insert")) {
            // link.insert pos val
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 3) {
                bool ok1=false, ok2=false;
                int pos = tokens.value(1).toInt(&ok1);
                int val = tokens.value(2).toInt(&ok2);
                if (ok1 && ok2) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::LinkedList;
                        linklistPosition->setValue(pos);
                        linklistValue->setText(QString::number(val));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        linklistInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("link.erase")) {
            // link.erase pos
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int pos = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::LinkedList;
                        linklistPosition->setValue(pos);
                        timer.stop(); steps.clear(); stepIndex = 0;
                        linklistErase();
                    });
                    continue;
                }
            }
        }
        if (s == "link.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::LinkedList;
                timer.stop(); steps.clear(); stepIndex = 0;
                linklistClear();
            });
            continue;
        }
        if (s.startsWith("link ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=, this](){
                currentKind_ = DocKind::LinkedList;
                linklistInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                linklistBuild();
            });
            continue;
        }

        // ================= 栈 =================
        if (s.startsWith("stack.push")) {
            // stack.push v
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::Stack;
                        stackValue->setText(QString::number(v));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        stackPush();
                    });
                    continue;
                }
            }
        }
        if (s == "stack.pop") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::Stack;
                timer.stop(); steps.clear(); stepIndex = 0;
                stackPop();
            });
            continue;
        }
        if (s == "stack.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::Stack;
                timer.stop(); steps.clear(); stepIndex = 0;
                stackClear();
            });
            continue;
        }
        if (s.startsWith("stack ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=, this](){
                currentKind_ = DocKind::Stack;
                stackInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                stackBuild();
            });
            continue;
        }

        // ================= 二叉树（bt）=================
        if (s == "bt.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::BinaryTree;
                timer.stop(); steps.clear(); stepIndex = 0;
                btClear();
            });
            continue;
        }
        if (s.startsWith("bt.preorder"))   { ops.push_back([=, this](){ timer.stop(); steps.clear(); stepIndex=0; btPreorder();   }); continue; }
        if (s.startsWith("bt.inorder"))    { ops.push_back([=, this](){ timer.stop(); steps.clear(); stepIndex=0; btInorder();    }); continue; }
        if (s.startsWith("bt.postorder"))  { ops.push_back([=, this](){ timer.stop(); steps.clear(); stepIndex=0; btPostorder();  }); continue; }
        if (s.startsWith("bt.levelorder")) { ops.push_back([=, this](){ timer.stop(); steps.clear(); stepIndex=0; btLevelorder(); }); continue; }

        if (s.startsWith("bt ")) {
            // 支持 bt ... null=x（默认 -1）
            QRegularExpression mNull(R"(null\s*=\s*(-?\d+))");
            auto m = mNull.match(s);
            int sentinel = -1;
            if (m.hasMatch()) sentinel = m.captured(1).toInt();

            auto a = asNumbers(s);
            // 若末尾是哨兵值且刚好是通过 null= 指定的，把它从序列里去掉
            if (m.hasMatch() && !a.isEmpty() && a.back() == sentinel) a.pop_back();

            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            const int nullSent = sentinel;

            ops.push_back([=, this](){
                currentKind_ = DocKind::BinaryTree;
                btInput->setText(numbers);
                btNull->setValue(nullSent);
                timer.stop(); steps.clear(); stepIndex = 0;
                btBuild();
            });
            continue;
        }

        // ================= BST =================
        if (s == "bst.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::BST;
                timer.stop(); steps.clear(); stepIndex = 0;
                bstClear();
            });
            continue;
        }
        if (s.startsWith("bst.find") || s.startsWith("bst.insert") || s.startsWith("bst.erase")) {
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    if (tokens[0] == "bst.find") {
                        ops.push_back([=, this](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstFind();
                        });
                        continue;
                    }
                    if (tokens[0] == "bst.insert") {
                        ops.push_back([=, this](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstInsert();
                        });
                        continue;
                    }
                    if (tokens[0] == "bst.erase") {
                        ops.push_back([=, this](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstErase();
                        });
                        continue;
                    }
                }
            }
        }
        if (s.startsWith("bst ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=, this](){
                currentKind_ = DocKind::BST;
                bstInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                bstBuild();
            });
            continue;
        }

        // ================= Huffman =================
        if (s == "huff.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::Huffman;
                timer.stop(); steps.clear(); stepIndex = 0;
                huffmanClear();
            });
            continue;
        }
        if (s.startsWith("huff ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=, this](){
                currentKind_ = DocKind::Huffman;
                huffmanInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                huffmanBuild();
            });
            continue;
        }

        // ================= AVL =================
        if (s == "avl.clear") {
            ops.push_back([=, this](){
                currentKind_ = DocKind::AVL;
                timer.stop(); steps.clear(); stepIndex = 0;
                avlClear();
            });
            continue;
        }
        if (s.startsWith("avl.insert")) {
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=, this](){
                        currentKind_ = DocKind::AVL;
                        avlValue->setText(QString::number(v));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        avlInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("avl ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=, this](){
                currentKind_ = DocKind::AVL;
                avlInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                avlBuild();
            });
            continue;
        }

        // 未识别：给出提示，不中断其它行
        ops.push_back([=, this](){ showMessage(QStringLiteral("未识别 DSL：%1").arg(ln.trimmed())); });
    }

    if (ops.isEmpty()) {
        showMessage(QStringLiteral("DSL：没有可执行的命令"));
        return;
    }

    // =============== 串行执行：每条命令的动画最后接“继续下一条” ===============
    auto runNext = std::make_shared<std::function<void()>>();
    *runNext = [this, ops, runNext]() mutable {
        if (ops.isEmpty()) {
            showMessage(QStringLiteral("DSL：执行完成"));
            return;
        }
        auto op = ops.takeFirst();
        op();
        QTimer::singleShot(0, this, [this, runNext](){
            if (steps.isEmpty()) {
                (*runNext)();
            } else {
                steps.push_back([runNext](){ (*runNext)(); });
                if (!timer.isActive()) timer.start();
            }
        });
    };

    // 开始执行第一条
    (*runNext)();
}

void MainWindow::runLLM()
{
    if (!llmEdit) {
        QMessageBox::warning(this, QStringLiteral("LLM"), QStringLiteral("LLM：界面未初始化"));
        return;
    }

    QString llmText = llmEdit->toPlainText().trimmed();
    if (llmText.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("LLM"), QStringLiteral("LLM：请输入自然语言指令"));
        return;
    }

    // ====== 彩蛋：如果输入中包含“苏航” ======
    if (llmText.contains(QStringLiteral("苏航"))) {
        QMessageBox::information(
            this,
            QStringLiteral("彩蛋"),
            QStringLiteral("苏航老师最帅")
        );
    }

    if (!llmClient) {
        QMessageBox::warning(this, QStringLiteral("LLM"), QStringLiteral("LLM：大模型客户端未初始化"));
        return;
    }

    // 清空旧的 DSL 内容，方便观察新生成的脚本
    if (dslEdit) dslEdit->clear();

    showMessage(QStringLiteral("LLM：正在调用大模型，将自然语言转换为DSL，请稍后。"));

    // ====== 显示“正在调用大模型”的提示弹窗 ======
    if (!llmProgressDialog) {
        llmProgressDialog = new QDialog(this);
        llmProgressDialog->setWindowTitle(QStringLiteral("LLM"));
        llmProgressDialog->setModal(true);                     // 调用期间禁止其它操作
        llmProgressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false); // 不允许手动关
        llmProgressDialog->setFixedSize(260, 100);

        auto* layout = new QVBoxLayout(llmProgressDialog);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->addStretch(1);

        auto* label = new QLabel(QStringLiteral("正在调用大模型，请稍候..."), llmProgressDialog);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        layout->addStretch(1);
    }

    llmProgressDialog->show();
    llmProgressDialog->raise();
    llmProgressDialog->activateWindow();

    // 调用大模型，把自然语言转成 DSL
    llmClient->callModel(llmText);
}

// 大模型成功返回 DSL 时调用
void MainWindow::onLlmDslReady(const QString& dslText)
{
    // 先关闭“正在调用”的弹窗
    if (llmProgressDialog) {
        llmProgressDialog->hide();
    }

    // 有些模型可能会把 DSL 包在 ``` 里，这里简单做一下清理
    QString cleaned = dslText.trimmed();

    if (cleaned.startsWith("```")) {
        int firstNewline = cleaned.indexOf('\n');
        if (firstNewline != -1) {
            cleaned = cleaned.mid(firstNewline + 1);
        }
        int lastFence = cleaned.lastIndexOf("```");
        if (lastFence != -1) {
            cleaned = cleaned.left(lastFence);
        }
        cleaned = cleaned.trimmed();
    }

    if (cleaned.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("LLM"),
                             QStringLiteral("LLM：大模型返回的DSL为空"));
        showMessage(QStringLiteral("LLM：大模型返回为空，未生成DSL"));
        return;
    }

    // 把 DSL 显示到编辑框中，方便用户查看/修改
    if (dslEdit) {
        dslEdit->setPlainText(cleaned);
    }

    showMessage(QStringLiteral("LLM：已由大模型生成DSL，开始执行脚本。"));

    // 直接复用现有 runDSL()
    runDSL();
}

// 大模型调用失败时的处理：提示 + 使用本地规则兜底（可选）
void MainWindow::onLlmError(const QString& message)
{
    // 先关闭“正在调用”的弹窗
    if (llmProgressDialog) {
        llmProgressDialog->hide();
    }

    QString msg = QStringLiteral("LLM：调用大模型失败：%1").arg(message);
    QMessageBox::warning(this, QStringLiteral("LLM错误"), msg);
    showMessage(msg);

    // —— 兜底逻辑：自动使用原来的规则 NLI → DSL ——
    // 如果你不想兜底，可以这里直接 return。
    if (!llmEdit) return;
    QString nliText = llmEdit->toPlainText().trimmed();
    if (nliText.isEmpty()) return;

    // 转小写，符合 dsl::nliToDsl 的预期
    QString low = nliText.toLower();

    // 复用已有的数字解析逻辑（和 runDSL 里 asNumbers 一样）
    auto parseIntListFn = [this](const QString& s) -> QVector<int> {
        return parseIntList(s);   // 已有成员函数
    };

    QString errorTitle;
    QString errorDialog;
    QStringList dslLines = dsl::nliToDsl(low, parseIntListFn, &errorTitle, &errorDialog);

    if (dslLines.isEmpty()) {
        if (!errorTitle.isEmpty()) {
            QMessageBox::warning(this, errorTitle, errorDialog);
        } else {
            QMessageBox::warning(this, QStringLiteral("NLI"),
                                 QStringLiteral("NLI：本地规则也无法解析这句话"));
        }
        return;
    }

    // 把兜底生成的 DSL 填到编辑框里，并执行
    if (dslEdit) {
        dslEdit->setPlainText(dslLines.join('\n'));
    }

    showMessage(QStringLiteral("NLI：已由本地规则生成 DSL，开始执行脚本..."));
    runDSL();
}

// ================== 辅助函数 ==================
QVector<int> MainWindow::dumpBTLevel(ds::BTNode* root, int nullSentinel) const {
    QVector<int> level;
    if(!root) return level;

    QVector<ds::BTNode*> q;
    q.reserve(128);
    q.push_back(root);
    int qi = 0, lastNonNull = -1;

    while(qi < q.size()) {
        ds::BTNode* p = q[qi++];
        if(p) {
            level.push_back(p->key);
            lastNonNull = level.size() - 1;
            q.push_back(p->left);
            q.push_back(p->right);
        } else {
            level.push_back(nullSentinel);
            q.push_back(nullptr);
            q.push_back(nullptr);
        }
        if(q.size() > 4096) break;
    }

    while(level.size() > 0 && level.back() == nullSentinel)
        level.pop_back();
    return level;
}

void MainWindow::dumpPreorder(ds::BTNode* r, QVector<int>& out) const {
    if(!r) return;
    out.push_back(r->key);
    dumpPreorder(r->left, out);
    dumpPreorder(r->right, out);
}

void MainWindow::collectLeafWeights(ds::BTNode* r, QVector<int>& out) const {
    if(!r) return;
    if(!r->left && !r->right) out.push_back(r->key);
    collectLeafWeights(r->left, out);
    collectLeafWeights(r->right, out);
}