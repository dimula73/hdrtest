#include "window.h"

#include "glwidget.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QDebug>

Window::Window(const QString &fname)
{
    setWindowTitle(tr("16 bit float QOpenGLWidget test"));
    QMenu *menu = menuBar()->addMenu("File");
    QToolBar *tb = addToolBar("File");

    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
    menu->addAction(m_quitAction);
    tb->addAction(m_quitAction);

    m_openAction = new QAction("Open", this);
    connect(m_openAction, SIGNAL(triggered(bool)), this, SLOT(fileOpen()));
    menu->addAction(m_openAction);
    tb->addAction(m_openAction);

    m_glWidget = new GLWidget(fname, this);
    setCentralWidget(m_glWidget);

}

void Window::fileOpen()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open EXR file", "", "*.exr");
    m_glWidget->loadImage(filename);
}
