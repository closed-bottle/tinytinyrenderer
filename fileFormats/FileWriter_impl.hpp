#pragma once

#include <cstring>
#include <fstream>
#include "FileWriter.h"

namespace {
    namespace TGA {
        // header :
        // ID Length 1 byte
        // Color Map Type 1 byte
        // Image Type 1 byte
        // Color Map Specification 5 bytes
        // - color map origin 2 bytes
        // - color map length 2 bytes
        // Color map deph 1 byte
        // Image Specification 10 bytes
        // - x origin 2 bytes
        // - y origin 2 bytes
        // - width 2 bytes
        // - height 2 bytes
        // - bits per pixel 1 byte
        // - image descriptor 1 byte
        struct Header
        {
            struct ColorMapSpec
            {
                short first_entry_index_ = 0;
                short colorMap_length_ = 0;
                char colorMap_entry_size_ = 0;
            };

            struct ImageSpec
            {
                short x_origin_ = 0;
                short y_origin_ = 0;
                short width_ = 0;
                short height_ = 0;
                char bits_per_pixel_ = 0;
                char image_origin_ = 0;
            };
            char id_length_ = 0;
            char colorMap_type_ = 0;
            char image_type_ = 0;
            ColorMapSpec colorMap_spec_;
            ImageSpec image_spec_;
        };

        //https://en.wikipedia.org/wiki/Run-length_encoding
        bool CompressRLE(std::ofstream& _input_file);

        bool IsInsideBoundary(unsigned int _x_coord, unsigned int _y_coord);
        template<typename T>
        bool IsInsideBoundary(Vector2D<T> _input)
        {
            return IsInsideBoundary(static_cast<unsigned int>(_input.x), static_cast<unsigned int>(_input.y));
        }

        enum Format
        {
            GRAYSCALE = 1, RGB = 3, RGBA = 4
        };

        // Image type
        // 0   No Image Data Included
        // 1   Uncompressed, Color - mapped Image
        // 2   Uncompressed, True - color Image
        // 3   Uncompressed, Black - and-white Image
        // 9   Run - length encoded, Color - mapped Image
        // 10  Run - length encoded, True - color Image
        // 11  Run - length encoded, Black - and-white Image
        enum Type
        {
            NO_IMAGE = 0,  UNCOMP_COLOR_MAPPED = 1, UNCOMP_TRUE_COLOR = 2, UNCOMP_BLACK_WHITE = 3,
            RLE_COLOR_MAPPED = 9, RLE_TRUE_COLOR = 10, RLE_BLACK_WHITE = 11,
        };

        // Image origin
        // Screen destination	: bit 5 : bit 4
        // of first pixel	    :       :
        //	bottom left			:      0:     0
        //	bottom right		:      0:     1
        //	top left			:      1:     0
        //	top right			:      1:     1
        enum Origin
        {
            RIGHT = 0b10000, TOP = 0b100000
        };

    	template<PixelFormat PF>
		bool SaveToFile(const std::string& _filename, bool is_compress, const Image<PF>& _image)
		{
			uint8_t developer_area[] = { 0, 0, 0, 0 }; // Let dev area empty.
			uint8_t extension_area[] = { 0, 0, 0, 0 }; // Let ext area empty.
			uint8_t signature_string[] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };

			std::ofstream output_file(_filename, std::ios_base::binary);

			if (!output_file.is_open())
			{
				std::cerr << "Can't save file : " << _filename.c_str() << ".\n";
				output_file.close();
				return false;
			}
			Header header;
			memset(&header, 0, sizeof(header));
			header.image_spec_.bits_per_pixel_ = _image.Stride() << 3;
			header.image_spec_.width_ =  _image.Width();
			header.image_spec_.height_ = _image.Height();
			header.image_type_ = (static_cast<int>(_image.Stride()) == GRAYSCALE ?
								 (is_compress ? RLE_BLACK_WHITE : UNCOMP_BLACK_WHITE)
							   : (is_compress ? RLE_TRUE_COLOR : UNCOMP_TRUE_COLOR));
			header.image_spec_.image_origin_ = 0b100000; // top-left origin

			output_file.write(reinterpret_cast<char *>(&header), sizeof(header));
			if (!output_file.good())
			{
				output_file.close();
				std::cerr << "Can't save tga file header.\n";
				return false;
			}

			if (!is_compress)
			{
				auto color_size = sizeof(PF); // default = 32bits

				if (header.image_spec_.bits_per_pixel_ == 24)
					color_size -= 1;

				//unsigned int count = 0;
				//unsigned int maximum = static_cast<unsigned int>(data_.size());
				for (uint32_t i = 0; i < _image.NPixels(); i += color_size)
				{
					auto* itr = reinterpret_cast<ImageFormat<PixelFormat::R8G8B8>*>(&_image.Data()[i]);
					// TGA pixel color is in form of : BGRA.
					// So swap RGBA to BGRA
					std::swap(itr->R, itr->B);

					//std::cout << "Saving " << count++ << " of " << maximum << "datas." << std::endl;

					output_file.write(reinterpret_cast<char*>(&itr), color_size);
					if (!output_file.good())
					{
						std::cerr << "Can't save raw data\n";
						output_file.close();
						return false;
					}
				}
			}
			//else
			//	CompressRLE(output_file, _image);


			output_file.write((char *)developer_area, sizeof(developer_area));
			if (!output_file.good())
			{
				std::cerr << "Can't save developer area to tga file\n";
				output_file.close();
				return false;
			}
			output_file.write((char *)extension_area, sizeof(extension_area));
			if (!output_file.good())
			{
				std::cerr << "Can't save extension area to tga file\n";
				output_file.close();
				return false;
			}
			output_file.write((char *)signature_string, sizeof(signature_string));
			if (!output_file.good())
			{
				std::cerr << "Can't save signature string to tga file\n";
				output_file.close();
				return false;
			}
			output_file.close();
			return true;
		}
    }
}

template<FileWriter::FFormat FF, PixelFormat PF>
void FileWriter::WriteImageToFile(std::string _path, const Image<PF>& _image) {
    if (FF ==FFormat::TGACompressed) {
		TGA::SaveToFile(_path, true, _image);
    }
}