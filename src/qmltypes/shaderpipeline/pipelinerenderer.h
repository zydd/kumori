/**************************************************************************
 *  pipelinerenderer.h
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

#ifndef PIPELINERENDERER_H
#define PIPELINERENDERER_H

#include <qquickwindow.h>
#include <qquickframebufferobject.h>

#include <QtOpenGL/QtOpenGL>
#include <GLES3/gl3.h>

struct PipelineItem;
class ShaderPipeline;

class PipelineRenderer : public QQuickFramebufferObject::Renderer {
public:
    PipelineRenderer(const ShaderPipeline *parent);
    virtual void synchronize(QQuickFramebufferObject *item) override;
    virtual void render() override;
    void render(PipelineItem *pipeline);
    virtual QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    PipelineItem *buildPipeline(ShaderPipeline *item);

private:
    QSize m_size;

    const ShaderPipeline *m_item;
    PipelineItem *m_pipeline;
    QOpenGLBuffer *m_vbo;
    QOpenGLFramebufferObject *m_fbo[2] = {0};
    QOpenGLVertexArrayObject *m_vao;
    unsigned m_cfbo = 0;
    bool m_initialized = false;

    void initFbo();
    QOpenGLShaderProgram *linkFragment(QString const& frag);
};

#endif // PIPELINERENDERER_H
