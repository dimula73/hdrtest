#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent);

protected:
    void resizeGL(int width, int height) override;
    void initializeGL() override;
    void paintGL() override;

private:

};


#endif
