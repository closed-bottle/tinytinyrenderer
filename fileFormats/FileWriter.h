#ifndef TINYTINYRENDERER_FILEWRITER_H
#define TINYTINYRENDERER_FILEWRITER_H

#include "../renderUtils/Image.h"
#include <string>


class FileWriter {
public:
    enum class FFormat {
        TGACompressed = 0,
        TGANonCompressed = 0,
        Count
    };

    template<FFormat FF, PixelFormat PF>
    static void WriteImageToFile(std::string _path, const Image<PF>& _image);
};

#include "FileWriter_impl.hpp"

#endif //TINYTINYRENDERER_FILEWRITER_H