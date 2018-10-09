#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTransform>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(const QString &fname, QWidget *parent);
    ~GLWidget() override;

    QSize sizeHint() const;
public Q_SLOTS:
    void loadImage(const QString &fname);

protected:
    void resizeGL(int width, int height) override;
    void initializeGL() override;
    void paintGL() override;

protected Q_SLOTS:
    void teardownGL();

private:

    void printVersionInformation();

    QString m_defaultFile;

    QTransform m_transform;

    int m_width {0};
    int m_height {0};

    QVector<QOpenGLTexture*> m_tiles;

    QOpenGLShaderProgram *m_program {0};
    QOpenGLBuffer m_verticesBuffer;
    QOpenGLBuffer m_textureVerticesBuffer;
    QOpenGLVertexArrayObject m_vao;
    QImage m_image;
};


#endif
