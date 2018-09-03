#include "glwidget.h"
#include <QDebug>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
//    setAttribute(Qt::WA_NoSystemBackground, true);
//    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    setTextureFormat(GL_RGBA16F);
    setFixedSize(644, 874);
    setAutoFillBackground(false);
}

void GLWidget::resizeGL(int width, int height)
{
    paintGL();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
}


