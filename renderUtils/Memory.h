
#ifndef TINYTINYRENDERER_MEMORY_H
#define TINYTINYRENDERER_MEMORY_H

#include <cstdint>
#include "special-lamp/lampVector.h++"

class Memory {
    Lamp::Vector<uint8_t> bytes_;
public:
    Memory() = delete;

    explicit Memory(const size_t _size) {
        bytes_.reserve(_size);
    };

    size_t Size() const {return bytes_.size();}
    uint8_t* Data() {return bytes_.data();}
    const uint8_t* Data() const {return bytes_.data();}
};


#endif //TINYTINYRENDERER_MEMORY_H