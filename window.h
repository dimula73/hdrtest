#ifndef WINDOW_H
#define WINDOW_H

#include "helper.h"

#include <QMainWindow>

class QAction;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(QIODevice &io);

private:
    QAction *m_quitAction {0};
};

#endif
