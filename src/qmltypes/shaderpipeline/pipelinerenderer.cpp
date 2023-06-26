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
    QVector<unsigned> inputs;
    QVector<unsigned> outputs;
    QVector<PipelineItem *> *subroutine;
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
    QFile fragfile(frag);
    fragfile.open(QFile::ReadOnly);

    auto prog = new QOpenGLShaderProgram;
    prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    prog->addShaderFromSourceCode(QOpenGLShader::Fragment, frag);
    prog->link();
    return prog;
}

void PipelineRenderer::initFbo() {
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    if (! m_fbo[0] || m_fbo[0]->size() != m_size) {
        Q_ASSERT(m_item->inputs().size() == 2);
        for (int i = 0; i < 2; ++i) {
            delete m_fbo[i];
            m_fbo[i] = new QOpenGLFramebufferObject(m_size,
                                      QOpenGLFramebufferObject::NoAttachment,
                                      GL_TEXTURE_2D, GL_RGBA16F);
            m_fbo[i]->addColorAttachment(m_size, GL_RGBA16F);
            f->glBindTexture(GL_TEXTURE_2D, m_fbo[i]->texture());
            f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        }
    }
}

void PipelineRenderer::render() {
    render(m_pipeline);
}

void PipelineRenderer::render(PipelineItem *pipeline) {
    auto f = QOpenGLContext::currentContext()->extraFunctions();

    // Render item

    m_vao->bind();
    pipeline->program->bind();

    // Bind all input FBOs
    auto const &cfbo_textures = m_fbo[m_cfbo]->textures();
    for (unsigned i = 0; i < std::min(pipeline->inputs.size(), 32); ++i) {
        auto texture_index = pipeline->inputs[i];
        f->glActiveTexture(GL_TEXTURE0 + i);
        f->glBindTexture(GL_TEXTURE_2D, cfbo_textures[texture_index]);
    }

    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao->release();

//    // Render to FBO

//    m_item->window()->resetOpenGLState();
//    f->glViewport(0, 0, m_size.width(), m_size.height());

//    m_vao->bind();

//    for (unsigned i = 0; i < m_iterations; ++i) {
////        m_fbo[!m_cfbo]->bind();
////        f->glActiveTexture(GL_TEXTURE0);
////        f->glBindTexture(GL_TEXTURE_2D, m_fbo[m_cfbo]->textures()[0]);
////        f->glActiveTexture(GL_TEXTURE1);
////        f->glBindTexture(GL_TEXTURE_2D, m_fbo[m_cfbo]->textures()[1]);

////        m_progFluid->bind();
////        f->glDrawArrays(GL_TRIANGLES, 0, 6);
////        m_fbo[!m_cfbo]->release();

//        m_cfbo = !m_cfbo;
//    }

//    m_vao->release();


    // Reset state
    m_item->window()->resetOpenGLState();
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
    auto *ret = new PipelineItem{0};

    ret->inputs = {};
    ret->outputs = {};
    ret->repeat = item->repeat();
    ret->program = linkFragment(item->fragment());

    foreach (auto const& child, item->children()) {
        if (! child->inherits("ShaderPipeline")) {
            qWarning() << "child" << child << "is not of type ShaderPipeline";
            continue;
        }

        ret->subroutine->push_back(buildPipeline(qobject_cast<ShaderPipeline *>(child)));
    }

    return ret;
}

