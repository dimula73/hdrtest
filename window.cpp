#include "glwidget.h"
#include "window.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>

Window::Window(QIODevice &io)
{
    setWindowTitle(tr("16 bit float QOpenGLWidget test"));
    QMenu *menu = menuBar()->addMenu("File");
    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
    menu->addAction(m_quitAction);
    QToolBar *tb = addToolBar("File");
    tb->addAction(m_quitAction);

    GLWidget *openGL = new GLWidget(this);
    setCentralWidget(openGL);
}
