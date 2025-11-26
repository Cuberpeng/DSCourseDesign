//
// Created by xiang on 25-11-25.
//

#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class LLMClient : public QObject
{
    Q_OBJECT
public:
    explicit LLMClient(QObject* parent = nullptr);

    // 方便以后换 key / url / model
    void setApiKey(const QString& key) { apiKey_ = key; }
    void setApiUrl(const QString& url) { apiUrl_ = url; }
    void setModel(const QString& model) { model_ = model; }

    // 调用大模型，把自然语言 prompt 转成 DSL
    void callModel(const QString& prompt);

    signals:
        void finished(const QString& dslText);  // 成功返回 DSL 脚本
    void error(const QString& message);     // 出错信息（网络/解析/配置）

    private slots:
        void onReplyFinished();

private:
    QNetworkAccessManager manager_;
    QString apiKey_;
    QString apiUrl_;
    QString model_;

    QString buildSystemPrompt() const;
};

#endif //LLMCLIENT_H
