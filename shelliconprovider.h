#ifndef SHELLICONPROVIDER_H
#define SHELLICONPROVIDER_H

#include <qquickimageprovider.h>

class ShellIconProvider : public QQuickImageProvider {
public:
    ShellIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};


#endif // SHELLICONPROVIDER_H
