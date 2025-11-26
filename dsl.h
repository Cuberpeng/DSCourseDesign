//
// Created by xiang on 25-11-25.
//

#ifndef DSL_H
#define DSL_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

namespace dsl {

    // 对 DSL 文本做两类校验：
    // 1）一次只操作一种数据结构
    // 2）每行只允许一条指令
    // 返回：true = 通过；false = 有错误，并把错误信息写到指针里
    bool validateDslScript(const QString& all, QString* errorTitle, QString* errorDialogText, QString* statusBarMessage);

    // 自然语言（已转小写） → 多行 DSL
    // parseIntListFn 由 MainWindow 传进来，用来从子句中抽取数字列表
    // 出错时返回空列表，并把错误标题+正文写到 errorTitle / errorDialogText
    QStringList nliToDsl(const QString& low, const std::function<QVector<int>(const QString&)>& parseIntListFn, QString* errorTitle, QString* errorDialogText);

} // namespace dsl

#endif //DSL_H
