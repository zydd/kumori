/**************************************************************************
 *  pipelinerenderer.cpp
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

#include "pipelinerenderer.h"
#include "shaderpipeline.h"


static const char *vertexShaderSource =
    "#version 300 es\n"
    "in vec2 vertex;\n"
    "out vec2 uv;\n"

    "void main() {\n"
    "   uv = vertex/vec2(2, 2) + vec2(0.5);\n"
    "   gl_Position = vec4(vertex, 0, 1);\n"
    "}\n";


struct PipelineItem {
    unsigned repeat;
    QOpenGLShaderProgram *program;
    QVector<int> inputs;
    QVector<int> outputs;
    QVector<PipelineItem *> subroutine;
};


PipelineRenderer::PipelineRenderer(const ShaderPipeline *parent)
    : m_item(parent)
{
    qDebug() << parent;

    auto f = QOpenGLContext::currentContext()->functions();
    qDebug() << (const char *) f->glGetString(GL_RENDERER)
             << (const char *) f->glGetString(GL_VERSION);

    m_vao = new QOpenGLVertexArrayObject;
    if (m_vao->create())
        m_vao->bind();

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->bind();

    static GLfloat vertices[] = {
        -1,  1,
         1, -1,
         1,  1,
        -1,  1,
        -1, -1,
         1, -1,
    };

    m_vbo->allocate(vertices, sizeof(vertices));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    m_vbo->release();

    m_size = parent->size().toSize();
    initFbo();
}

QOpenGLShaderProgram *PipelineRenderer::linkFragment(QString const& frag) {
    qDebug();

    QFile fragfile(frag);
    fragfile.open(QFile::ReadOnly);

    auto prog = new QOpenGLShaderProgram;
    prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    prog->addShaderFromSourceCode(QOpenGLShader::Fragment, frag);
    prog->link();
    return prog;
}

void PipelineRenderer::initFbo() {
    qDebug();

    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    if (! m_fbo[0] || m_fbo[0]->size() != m_size) {
        Q_ASSERT(m_item->inputs().size() == 2);
        for (int i = 0; i < 2; ++i) {
            qDebug() << "init FBO" << i << m_size;

            delete m_fbo[i];
            m_fbo[i] = new QOpenGLFramebufferObject(m_size,
                                      QOpenGLFramebufferObject::NoAttachment,
                                      GL_TEXTURE_2D, GL_RGBA16F);

            qDebug() << "add attachment";
            m_fbo[i]->addColorAttachment(m_size, GL_RGBA16F);

            f->glBindTexture(GL_TEXTURE_2D, m_fbo[i]->texture());
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }
}

void PipelineRenderer::render() {
    render(m_pipeline);
    m_item->window()->resetOpenGLState();
}

void PipelineRenderer::render(PipelineItem *pipeline) {
    if (pipeline->program) {
        auto f = QOpenGLContext::currentContext()->extraFunctions();

//        // if `outputs` is not empty render to FBO
//        if (!pipeline->outputs.empty()) {
//            m_item->window()->resetOpenGLState();
//            f->glViewport(0, 0, m_size.width(), m_size.height());
//        }

        m_vao->bind();
        pipeline->program->bind();

        const QVector<GLuint> cfbo_textures[2] = {
            m_fbo[0]->textures(),
            m_fbo[1]->textures()
        };

        m_fbo[!m_cfbo]->bind();

        for (int i = 0; i < std::min(pipeline->inputs.size(), 32); ++i) {
            // FIXME: only bind used textures
            Q_ASSERT(m_item->inputs().size() == 2);
            f->glActiveTexture(GL_TEXTURE0 + i);
            f->glBindTexture(GL_TEXTURE_2D, cfbo_textures[m_cfbo][i]);

            if (pipeline->inputs[i] >= 0)
                pipeline->program->setUniformValue(pipeline->inputs[i], GL_TEXTURE0 + i);
        }

        f->glDrawArrays(GL_TRIANGLES, 0, 6);
        m_fbo[!m_cfbo]->release();

        m_vao->release();

        m_cfbo = !m_cfbo;
    } else if (!pipeline->subroutine.empty()) {
        foreach (PipelineItem *subroutine, pipeline->subroutine) {
            render(subroutine);
        }
    }

    update();
}

void PipelineRenderer::synchronize(QQuickFramebufferObject *item) {
    qDebug() << item;
    auto shaderPipeline = qobject_cast<ShaderPipeline *>(item);
    if (!shaderPipeline) return;
    bool reinit_buffer = false;

    if (!m_initialized) {
        m_pipeline = buildPipeline(shaderPipeline);
    }

//    m_displayShader->bind();
//    m_displayShader->setUniformValue(0,GLint(sim->m_display));
//    m_progFluid->bind();
    // set uniforms for all shaders

//    m_progFluid->release();

//    if (m_initialTexture != shaderPipeline->m_initialTexture) {
//        m_initialTexture = shaderPipeline->m_initialTexture;
//        reinit_buffer = true;
//    }

    if (reinit_buffer) {
//        initializeBuffer();
    }

    m_initialized = true;
}


PipelineItem * PipelineRenderer::buildPipeline(ShaderPipeline *item) {
    qDebug() << item;

    // FIXME: use managed pointers to fix memeory leaks
    auto *pipeline = new PipelineItem{0};

    pipeline->inputs = {};
    pipeline->outputs = {};
    pipeline->repeat = item->repeat();

    if (!item->fragment().isNull()) {
        pipeline->program = linkFragment(item->fragment());
        pipeline->program->bind();

        foreach (auto const& input, item->inputs()) {
            auto uniform = pipeline->program->uniformLocation("dom");
            if (uniform < 0) {
                qCritical() << "invalid shader input:" << input;
            }
            pipeline->inputs.push_back(uniform);
        }
    }

    foreach (auto const& child, item->children()) {
        if (! child->inherits("ShaderPipeline")) {
            qWarning() << "child" << child << "is not of type ShaderPipeline";
            continue;
        }

        pipeline->subroutine.push_back(buildPipeline(qobject_cast<ShaderPipeline *>(child)));
    }

    if (pipeline->program && !pipeline->subroutine.empty()) {
        delete pipeline->program;
        pipeline->program = nullptr;
        pipeline->subroutine.clear();
        qCritical() << "cannot have fragment shader and subroutines in same ShaderPipeline";
    }

    return pipeline;
}

