
#ifndef TINYTINYRENDERER_MEMORY_H
#define TINYTINYRENDERER_MEMORY_H

#include <cstdint>
#include "special-lamp/lampVector.h++"

class Memory {
    Lamp::Vector<uint8_t> bytes_;
    size_t offset_ = 0;
    size_t align_ = 0;
    uint8_t* data_ = nullptr;
public:
    Memory() = delete;

    explicit Memory(const size_t _size, const size_t _align = 0) {
        bytes_.reserve(_size + _align);
        align_ = _align;
        offset_ = _align - (reinterpret_cast<uint64_t>(bytes_.data()) % _align);
        data_ = bytes_.data() + offset_;
    };

    size_t Size() const {return bytes_.size();}
    uint8_t* Data() {return data_;}
    const uint8_t* Data() const {return data_;}
};


#endif //TINYTINYRENDERER_MEMORY_H