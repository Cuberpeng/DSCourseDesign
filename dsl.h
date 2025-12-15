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

    // 对 DSL 文本做两类校验：一次只操作一种数据结构，每行只允许一条指令
    bool validateDslScript(const QString& all, QString* errorTitle, QString* errorDialogText, QString* statusBarMessage);

    // 自然语言 → DSL
    QStringList nliToDsl(const QString& low, const std::function<QVector<int>(const QString&)>& parseIntListFn, QString* errorTitle, QString* errorDialogText);

} // namespace dsl

#endif //DSL_H
