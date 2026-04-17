#pragma once

#include <cstring>
#include <fstream>
#include "FileUtils.h"

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
#pragma pack(push, 1)
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
#pragma pop
            uint8_t id_length_ = 0;
            uint8_t colorMap_type_ = 0;
            uint8_t image_type_ = 0;
            ColorMapSpec colorMap_spec_;
            ImageSpec image_spec_;
        };

        //https://en.wikipedia.org/wiki/Run-length_encoding
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

    		// TODO: Noticed I need to handle endianness.
    		//       Add byteswap if it is bigendian(TGA stores everything in LE).

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

				for (uint32_t i = 0; i < _image.NPixels(); i += color_size)
				{
					auto* itr = reinterpret_cast<Texel<PixelFormat::R8G8B8>*>(&_image.Data()[i]);
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
			else {
				Lamp::Vector<uint8_t> bytes;
				bytes = FileUtils::RLE<128, PF>(_image);

				//for (auto& b : bytes)
				//	cout << (int)b << '\n';
				//cout << endl;

				Lamp::Vector<uint8_t> tga;


				uint64_t i = 0;
				while (i < bytes.size()) {
					auto header = bytes[i];
					tga.push_back(header);
					auto header_index = tga.size() -1;

					if (header == 0) {
						uint64_t window = 0;
						while (i + window < bytes.size() && tga[header_index] < 128) {
							header = bytes[i + window];
							if (header != 0)
								break;
							++window;
							FileUtils::PushBytes(tga, _image.Stride(), &bytes[i + window]);
							Texel<PixelFormat::B8G8R8> debg = *reinterpret_cast<Texel<PixelFormat::B8G8R8>*>(&bytes[i + window]);

							//cout << (int)debg.B << " | " << (int)debg.G << " | " << (int)debg.R << endl;

							window += _image.Stride();
							++tga[header_index];
						}

						tga[header_index] -= 1; // map range, 1->0, 128->127.
						i += window;
					}
					else {
						tga[header_index] += 128 -1; // map range.
						++i;
						FileUtils::PushBytes(tga, _image.Stride(), &bytes[i]);
						i += _image.Stride();
					}
				}


				output_file.write((char*)tga.data(), tga.size());
			}

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

template<FFormat FF, PixelFormat PF>
void FileWriter::WriteImageToFile(std::string _path, const Image<PF>& _image) {
    if (FF ==FFormat::TGACompressed) {
		TGA::SaveToFile(_path, true, _image);
    }
}