#include "window.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <config-opengles.h>


#include "kis_debug.h"
#include <QWindow>
#include <QSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <boost/optional.hpp>

class KisOpenGLCheckResult {
public:
    KisOpenGLCheckResult(QOpenGLContext &context) {
        if (!context.isValid()) {
            return;
        }

        QOpenGLFunctions *funcs = context.functions(); // funcs is ready to be used

        m_rendererString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER)));
        m_driverVersionString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));
        m_glMajorVersion = context.format().majorVersion();
        m_glMinorVersion = context.format().minorVersion();
        m_supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);
        m_isOpenGLES = context.isOpenGLES();
    }

    int glMajorVersion() const {
        return m_glMajorVersion;
    }

    int glMinorVersion() const {
        return m_glMinorVersion;
    }

    bool supportsDeprecatedFunctions() const {
        return m_supportsDeprecatedFunctions;
    }

    bool isOpenGLES() const {
        return m_isOpenGLES;
    }

    QString rendererString() const {
        return m_rendererString;
    }

    QString driverVersionString() const {
        return m_driverVersionString;
    }

    bool isSupportedVersion() const {
        return
#ifdef Q_OS_OSX
                ((m_glMajorVersion * 100 + m_glMinorVersion) >= 302)
#else
                (m_glMajorVersion >= 3 && (m_supportsDeprecatedFunctions || m_isOpenGLES)) ||
                ((m_glMajorVersion * 100 + m_glMinorVersion) == 201)
#endif
                ;
    }

    bool supportsLoD() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 300;
    }

    bool hasOpenGL3() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 302;
    }

    bool supportsFenceSync() const {
        return m_glMajorVersion >= 3;
    }

#ifdef Q_OS_WIN
    // This is only for detecting whether ANGLE is being used.
    // For detecting generic OpenGL ES please check isOpenGLES
    bool isUsingAngle() const {
        return m_rendererString.startsWith("ANGLE", Qt::CaseInsensitive);
    }
#endif
private:
    int m_glMajorVersion = 0;
    int m_glMinorVersion = 0;
    bool m_supportsDeprecatedFunctions = false;
    bool m_isOpenGLES = false;
    QString m_rendererString;
    QString m_driverVersionString;
};

struct GLRenderableSetter
{
    GLRenderableSetter(Qt::ApplicationAttribute attribute, bool useOpenGLES)
        : m_attribute(attribute),
          m_oldUseOpenGLES(QCoreApplication::testAttribute(attribute))
    {
        QCoreApplication::setAttribute(attribute, useOpenGLES);
    }

    ~GLRenderableSetter() {
        QCoreApplication::setAttribute(m_attribute, m_oldUseOpenGLES);
    }

private:
    Qt::ApplicationAttribute m_attribute;
    bool m_oldUseOpenGLES = false;
};

boost::optional<KisOpenGLCheckResult> probeSurfaceFormat(const QSurfaceFormat &format)
{
    int argc = 1;
    QByteArray data("krita");
    char *argv = data.data();

    QApplication app(argc, &argv);
    GLRenderableSetter setter1(Qt::AA_UseOpenGLES, format.renderableType() == QSurfaceFormat::OpenGLES);
    GLRenderableSetter setter2(Qt::AA_UseDesktopOpenGL, format.renderableType() != QSurfaceFormat::OpenGLES);

    QWindow surface;
    surface.setSurfaceType(QSurface::OpenGLSurface);
    surface.create();
    QOpenGLContext context;
    if (!context.create()) {
        qDebug() << "OpenGL context cannot be created";
        return boost::none;
    }
    if (!context.isValid()) {
        qDebug() << "OpenGL context is not valid while checking Qt's OpenGL status";
        return boost::none;
    }
    if (!context.makeCurrent(&surface)) {
        qDebug() << "OpenGL context cannot be made current";
        return boost::none;
    }

    if (context.format().colorSpace() != format.colorSpace() &&
        format.colorSpace() != QSurfaceFormat::DefaultColorSpace) {
        qDebug() << "Failed to create an OpenGL context with requested color space. Requested:" << format.colorSpace() << "Actual:" << context.format().colorSpace();
        return boost::none;
    }

    return KisOpenGLCheckResult(context);
}



int main(int argc, char *argv[])
{
    //qputenv("QT_OPENGL", "angle");

    QVector<QSurfaceFormat::RenderableType> renderables({QSurfaceFormat::OpenGLES, QSurfaceFormat::OpenGL});
    QVector<QSurfaceFormat::ColorSpace> colorSpaces({QSurfaceFormat::DefaultColorSpace, QSurfaceFormat::sRGBColorSpace, QSurfaceFormat::scRGBColorSpace, QSurfaceFormat::bt2020PQColorSpace});
    QVector<int> bitDepths({8, 16, 10});

    Q_FOREACH (QSurfaceFormat::RenderableType renderable, renderables) {
        Q_FOREACH (QSurfaceFormat::ColorSpace colorSpace, colorSpaces) {
            Q_FOREACH (int bitDepth, bitDepths) {
                QSurfaceFormat fmt;
                fmt.setRenderableType(renderable);
                fmt.setProfile(QSurfaceFormat::CoreProfile);
                fmt.setOption(QSurfaceFormat::DebugContext);
                fmt.setVersion(3, 0);
                fmt.setRedBufferSize(bitDepth);
                fmt.setGreenBufferSize(bitDepth);
                fmt.setBlueBufferSize(bitDepth);
                fmt.setAlphaBufferSize(bitDepth == 10 ? 2 : bitDepth);
                fmt.setColorSpace(colorSpace);
                QSurfaceFormat::setDefaultFormat(fmt);

                boost::optional<KisOpenGLCheckResult> result =
                        probeSurfaceFormat(fmt);

                qDebug() << "SPT:" << bool(result) << bitDepth << renderable << colorSpace;
            }
        }
    }

#if !USE_OPENGLES
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setOption(QSurfaceFormat::DebugContext);
    fmt.setVersion(3, 2);
    fmt.setRedBufferSize(16);
    fmt.setGreenBufferSize(16);
    fmt.setBlueBufferSize(16);
    fmt.setAlphaBufferSize(16);
    QSurfaceFormat::setDefaultFormat(fmt);
#else
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setOption(QSurfaceFormat::DebugContext);
    fmt.setVersion(3, 0);
    fmt.setRedBufferSize(10);
    fmt.setGreenBufferSize(10);
    fmt.setBlueBufferSize(10);
    fmt.setAlphaBufferSize(10);
    fmt.setColorSpace(QSurfaceFormat::bt2020PQColorSpace);
    QSurfaceFormat::setDefaultFormat(fmt);
#endif

    QApplication app(argc, argv);

#if !USE_OPENGLES
    // OpenGLES isn't possible, and on Windows with Intel we have to use ES, because Angle
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
#else
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
#endif


    QString fname = "./Desk.exr";
    QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        fname = app.arguments().at(1);
    }

    Window window(fname);
    window.show();
    return app.exec();
}
