#ifndef HDRIMAGEF16_H
#define HDRIMAGEF16_H

#include <openexr/half.h>
#include <QSize>


#include <ImfArray.h>
#include <ImfRgba.h>


class HdrImageF16
{
public:
    HdrImageF16();
    ~HdrImageF16();

    void setData(const Imf::Array2D<Imf::Rgba> &data);
    QSize size() const;
    const Imf::Array2D<Imf::Rgba> & data() const;

private:
    QSize m_size;
    Imf::Array2D<Imf::Rgba> m_data;
};

#endif // HDRIMAGEF16_H
