#include "glwidget.h"

#include <QFile>
#include <QDebug>

#include "ExrReader.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QRectF>

#include "kis_debug.h"
#include <config-opengles.h>

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

GLWidget::GLWidget(const QString &fname, QWidget *parent)
    : QOpenGLWidget(parent)
    , m_defaultFile(fname)
{
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);

    qDebug() << "*** Creating gl widget";

#if !USE_OPENGLES
    setTextureFormat(GL_RGBA16F);
#else
    setTextureFormat(GL_RGBA16F);
    //setTextureFormat(GL_RGBA8); // doesn't work for OpenGL ES!!!
#endif
    setAutoFillBackground(false);

    //setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

GLWidget::~GLWidget()
{
    delete m_program;
}

QSize GLWidget::sizeHint() const {
    return QSize(m_width, m_height);
}

void GLWidget::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);
}

#include "EGL/egl.h"
#include "EGL/eglext.h"


typedef const char *(EGLAPIENTRYP PFNEGLQUERYSTRINGPROC) (EGLDisplay dpy, EGLint name);

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(teardownGL()), Qt::DirectConnection);
    printVersionInformation();
    glClearColor(0.7f, 0.5f, 0.5f, 1.0f);

    QOpenGLContext *context = this->context();
    QOpenGLFunctions *funcs = context->functions();
    QString rendererString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER)));
    QString driverVersionString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));

    {
        PFNEGLQUERYSTRINGPROC queryString = nullptr;
        queryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(
            context->getProcAddress("eglQueryString"));

        const char* client_extensions = queryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        const QList<QByteArray> extensions = QByteArray(client_extensions).split(' ');

        qDebug () << "Available EGL extensions";
        Q_FOREACH(const QString &ext, extensions) {
            qDebug() << ppVar(ext);
        }

        qDebug() << "Available OpenGL ES extensions";
        Q_FOREACH(const QString &ext, context->extensions()) {
            qDebug() << ppVar(ext);
        }

        if (extensions.contains("EGL_ANGLE_platform_angle_d3d")) {
            PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttribEXT = nullptr;

            {
                queryDisplayAttribEXT = reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(
                            context->getProcAddress("eglQueryDisplayAttribEXT"));

                ENTER_FUNCTION() << ppVar(queryDisplayAttribEXT);
            }

            PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttribEXT = nullptr;

            {
                queryDeviceAttribEXT = reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(
                            context->getProcAddress("eglQueryDeviceAttribEXT"));

                ENTER_FUNCTION() << ppVar(queryDeviceAttribEXT);
            }
        }
    }



    qDebug() << ppVar(rendererString);
    qDebug() << ppVar(driverVersionString);

    m_vao.create();
    m_vao.bind();


    QFile vertexShaderFile(QString(":/") + "simple_texture.vert");
    vertexShaderFile.open(QIODevice::ReadOnly);
    QString vertSource = vertexShaderFile.readAll();

    QFile fragShaderFile(QString(":/") + "simple_texture.frag");
    fragShaderFile.open(QIODevice::ReadOnly);
    QString fragSource = fragShaderFile.readAll();

    ENTER_FUNCTION() << ppVar(context->isOpenGLES());

    if (context->isOpenGLES()) {
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




    m_program = new QOpenGLShaderProgram();
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource)) {
        qDebug() << "Could not add vertex code";
        return;
    }

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource)) {
        qDebug() << "Could not add fragment code";
        return;
    }

    if (!m_program->link()) {
        qDebug() << "Could not link";
        return;
    }

    if (!m_program->bind()) {
        qDebug() << "Could not bind";
        return;
    }

    QVector<QVector3D> vertices(6);
    QVector<QVector2D> textureVertices(6);
    rectToVertices(vertices.data(), QRect(0,0,600,800));
    rectToTexCoords(textureVertices.data(), QRect(0,0,1.0,1.0));

    m_verticesBuffer.create();
    m_verticesBuffer.bind();
    m_verticesBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_verticesBuffer.allocate(2 * 3 * sizeof(QVector3D));
    m_verticesBuffer.write(0, vertices.data(), m_verticesBuffer.size());
    m_verticesBuffer.release();

    m_textureVerticesBuffer.create();
    m_textureVerticesBuffer.bind();
    m_textureVerticesBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_textureVerticesBuffer.allocate(2 * 3 * sizeof(QVector2D));
    m_verticesBuffer.write(0, textureVertices.data(), m_textureVerticesBuffer.size());
    m_textureVerticesBuffer.release();


    m_program->release();

    m_vao.release();

    loadImage(m_defaultFile);
}

