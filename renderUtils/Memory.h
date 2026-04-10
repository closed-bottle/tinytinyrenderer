
#ifndef TINYTINYRENDERER_MEMORY_H
#define TINYTINYRENDERER_MEMORY_H

#include <cstdint>
#include <vector>

class Memory {
    std::vector<uint8_t> bytes;
public:
    Memory() = delete;

    explicit Memory(const size_t _size) {
        bytes.reserve(_size);
    };

    size_t Size() const {return bytes.size();}
    uint8_t* Data() {return bytes.data();}
};


#endif //TINYTINYRENDERER_MEMORY_H