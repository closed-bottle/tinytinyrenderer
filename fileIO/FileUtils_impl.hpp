#include "FileUtils.h"

void FileUtils::PushBytes(Lamp::Vector<uint8_t>& _target, uint8_t _stride, const uint8_t* _data) {
    const auto buff = _data;
    for (size_t j = 0; j < _stride; ++j)
        _target.push_back(buff[j]);
}

namespace {
    inline uint8_t* Index(const Image& _image, uint8_t* _ptr, size_t _begin, size_t _index) {
        return _ptr + _begin + (_index * _image.Stride());
    }
}

// Runs simple Intermediate Run Length Encoding
// header :
// 0 - 0 + Literal
// anything else - Run Length + Literal
template<size_t RunLength>
Lamp::Vector<uint8_t> FileUtils::RLE(const Image& _image) {
    Lamp::Vector<uint8_t> result;
    size_t begin = 0;
    const auto& data = static_cast<uint8_t*>(_image.Data());
    auto n = _image.NPixels();
    uint8_t header = 0;

    while (n) {
        size_t offset = 1;

        auto while_same = [&]() {
            while (offset < n && offset < RunLength) {
                if (memcmp(Index(_image, data, begin, offset - 1),
                    Index(_image, data, begin, offset), _image.Stride()) != 0)
                    break;
                ++offset;
            }
        };


        auto while_diff = [&]() {
            while (offset < n && offset < RunLength) {
                if (memcmp(Index(_image, data, begin, offset - 1),
                    Index(_image, data, begin, offset), _image.Stride()) == 0)
                    break;
                ++offset;
            }
        };


        while_same();
        if (offset <= 1) {
            while_diff();

            header = 0;
            for (size_t i = 0; i < offset; ++i) {
                result.push_back(header);
                PushBytes(result, _image.Stride(), reinterpret_cast<uint8_t *>(&data[begin + i * _image.Stride()]));

                --n;
            }
        }
        else {
            n -= offset;
            header = offset;

            result.push_back(header);
            PushBytes(result, _image.Stride(), reinterpret_cast<uint8_t *>(&data[begin]));
        }

        begin += offset * _image.Stride();
    }

    return result;
}