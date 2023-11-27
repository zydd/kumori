#ifndef SHADERPIPELINE_H
#define SHADERPIPELINE_H

#include <qquickframebufferobject.h>
#include <qquickitemgrabresult.h>

class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLFramebufferObject;
class ShaderPipeline;

class ShaderPipeline : public QQuickFramebufferObject {
    Q_OBJECT
public:
    friend class ShaderPipelineRenderer;

    Q_PROPERTY(bool running MEMBER m_running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString name MEMBER m_name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString fragment READ fragment WRITE setFragment NOTIFY fragmentChanged)
    Q_PROPERTY(QVector<QString> inputs READ inputs WRITE setInputs NOTIFY inputsChanged)
    Q_PROPERTY(QVector<QString> outputs READ outputs WRITE setOutputs NOTIFY outputsChanged)
    Q_PROPERTY(uint repeat READ repeat WRITE setRepeat NOTIFY repeatChanged)

    ShaderPipeline();

    void setName(const QString &newName);

    const QString &fragment() const;
    void setFragment(const QString &newFragment);

    const QVector<QString> &inputs() const;
    void setInputs(const QVector<QString> &newInputs);

    const QVector<QString> &outputs() const;
    void setOutputs(const QVector<QString> &newOutputs);

    uint repeat() const;
    void setRepeat(uint newRepeat);

public slots:
    inline void setRunning(bool running) {
        if (m_running == running) return;
        m_running = running;
        emit runningChanged(running);
        update();
    }

private:
    bool m_running = true;

    QString m_name;
    QString m_fragment;
    QVector<QString> m_inputs;
    QVector<QString> m_outputs;

    uint m_repeat;

signals:
    void runningChanged(bool running);
    void nameChanged();
    void fragmentChanged();
    void inputsChanged();
    void outputsChanged();
    void repeatChanged();

    // QQuickFramebufferObject interface
public:
    virtual Renderer *createRenderer() const override;
};

#endif // SHADERPIPELINE_H

