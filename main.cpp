#include "window.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <config-opengles.h>


int main(int argc, char *argv[])
{
    //qputenv("QT_OPENGL", "angle");

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
