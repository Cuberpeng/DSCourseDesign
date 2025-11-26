//
// Created by xiang on 25-11-25.
//

#include "dsl.h"

#include <QSet>
#include <QRegularExpression>
#include <QStringList>

namespace dsl {

// ==================== DSL 校验 ====================

bool validateDslScript(const QString& all, QString* errorTitle, QString* errorDialogText, QString* statusBarMessage)
{
    if (errorTitle) errorTitle->clear();
    if (errorDialogText)  errorDialogText->clear();
    if (statusBarMessage) statusBarMessage->clear();

    // 先按行拆分（和原来的 runDSL 一致）
    QStringList lines = all.split('\n', Qt::SkipEmptyParts);

    // =============== 整体校验：一次只能操作一种数据结构 ===============
    {
        QSet<QString> families;
        for (const QString& rawLine : lines) {
            QString s = rawLine.trimmed().toLower();
            if (s.isEmpty()) continue;

            QRegularExpression headRe("^\\s*([a-z]+)");
            auto mm = headRe.match(s);
            if (!mm.hasMatch()) continue;
            const QString head = mm.captured(1);

            if (head == "seq"  || head == "link" || head == "stack" ||
                head == "bt"   || head == "bst"  || head == "huff"  || head == "avl") {
                families.insert(head);
            }
        }

        if (families.size() > 1) {
            QStringList lst;
            for (const auto& f : families) lst << f;

            if (errorTitle)
                *errorTitle = QStringLiteral("输入有误");
            if (errorDialogText) {
                *errorDialogText =
                    QStringLiteral("检测到 DSL 同时包含多种数据结构的操作（%1）。当前版本每次仅支持一种数据结构，请修改后重新输入。")
                        .arg(lst.join(", "));
            }
            if (statusBarMessage)
                *statusBarMessage = QStringLiteral("DSL 校验失败：包含多种数据结构");
            return false;
        }
    }

    // =============== 单行多指令校验（不允许一行包含两条或以上指令） ===============
    static const QRegularExpression kCmdTokenRe(
        R"((?<![a-z])(seq|link|stack|bt|bst|huff|avl)(?:\.[a-z]+)?(?![a-z]))",
        QRegularExpression::CaseInsensitiveOption
    );

    for (const QString& rawLine : lines) {
        const QString s = rawLine.trimmed();
        if (s.isEmpty()) continue;

        const bool hasBadSep = s.contains(';') || s.contains(u'；')
                             || s.contains("&&") || s.contains("||")
                             || s.contains(u'、') || s.contains(u'，');

        int cmdCount = 0;
        for (auto it = kCmdTokenRe.globalMatch(s); it.hasNext(); it.next())
            ++cmdCount;

        if (hasBadSep || cmdCount >= 2) {
            if (errorTitle)
                *errorTitle = QStringLiteral("输入有误");
            if (errorDialogText) {
                *errorDialogText =
                    QStringLiteral("DSL 每行仅能包含一条指令，请拆分后重新输入。\n问题行：%1")
                        .arg(rawLine.trimmed());
            }
            if (statusBarMessage)
                *statusBarMessage = QStringLiteral("DSL 校验失败：单行包含多条指令");
            return false;
        }
    }

    return true;
}

// ==================== NLI → DSL ====================

QStringList nliToDsl(const QString& low, const std::function<QVector<int>(const QString&)>& parseIntListFn, QString* errorTitle, QString* errorDialogText)
{
    if (errorTitle)      errorTitle->clear();
    if (errorDialogText) errorDialogText->clear();

    QStringList dslLines;

    const QString trimmed = low.trimmed();
    if (trimmed.isEmpty()) {
        if (errorTitle)
            *errorTitle = QStringLiteral("提示");
        if (errorDialogText)
            *errorDialogText = QStringLiteral("NLI：请输入自然语言指令");
        return dslLines;
    }

    // —— 检测是否同时提到了多种数据结构 ——（和原 runNLI 相同）
    QSet<QString> hits;
    auto hitIf = [&](const QString& key, std::initializer_list<QString> kws){
        for (const auto& k : kws) {
            if (trimmed.contains(k)) {
                hits.insert(key);
                break;
            }
        }
    };
    hitIf("seq",  {"顺序表","顺序","数组","seqlist","seq"});
    hitIf("link", {"链表","链","linklist","link"});
    hitIf("stack",{"栈","stack"});
    hitIf("bt",   {"二叉树","普通二叉树","binary tree","bt"});
    hitIf("bst",  {"二叉搜索树","binary search tree","bst"});
    hitIf("huff", {"哈夫曼","huffman","huff"});
    hitIf("avl",  {"平衡二叉树","avl"});

    if (hits.size() > 1) {
        QStringList fam;
        for (const auto& s : hits) fam << s;

        if (errorTitle)
            *errorTitle = QStringLiteral("输入不合法");
        if (errorDialogText) {
            *errorDialogText =
                QStringLiteral("NLI：同一条指令内只能包含一种数据结构（检测到：%1），请重新输入。")
                    .arg(fam.join(", "));
        }
        return dslLines;
    }

    if (hits.isEmpty()) {
        if (errorTitle)
            *errorTitle = QStringLiteral("未识别");
        if (errorDialogText) {
            *errorDialogText =
                QStringLiteral("NLI：未能识别数据结构类型，请补充如“顺序表/链表/栈/二叉树/BST/哈夫曼/AVL”等关键词。");
        }
        return dslLines;
    }

    const QString kind = *hits.begin();

    // ================== 按“然后/接着/之后/并且/同时/句号”等拆成多个子句 ==================
    QString normalized = trimmed;

    // 这些词一般用来串联多个操作；替换成句号，便于 split
    const QStringList connectors = {
        QStringLiteral("然后"),
        QStringLiteral("接着"),
        QStringLiteral("之后"),
        QStringLiteral("并"),
        QStringLiteral("并且"),
        QStringLiteral("最后"),
        QStringLiteral("同时")
    };
    for (const QString& w : connectors) {
        normalized.replace(w, QStringLiteral("。"));
    }

    // 按句号、问号、感叹号、分号和换行分割；不要用“，”以免把数字拆散
    QStringList clauses = normalized.split(
        QRegularExpression("[。！？;；\\n]+"),
        Qt::SkipEmptyParts
    );

    // 工具：把整数列表拼成 "1 2 3" 的形式
    auto joinNums = [&](const QVector<int>& a){
        QString s;
        for (int i = 0; i < a.size(); ++i) {
            if (i) s += ' ';
            s += QString::number(a[i]);
        }
        return s;
    };

    // ================== 对每个子句单独生成一条 DSL ==================
    for (QString clause : clauses) {
        clause = clause.trimmed();
        if (clause.isEmpty())
            continue;

        // 只在当前子句中找关键字和数字
        QVector<int> nums;
        if (parseIntListFn)
            nums = parseIntListFn(clause);

        auto hasAny = [&](std::initializer_list<QString> kws)->bool{
            for (const auto& k : kws)
                if (clause.contains(k))
                    return true;
            return false;
        };

        QString dsl;  // 当前子句对应的一条 DSL

        if (kind == "seq") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "seq.clear";
            } else if (hasAny({"插入","插","增","insert"})) {
                if (nums.size() >= 2)
                    dsl = QString("seq.insert %1 %2").arg(nums[0]).arg(nums[1]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("seq.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "seq " + joinNums(nums);
            }
        }
        else if (kind == "link") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "link.clear";
            } else if (hasAny({"插入","插","增","insert"})) {
                if (nums.size() >= 2)
                    dsl = QString("link.insert %1 %2").arg(nums[0]).arg(nums[1]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("link.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "link " + joinNums(nums);
            }
        }
        else if (kind == "stack") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "stack.clear";
            } else if (hasAny({"出栈","弹栈","pop"})) {
                dsl = "stack.pop";
            } else if (hasAny({"入栈","压栈","push","加入","添加","增"})) {
                if (nums.size() >= 1)
                    dsl = QString("stack.push %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "stack " + joinNums(nums);
            }
        }
        else if (kind == "bt") {
            if (hasAny({"先序","前序","preorder"})) {
                dsl = "bt.preorder";
            } else if (hasAny({"中序","inorder"})) {
                dsl = "bt.inorder";
            } else if (hasAny({"后序","postorder"})) {
                dsl = "bt.postorder";
            } else if (hasAny({"层序","层次","广度","levelorder"})) {
                dsl = "bt.levelorder";
            } else if (hasAny({"清空","清除","clear"})) {
                dsl = "bt.clear";
            } else if (!nums.isEmpty()) {
                // 默认哨兵 -1（与 BT DSL 约定保持一致）
                dsl = "bt " + joinNums(nums) + " null=-1";
            }
        }
        else if (kind == "bst") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "bst.clear";
            } else if (hasAny({"查找","寻找","搜索","find","search"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.find %1").arg(nums[0]);
            } else if (hasAny({"插入","插","加入","添加","insert","add"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.insert %1").arg(nums[0]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "bst " + joinNums(nums);
            }
        }
        else if (kind == "huff") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "huff.clear";
            } else if (!nums.isEmpty()) {
                dsl = "huff " + joinNums(nums);
            }
        }
        else if (kind == "avl") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "avl.clear";
            } else if (hasAny({"插入","插","加入","添加","insert","add"})) {
                if (nums.size() >= 1)
                    dsl = QString("avl.insert %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "avl " + joinNums(nums);
            }
        }

        if (!dsl.isEmpty())
            dslLines << dsl;
    }

    // 如果所有子句都没生成有效 DSL，则给出提示
    if (dslLines.isEmpty()) {
        if (errorTitle)
            *errorTitle = QStringLiteral("信息不足");
        if (errorDialogText) {
            *errorDialogText =
                QStringLiteral("NLI：无法从该句生成 DSL，请补充必要的信息（例如位置/值/遍历方式等）。");
        }
    }

    return dslLines;
}

} // namespace dsl