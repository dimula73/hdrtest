#include "window.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QIODevice>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(3, 3);
    fmt.setRedBufferSize(16);;
    fmt.setGreenBufferSize(16);
    fmt.setBlueBufferSize(16);
    fmt.setAlphaBufferSize(16);

    QSurfaceFormat::setDefaultFormat(fmt);


    QString fname = "c:/dev/hdr/test/Desk.exr";
    QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        fname = app.arguments().at(1);
    }

    if (!QFileInfo(fname).exists()) {
        QMessageBox::warning(0, "Test", QString("%1 does not exist").arg(fname));
        return -1;
    }

    QFile io(fname);
    if (!io.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, "Test", QString("%1 cannot be opened").arg(fname));
        return -3;
    }

    Window window(io);
    window.show();
    return app.exec();
}
