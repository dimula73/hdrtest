#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

class QAction;

class GLWidget;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(const QString &fname);

public Q_SLOTS:

    void fileOpen();
    void slotDoSomething();

private:
    GLWidget *m_glWidget {0};
    QAction *m_openAction {0};
    QAction *m_quitAction {0};
};

#endif
