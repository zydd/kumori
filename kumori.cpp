#include "kumori.h"

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qqmlengine.h>
#include <qstandardpaths.h>

Kumori *Kumori::m_instance = nullptr;

Kumori::Kumori(QStringList args):
    QQmlPropertyMap(this, nullptr),
    m_args(args)
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    addConfig("appImportDir", qApp->applicationDirPath() + "/qml");
    addConfig("userImportDir",
              QStringLiteral("%1/%2/desktop")
              .arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
              .arg(qApp->applicationName()));
}

QString Kumori::config(QString const& key) {
    auto keys = key.split('.');

    QQmlPropertyMap *obj = instance();

    for (int i = 0; i < keys.size() - 1 && obj; ++i)
        obj = obj->value(keys[i]).value<QQmlPropertyMap *>();

    return obj ? obj->value(keys.last()).toString() : QString();
}

void Kumori::addConfig(QString key, QVariant const& defaultValue, bool setDefault, bool ignoreArgs) {
    if (key.isEmpty())
        return;

    auto config = m_config.value("config/" + key);

    if (config.isNull() && !defaultValue.isNull()) {
        config = defaultValue;

        if (setDefault)
            m_config.setValue("config/" + key, config);
    }

    if (!ignoreArgs) {
        auto argName = QStringLiteral("--%1=").arg(key);

        for (auto const& arg : m_args)
            if (arg.startsWith(argName))
                config = arg.mid(argName.size());
    }

    auto keys = key.split('.');

    QQmlPropertyMap *obj = this;
    for (int i = 0; i < keys.size() - 1; ++i) {
        if (obj->contains(keys[i])) {
            Q_ASSERT_X(obj->value(keys[i]).type() == qMetaTypeId<QObject *>(),
                       "Kumori::addConfig()", obj->value(keys[i]).typeName());

            obj = obj->value(keys[i]).value<QQmlPropertyMap *>();
        } else {
            auto sub = new QQmlPropertyMap;
            obj->insert(keys[i], QVariant::fromValue<QObject *>(sub));
            obj = sub;
        }
    }

    Q_ASSERT(!obj->contains(keys.last()));

    obj->insert(keys.last(), config);
}

void Kumori::clearComponentCache() {
    if (m_engine)
        m_engine->clearComponentCache();
    else
        qDebug() << "Kumori::clearComponentCache(): engine unavailable";
}