void GLWidget::paintGL()
{
    ENTER_FUNCTION();

    m_vao.bind();

    // viewport and matrices setup for a 2D tile system
    glViewport(0, 0, width(), height());

    // clear
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_tiles.isEmpty()) {

        QMatrix4x4 projectionMatrix;
        projectionMatrix.setToIdentity();
        projectionMatrix.ortho(0, width(), height(), 0, -1, 1);
        QMatrix4x4 viewProjectionMatrix;

        // use a QTransform to scale, translate, rotate your view
        viewProjectionMatrix = projectionMatrix * QMatrix4x4(m_transform);

        m_program->bind();

        m_program->setUniformValue("viewProjectionMatrix", viewProjectionMatrix);
        m_program->setUniformValue("textureMatrix", QMatrix4x4());
        m_program->setUniformValue("f_opacity", 1.0f);

        QOpenGLTexture* tile = m_tiles.at(0);

        // activate texture unit 0
        glActiveTexture(GL_TEXTURE0);

        // setup texture options here if needed...
        // set sampler2D on texture unit 0
        m_program->setUniformValue("f_tileTexture", 0);

        m_program->enableAttributeArray("vertexPosition");
        m_verticesBuffer.bind();
        m_program->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

        m_program->enableAttributeArray("texturePosition");
        m_textureVerticesBuffer.bind();
        m_program->setAttributeBuffer("texturePosition", GL_FLOAT, 0, 2);

        tile->bind();

        // draw 2 triangles = 6 vertices starting at offset 0 in the buffer
        glDrawArrays(GL_TRIANGLES, 0, 6);

        tile->release();

        m_program->release();
    }

    m_vao.release();
}

void GLWidget::teardownGL()
{
    ENTER_FUNCTION();

    makeCurrent();
    // Actually destroy our OpenGL information
    foreach(QOpenGLTexture* tile, m_tiles) {
        if (tile->isCreated()) {
            tile->destroy();
        }
    }

    m_verticesBuffer.destroy();
}

void GLWidget::loadImage(const QString &fname)
{
    QFile f(fname);
    if (!f.open(QFile::ReadOnly)) return;

    qDeleteAll(m_tiles);
    m_tiles.clear();

    ExrReader reader(&f);
    if (reader.read(&m_image, &m_hdrImage)) {

        m_image.save("bla.png");

        const bool useHalfFloatTexture = this->format().redBufferSize() > 8;

        ENTER_FUNCTION() << ppVar(useHalfFloatTexture);

        //QOpenGLTexture::TextureFormat textureFormat = QOpenGLTexture::SRGB8_Alpha8;
        QOpenGLTexture::TextureFormat textureFormat = QOpenGLTexture::RGBA8_UNorm;
        QOpenGLTexture::PixelFormat pixelFormat = QOpenGLTexture::RGBA;
        QOpenGLTexture::PixelType pixelType = QOpenGLTexture::UInt8;

        if (useHalfFloatTexture) {
            textureFormat = QOpenGLTexture::RGBA16F;
            pixelFormat = QOpenGLTexture::RGBA;
            pixelType = QOpenGLTexture::Float16;
        }

        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        qDebug() << "start format" << ppVar(m_hdrImage.size());
        texture->setFormat(textureFormat);
        texture->setSize(m_image.width(), m_image.height());
        texture->allocateStorage(pixelFormat, pixelType);
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);

        // we expect the mipmaps to be generated automatically!
        assert(texture->isAutoMipMapGenerationEnabled());

        if (!useHalfFloatTexture) {
            const QImage rgbaImage = m_image.convertToFormat(QImage::Format_RGBA8888);
            texture->setData(pixelFormat, pixelType, rgbaImage.constBits());
        } else {
            texture->setData(pixelFormat, pixelType, &m_hdrImage.data()[0][0]);
        }

        m_tiles << texture;

        m_width = m_image.width();
        m_height = m_image.height();
        setFixedSize(m_image.size());

    }

    update();
}


void GLWidget::printVersionInformation()
{
    QString glType;
    QString glVersion;
    QString glProfile;

    // Get Version Information
    glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
    switch (format().profile())
    {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
    }
#undef CASE

    // qPrintable() will print our QString w/o quotes around it.
    qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
    qDebug() << ppVar(format().redBufferSize()) << ppVar(format().options());

}
