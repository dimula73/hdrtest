#include "HdrImageF16.h"

HdrImageF16::HdrImageF16()
{
}

HdrImageF16::~HdrImageF16()
{
}

void HdrImageF16::setData(const Imf::Array2D<Imf::Rgba> &data)
{
    m_data.resizeErase(data.height(), data.width());

    for (int i = 0; i < data.height(); i++) {
        for (int j = 0; j < data.height(); j++) {
            m_data[i][j] = data[i][j];
        }
    }

    m_size = QSize(data.width(), data.height());
}

QSize HdrImageF16::size() const
{
    return m_size;
}

const Imf::Array2D<Imf::Rgba> &HdrImageF16::data() const
{
    return m_data;
}
