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
    QOpenGLFramebufferObject *m_fbo[2];
    QOpenGLVertexArrayObject *m_vao;
    unsigned m_cfbo;
    unsigned m_iterations;
    bool m_initialized = false;

    void initFbo();
    QOpenGLShaderProgram *linkFragment(QString const& frag);
};

#endif // PIPELINERENDERER_H
