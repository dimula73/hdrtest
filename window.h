#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

class QAction;

class GLWidget;
class KisGLImageWidget;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(const QString &fname);

public Q_SLOTS:

    void fileOpen();
    void slotDoSomething();
    void initializePalette();

private:
    GLWidget *m_glWidget {0};
    QAction *m_openAction {0};
    QAction *m_quitAction {0};
    KisGLImageWidget *m_imageWidget;
};

#endif
