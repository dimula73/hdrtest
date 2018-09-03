#ifndef EXRREADER_H
#define EXRREADER_H

#include <ImfRgbaFile.h>
#include <ImfStandardAttributes.h>
#include <ImathBox.h>
#include <ImfInputFile.h>
#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfArray.h>
#include <ImfConvert.h>
#include <ImfVersion.h>
#include <IexThrowErrnoExc.h>


class QIODevice;
class QImage;

class ExrReader
{
public:
    ExrReader(QIODevice *io);
    bool read(QImage *outImage);
    Imf::Array2D<Imf::Rgba> &pixels() { return m_pixels; }
private:
    QIODevice *m_io {0};
    Imf::Array2D<Imf::Rgba> m_pixels;
};

#endif // EXRREADER_H
