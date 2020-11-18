#ifndef SPECTRUMDATA_H
#define SPECTRUMDATA_H

#include <cstdint>
#include <vector>

struct SpectrumData
{
    SpectrumData( uint64_t apos, const std::vector<float>&& aspectrum )
        : position( apos )
        , spectrum( std::move(aspectrum) )
    {}
    SpectrumData() = default;
    uint64_t position = 0;
    std::vector<float> spectrum;
};


#endif // SPECTRUMDATA_H
