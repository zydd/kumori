#include "kumori.h"

#include <qcoreapplication.h>
#include <qstandardpaths.h>

Kumori *Kumori::m_instance = nullptr;

QString Kumori::userImportDir() {
    auto userImports = m_config.value("userImports").toString();

    if (userImports.isNull()) {
        userImports = QStringLiteral("%1/%2")
                .arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                .arg(qApp->applicationName());

        m_config.setValue("userImports", userImports);
    }

    return userImports;
}
