#include "window.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>

int main(int argc, char *argv[])
{
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



    QApplication app(argc, argv);
    // OpenGLES isn't possible, and on Windows with Intel we have to use ES, because Angle
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);


    QString fname = "./Desk.exr";
    QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        fname = app.arguments().at(1);
    }

    Window window(fname);
    window.show();
    return app.exec();
}
