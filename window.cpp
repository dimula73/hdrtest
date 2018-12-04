#include "window.h"

#include "glwidget.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QDebug>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include <QSurface>
#include <QWindow>

#include "kis_debug.h"


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


    QWidget *centralWidget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    QHBoxLayout *hLayout = new QHBoxLayout(centralWidget);

    QLabel *label = new QLabel(centralWidget);
    QImage icon(":/64-apps-calligrakrita.png");
    label->setPixmap(QPixmap::fromImage(icon));
    hLayout->addWidget(label, 0, Qt::AlignLeft);

    QPushButton *button = new QPushButton("Quit", centralWidget);
    connect(button, SIGNAL(clicked()), this, SLOT(slotDoSomething()));
    hLayout->addWidget(button, 0, Qt::AlignCenter);

    layout->addLayout(hLayout);

    m_glWidget = new GLWidget(fname, centralWidget);
    layout->addWidget(m_glWidget, 1);


    centralWidget->setLayout(layout);

    setCentralWidget(centralWidget);
}

void Window::fileOpen()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open EXR file", "", "*.exr");
    m_glWidget->loadImage(filename);
}

void Window::slotDoSomething()
{
    close();
}
