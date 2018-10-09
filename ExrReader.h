#ifndef EXRREADER_H
#define EXRREADER_H

#include <QObject>
#include <half.h>

class QIODevice;
class QImage;

class ExrReader
{
public:
    ExrReader(QIODevice *io);
    bool read(QImage *outImage);
    quint8 *pixels() { return reinterpret_cast<quint8*>(m_pixels); }
private:
    QIODevice *m_io {0};
    half *m_pixels {0};
};

#endif // EXRREADER_H
