#include "KisGLImageWidget.h"

#include <QPainter>
#include <QFile>
#include <QResizeEvent>
#include "kis_debug.h"

#include "KisGLImageF16.h"

namespace {
inline void rectToVertices(QVector3D* vertices, const QRectF &rc)
{
    vertices[0] = QVector3D(rc.left(),  rc.bottom(), 0.f);
    vertices[1] = QVector3D(rc.left(),  rc.top(),    0.f);
    vertices[2] = QVector3D(rc.right(), rc.bottom(), 0.f);
    vertices[3] = QVector3D(rc.left(),  rc.top(), 0.f);
    vertices[4] = QVector3D(rc.right(), rc.top(), 0.f);
    vertices[5] = QVector3D(rc.right(), rc.bottom(),    0.f);
}

inline void rectToTexCoords(QVector2D* texCoords, const QRectF &rc)
{
    texCoords[0] = QVector2D(rc.left(), rc.bottom());
    texCoords[1] = QVector2D(rc.left(), rc.top());
    texCoords[2] = QVector2D(rc.right(), rc.bottom());
    texCoords[3] = QVector2D(rc.left(), rc.top());
    texCoords[4] = QVector2D(rc.right(), rc.top());
    texCoords[5] = QVector2D(rc.right(), rc.bottom());
}
}

KisGLImageWidget::KisGLImageWidget(QWidget *parent)
    : KisGLImageWidget(QSurfaceFormat::sRGBColorSpace, parent)
{
}

KisGLImageWidget::KisGLImageWidget(QSurfaceFormat::ColorSpace colorSpace,
                                   QWidget *parent)
    : QOpenGLWidget(parent),
      m_texture(QOpenGLTexture::Target2D)
{
    setTextureFormat(GL_RGBA16F);
    setTextureColorSpace(colorSpace);

    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

void KisGLImageWidget::initializeGL()
{
    initializeOpenGLFunctions();

    QFile vertexShaderFile(QString(":/") + "kis_gl_image_widget.vert");
    vertexShaderFile.open(QIODevice::ReadOnly);
    QString vertSource = vertexShaderFile.readAll();

    QFile fragShaderFile(QString(":/") + "kis_gl_image_widget.frag");
    fragShaderFile.open(QIODevice::ReadOnly);
    QString fragSource = fragShaderFile.readAll();

    if (context()->isOpenGLES()) {
        const char *versionHelper = "#define USE_OPENGLES\n";
        vertSource.prepend(versionHelper);
        fragSource.prepend(versionHelper);

        const char *versionDefinition = "#version 100\n";
        vertSource.prepend(versionDefinition);
        fragSource.prepend(versionDefinition);
    } else {
        const char *versionDefinition = "#version 330 core\n";
        vertSource.prepend(versionDefinition);
        fragSource.prepend(versionDefinition);
    }

    if (!m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource)) {
        qDebug() << "Could not add vertex code";
        return;
    }

    if (!m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource)) {
        qDebug() << "Could not add fragment code";
        return;
    }

    if (!m_shader.link()) {
        qDebug() << "Could not link";
        return;
    }

    if (!m_shader.bind()) {
        qDebug() << "Could not bind";
        return;
    }

    m_shader.release();


    m_vao.create();
    m_vao.bind();

    m_verticesBuffer.create();
    updateVerticesBuffer(this->rect());

    QVector<QVector2D> textureVertices(6);
    rectToTexCoords(textureVertices.data(), QRect(0.0, 0.0, 1.0, 1.0));

    m_textureVerticesBuffer.create();
    m_textureVerticesBuffer.bind();
    m_textureVerticesBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_textureVerticesBuffer.allocate(2 * 3 * sizeof(QVector2D));
    m_verticesBuffer.write(0, textureVertices.data(), m_textureVerticesBuffer.size());
    m_textureVerticesBuffer.release();

    m_vao.release();


    if (!m_sourceImage.isNull()) {
        loadImage(m_sourceImage);
    }
}

void KisGLImageWidget::updateVerticesBuffer(const QRect &rect)
{
    if (!m_vao.isCreated() || !m_verticesBuffer.isCreated()) return;

    QVector<QVector3D> vertices(6);
    rectToVertices(vertices.data(), rect);

    m_verticesBuffer.bind();
    m_verticesBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_verticesBuffer.allocate(2 * 3 * sizeof(QVector3D));
    m_verticesBuffer.write(0, vertices.data(), m_verticesBuffer.size());
    m_verticesBuffer.release();
}


void KisGLImageWidget::paintGL()
{
    const QColor bgColor = palette().background().color();
    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_texture.isCreated()) return;

    glViewport(0, 0, width(), height());

    m_vao.bind();
    m_shader.bind();

    {
        QMatrix4x4 projectionMatrix;
        projectionMatrix.setToIdentity();
        projectionMatrix.ortho(0, width(), height(), 0, -1, 1);
        QMatrix4x4 viewProjectionMatrix;

        // use a QTransform to scale, translate, rotate your view
        QTransform transform; // TODO: noop!
        viewProjectionMatrix = projectionMatrix * QMatrix4x4(transform);

        m_shader.setUniformValue("viewProjectionMatrix", viewProjectionMatrix);
    }

    m_shader.enableAttributeArray("vertexPosition");
    m_verticesBuffer.bind();
    m_shader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    m_shader.enableAttributeArray("texturePosition");
    m_textureVerticesBuffer.bind();
    m_shader.setAttributeBuffer("texturePosition", GL_FLOAT, 0, 2);

    glActiveTexture(GL_TEXTURE0);
    m_texture.bind();

    // draw 2 triangles = 6 vertices starting at offset 0 in the buffer
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_verticesBuffer.release();
    m_textureVerticesBuffer.release();
    m_texture.release();
    m_shader.release();
    m_vao.release();
}

void KisGLImageWidget::loadImage(const KisGLImageF16 &image)
{
    if (m_sourceImage != image) {
        m_sourceImage = image;
    }

    if (m_vao.isCreated()) {
        m_texture.setFormat(QOpenGLTexture::RGBA16F);
        m_texture.setSize(image.width(), image.height());
        m_texture.allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::Float16);
        m_texture.setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        m_texture.setMagnificationFilter(QOpenGLTexture::Linear);
        m_texture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float16, image.constData());
        updateGeometry();
    }
}

void KisGLImageWidget::paintEvent(QPaintEvent *event)
{
    QOpenGLWidget::paintEvent(event);
}

void KisGLImageWidget::resizeEvent(QResizeEvent *event)
{
    updateVerticesBuffer(QRect(QPoint(),event->size()));
    QOpenGLWidget::resizeEvent(event);
}

QSize KisGLImageWidget::sizeHint() const
{
    return m_sourceImage.size();
}

