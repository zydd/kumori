#include "shaderpipeline.h"
#include "pipelinerenderer.h"

QOpenGLFramebufferObject *PipelineRenderer::createFramebufferObject(const QSize &size) {
    qDebug() << size;
    return new QOpenGLFramebufferObject(size, QOpenGLFramebufferObject::NoAttachment);
}


ShaderPipeline::ShaderPipeline() {
    qDebug();
}

QQuickFramebufferObject::Renderer *ShaderPipeline::createRenderer() const {
    qDebug() << this;
    return  new PipelineRenderer(this);
}


void ShaderPipeline::setName(const QString &newName) {
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged();
}

const QString &ShaderPipeline::fragment() const {
    return m_fragment;
}

void ShaderPipeline::setFragment(const QString &newFragment) {
    qDebug() << this;

    if (m_fragment == newFragment)
        return;
    m_fragment = newFragment;
    emit fragmentChanged();
}

const QVector<QString> &ShaderPipeline::inputs() const {
    return m_inputs;
}

void ShaderPipeline::setInputs(const QVector<QString> &newInputs) {
    if (m_inputs == newInputs)
        return;
    m_inputs = newInputs;
    emit inputsChanged();
}

const QVector<QString> &ShaderPipeline::outputs() const {
    return m_outputs;
}

void ShaderPipeline::setOutputs(const QVector<QString> &newOutputs) {
    if (m_outputs == newOutputs)
        return;
    m_outputs = newOutputs;
    emit outputsChanged();
}

uint ShaderPipeline::repeat() const {
    return m_repeat;
}

void ShaderPipeline::setRepeat(uint newRepeat) {
    if (m_repeat == newRepeat)
        return;
    m_repeat = newRepeat;
    emit repeatChanged();
}
