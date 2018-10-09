#include "glwidget.h"

#include <QFile>
#include <QDebug>

#include "ExrReader.h"

#include <QVector2D>
#include <QVector3D>
#include <QVector>

QString __methodName(const char *_prettyFunction)
{
    std::string prettyFunction(_prettyFunction);

    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return QString(std::string(prettyFunction.substr(begin,end) + "()").c_str());
}

/**
 * Please pretty print my variable
 *
 * Use this macro to display in the output stream the name of a variable followed by its value.
 */
#define ppVar( var ) #var << "=" << (var)

#ifdef __GNUC__
QString __methodName(const char *prettyFunction);
#define __METHOD_NAME__ __methodName(__PRETTY_FUNCTION__)
#else
#define __METHOD_NAME__ "<unknown>:<unknown>"
#endif

#define PREPEND_METHOD(msg) QString("%1: %2").arg(__METHOD_NAME__).arg(msg)

#ifdef __GNUC__
#define ENTER_FUNCTION() qDebug() << "Entering" << __METHOD_NAME__
#define LEAVE_FUNCTION() qDebug() << "Leaving " << __METHOD_NAME__
#else
#define ENTER_FUNCTION() qDebug() << "Entering" << "<unknown>"
#define LEAVE_FUNCTION() qDebug() << "Leaving " << "<unknown>"
#endif


#include <QRectF>
#include <QVector3D>

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


const QString texture_frag =
        "#version 330 core\n"
        "// vertices datas\n"
        "in vec4 textureCoordinates;\n"
        "// uniforms\n"
        "uniform sampler2D f_tileTexture; // tile texture\n"
        "uniform float f_opacity = 1; // tile opacity\n"
        "out vec4 f_fragColor; // shader output color\n"
        "void main()\n"
        "{\n"
        "    // get the fragment color from the tile texture\n"
        "    vec4 color = texture(f_tileTexture, textureCoordinates.st);\n"
        "    // premultiplied output color\n"
        "    f_fragColor = vec4(color * f_opacity);\n"
        "    //f_fragColor = vec4(1.0, 0, 0, 1.0);\n"
        "}\n";


const QString texture_vert =
        "#version 330 core\n"
        "uniform mat4 viewProjectionMatrix;\n"
        "uniform mat4 textureMatrix;\n"
        "in vec3 vertexPosition;\n"
        "in vec2 texturePosition;\n"
        "out vec4 textureCoordinates;\n"
        "void main()\n"
        "{\n"
        "   textureCoordinates = textureMatrix * vec4(texturePosition.x, texturePosition.y, 0, 1);\n"
        "   gl_Position = viewProjectionMatrix * vec4(vertexPosition.x, vertexPosition.y, 0, 1);\n"
        "}\n";


GLWidget::GLWidget(const QString &fname, QWidget *parent)
    : QOpenGLWidget(parent)
    , m_defaultFile(fname)
{
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    setTextureFormat(GL_RGBA16F);
    setAutoFillBackground(false);
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

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(teardownGL()), Qt::DirectConnection);
    printVersionInformation();
    glClearColor(0.7f, 0.5f, 0.5f, 1.0f);

    m_vao.create();
    m_vao.bind();


    m_program = new QOpenGLShaderProgram();
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, texture_vert.toLatin1())) {
        qDebug() << "Could not add vertex code";
        return;
    }

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, texture_frag.toLatin1())) {
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
    if (reader.read(&m_image)) {

        m_image.save("bla.png");

        QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        qDebug() << "start format";
        texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        qDebug() << "end format";
        qDebug() << ppVar(m_image.size());
        texture->setSize(m_image.width(), m_image.height());
        texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, m_image.bits());
        texture->generateMipMaps();
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
