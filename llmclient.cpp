//
// Created by xiang on 25-11-25.
//
#include "llmclient.h"

#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>

LLMClient::LLMClient(QObject* parent)
    : QObject(parent)
{
    // 1. API Key：优先用环境变量，没有再用你在代码里手动写的
    if (apiKey_.isEmpty()) {
        QByteArray envKey = qgetenv("DEEPSEEK_API_KEY");
        if (!envKey.isEmpty()) {
            apiKey_ = QString::fromLocal8Bit(envKey);
        } else {
            // 这里保持你现在的写法：直接写死 key
            // !!! 注意：提交作业前记得删掉真实 key !!!
            apiKey_ = "sk-f450ccf8fcc3487eb6b3e4612ded2619"; // <-- 换成你的
            //apiKey_ = QString::fromLocal8Bit(qgetenv("sk-d8c61f93a7ae4f77b096ce42b7efd426"));
        }
    }

    // 2. DeepSeek API 地址 & 模型
    // 官方文档示例：POST https://api.deepseek.com/chat/completions
    apiUrl_ = "https://api.deepseek.com/chat/completions";
    model_  = "deepseek-chat";
}

QString LLMClient::buildSystemPrompt() const
{
    QString prompt = QString::fromUtf8(
        "你现在是一个“自然语言 → DSL 脚本”的转换器。\n"
        "用户会用中文或英文描述数据结构的操作，你需要把它翻译成下面这种 DSL，"
        "并且严格遵守规则：\n"
        "1. 只输出 DSL 代码，不能输出解释、注释或其他文字。\n"
        "2. 每行一条命令，命令不区分大小写。\n"
        "3. 一次脚本中，只能操作一种数据结构（顺序表 / 单链表 / 栈 / 普通二叉树 / BST / 哈夫曼树 / AVL）。\n"
        "   如果用户同时提了多种结构，请选择其中最核心的一种来生成脚本，忽略其他结构。\n"
        "4. 不要使用你没见过的命令名，只用下面给出的命令。\n"
        "\n"
        "【顺序表 Seqlist】\n"
        "  初始建立:    seq 1 3 5 7\n"
        "  插入元素:    seq.insert pos value   （pos 为下标，从 0 开始）\n"
        "  删除元素:    seq.erase pos\n"
        "  清空:        seq.clear\n"
        "\n"
        "【单链表 Linklist】\n"
        "  初始建立:    link 2 4 6 8\n"
        "  插入元素:    link.insert pos value\n"
        "  删除元素:    link.erase pos\n"
        "  清空:        link.clear\n"
        "\n"
        "【顺序栈 Stack】\n"
        "  初始建立:    stack 3 8 13\n"
        "  入栈:        stack.push value\n"
        "  出栈:        stack.pop\n"
        "  清空:        stack.clear\n"
        "\n"
        "【普通二叉树 BT】\n"
        "  建立:        bt 1 2 3 -1 4 -1 5   （-1 表示空结点，可配置）\n"
        "  如果没有特意提及，新增节点时不要使用空结点 \n"
        "  先序遍历:    bt.preorder\n"
        "  中序遍历:    bt.inorder\n"
        "  后序遍历:    bt.postorder\n"
        "  层序遍历:    bt.levelorder\n"
        "  清空:        bt.clear\n"
        "\n"
        "【二叉搜索树 BST】\n"
        "  建立:        bst 15 6 23 4 7 17 71\n"
        "  查找:        bst.find value\n"
        "  插入:        bst.insert value\n"
        "  删除:        bst.erase value\n"
        "  清空:        bst.clear\n"
        "\n"
        "【哈夫曼树 Huffman】\n"
        "  建立:        huff 5 9 12 13 16 45\n"
        "  清空:        huff.clear\n"
        "\n"
        "【AVL 树】\n"
        "  建立:        avl 10 20 30 40\n"
        "  插入:        avl.insert value\n"
        "  清空:        avl.clear\n"
        "\n"
        "请根据用户的自然语言，选择一种最合适的数据结构，并用上述 DSL 命令写出脚本。\n"
        "再次强调：一次脚本只操作一种数据结构；只输出 DSL 代码本身。\n"
    );
    return prompt;
}

void LLMClient::callModel(const QString& prompt)
{
    if (apiKey_.isEmpty()) {
        emit error(QStringLiteral(
            "未配置大模型 API 密钥，请在环境变量 DEEPSEEK_API_KEY 中设置，"
            "或在程序中调用 LLMClient::setApiKey。"));
        return;
    }

    if (apiUrl_.isEmpty() || model_.isEmpty()) {
        emit error(QStringLiteral("API 地址或模型名未配置。"));
        return;
    }

    QNetworkRequest request{ QUrl(apiUrl_) };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey_.toUtf8());
    request.setRawHeader("Accept", "application/json");

    // 组装 DeepSeek Chat Completion 请求体
    QJsonObject root;
    root["model"] = model_;

    QJsonArray messages;
    QJsonObject sys;
    sys["role"] = "system";
    sys["content"] = buildSystemPrompt();
    messages.append(sys);

    QJsonObject user;
    user["role"] = "user";
    user["content"] = prompt;
    messages.append(user);

    root["messages"] = messages;
    root["stream"]   = false;

    QJsonDocument doc(root);
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = manager_.post(request, body);
    connect(reply, &QNetworkReply::finished,
            this,  &LLMClient::onReplyFinished);
}

void LLMClient::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    // 先把服务器返回的原始内容读出来（无论成功/失败都要）
    QByteArray raw = reply->readAll();

    // ========== 1. 网络 / HTTP 层错误 ==========
    if (reply->error() != QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString serverText = raw.isEmpty() ? QString() : QString::fromUtf8(raw);

        QString msg = QStringLiteral("网络错误（%1，code=%2）：%3")
                .arg(static_cast<int>(reply->error()))
                .arg(statusCode)
                .arg(reply->errorString());

        if (!serverText.isEmpty()) {
            msg += QStringLiteral("\n服务器返回：\n%1").arg(serverText);
        }

        emit error(msg);
        return;
    }

    // ========== 2. JSON 解析 ==========
    QJsonParseError jsonErr;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError || !doc.isObject()) {
        QString msg = QStringLiteral("解析 JSON 失败：%1").arg(jsonErr.errorString());
        msg += QStringLiteral("\n原始响应：\n%1").arg(QString::fromUtf8(raw));
        emit error(msg);
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty() || !choices.at(0).isObject()) {
        QString msg = QStringLiteral("响应中缺少 choices 字段。\n原始响应：\n%1")
                .arg(QString::fromUtf8(raw));
        emit error(msg);
        return;
    }

    QJsonObject choice0 = choices.at(0).toObject();
    QJsonObject msgObj  = choice0.value("message").toObject();
    QString content     = msgObj.value("content").toString().trimmed();

    if (content.isEmpty()) {
        QString msg = QStringLiteral("模型返回的 content 为空。\n原始响应：\n%1")
                .arg(QString::fromUtf8(raw));
        emit error(msg);
        return;
    }

    // 一切正常，发出去给 MainWindow::onLlmDslReady
    emit finished(content);
}