/**************************************************************************
 *  appbarwindow.h
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/

#pragma once

#include <qquickwindow.h>

struct AppbarWindowPrivate;

class AppbarWindow : public QQuickWindow {
    Q_OBJECT
    Q_PROPERTY(Qt::Edge edge READ edge WRITE setEdge NOTIFY edgeChanged)
    Q_PROPERTY(int thickness READ thickness WRITE setThickness NOTIFY thicknessChanged)
public:
    explicit AppbarWindow(QWindow *parent = nullptr);
    ~AppbarWindow();

    inline Qt::Edge edge() const { return _edge; }
    void setEdge(Qt::Edge newEdge);

    int thickness() const { return _thickness; }
    void setThickness(int newThickness);

private:
    AppbarWindowPrivate *d;
    Qt::Edge _edge = Qt::BottomEdge;
    bool _registered = false;

    int _thickness = 36;

    void updateGeometry();

private slots:
    void registerAppbar();
    void unregisterAppbar();

signals:
    void edgeChanged();
    void thicknessChanged();

protected:
    // QWindow interface
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    virtual void showEvent(QShowEvent *event) override;
};

