#include <algorithm>
#include <utility>
#include <iostream>
#include <fstream>
#include <cassert>

#include "TGAImage.h"

#include <cstring>

void TGAImage::FlipVertically()
{
	std::reverse(std::begin(data_), std::end(data_));
	FlipHorizontally();
}

void TGAImage::FlipHorizontally()
{
	int nPixel = width_ * height_;
	for (int current_line = 0; current_line < nPixel; current_line += width_)
	{
		auto begin = std::begin(data_) + current_line;
		auto end   = std::begin(data_) + current_line + width_;
		std::reverse(begin, end);
	}
	
}

bool TGAImage::UncompressRLE(std::ifstream& _input_file)
{
	auto nPixels = width_ * height_;

	
	if (header_.image_spec_.bits_per_pixel_ == 32)
	{
		while (nPixels)
		{
			assert(nPixels > 0); // if number of remaining pixel is less than 0, something is wrong.
			unsigned char chunk_header = _input_file.get();
			

			if (chunk_header >= 128) // if 8th bit is exist.
			{
				chunk_header -= 127;

				Color pixel;

				_input_file.read((char*)&pixel.blue, sizeof(unsigned char));
				_input_file.read((char*)&pixel.green, sizeof(unsigned char));
				_input_file.read((char*)&pixel.red, sizeof(unsigned char));
				_input_file.read((char*)&pixel.alpha, sizeof(unsigned char));

				for (int i = 0; i < chunk_header; i++)
					data_.push_back(pixel);
			}
			else
			{
				chunk_header++;


				for (int i = 0; i < chunk_header; i++)
				{
					Color pixel;

					_input_file.read((char*)&pixel.blue, sizeof(unsigned char));
					_input_file.read((char*)&pixel.green, sizeof(unsigned char));
					_input_file.read((char*)&pixel.red, sizeof(unsigned char));
					_input_file.read((char*)&pixel.alpha, sizeof(unsigned char));

					data_.push_back(pixel);
				}
			}



			nPixels -= chunk_header;
		}

	}
	else if (header_.image_spec_.bits_per_pixel_ == 24)
	{
		while (nPixels)
		{
			assert(nPixels > 0); // if number of remaining pixel is less than 0, something is wrong.
			unsigned char chunk_header = _input_file.get();
			

			if (chunk_header >= 128) // if 8th bit is exist.
			{
				chunk_header -= 127;

				Color pixel;

				_input_file.read((char*)&pixel.blue, sizeof(unsigned char));
				_input_file.read((char*)&pixel.green, sizeof(unsigned char));
				_input_file.read((char*)&pixel.red, sizeof(unsigned char));
				pixel.alpha = 255;

				for (int i = 0; i < chunk_header; i++)
					data_.push_back(pixel);
			}
			else
			{
				chunk_header++;


				for (int i = 0; i < chunk_header; i++)
				{
					Color pixel;

					_input_file.read((char*)&pixel.blue, sizeof(unsigned char));
					_input_file.read((char*)&pixel.green, sizeof(unsigned char));
					_input_file.read((char*)&pixel.red, sizeof(unsigned char));
					pixel.alpha = 255;

					data_.push_back(pixel);
				}
			}
				


				
			nPixels -= chunk_header;
		}
	}


	return true;
}

bool TGAImage::CompressRLE(std::ofstream & _output_file)
{
	auto nPixels = width_ * height_;
	float maximum = static_cast<float>(nPixels);

	auto chunck_begin = std::begin(data_);
	auto color_size = sizeof(Color); // default = 32bits

	if (header_.image_spec_.bits_per_pixel_ == 24)
		color_size -= 1;

	while(nPixels)
	{
		unsigned char chunk_header = 0;
		auto chunck_end = std::end(data_);
		chunck_end = std::adjacent_find(chunck_begin, chunck_end, [](Color _i, Color _j) { return _i == _j; });

		int chunk_size = static_cast<int>(chunck_end - chunck_begin);
		if (!chunk_size)
		{
			chunck_end = std::end(data_);
			chunck_end = std::adjacent_find(chunck_begin, chunck_end, [](Color _i, Color _j) { return _i != _j; });
			chunk_size = static_cast<int>(chunck_end - chunck_begin);
			if (chunk_size > 128)
			{
				chunck_end = chunck_begin + 128; // Max length of chunk is 128.;
				chunk_size = static_cast<int>(chunck_end - chunck_begin);
			}

			nPixels -= chunk_size;
			chunk_header += chunk_size-1; //-1 To map 1~256 to 0~255 

			_output_file.put(chunk_header);
			std::for_each(chunck_begin, chunck_end, [&] (Color _input_color)
			{
				auto temp = _input_color.red;
				_input_color.red = _input_color.blue;
				_input_color.blue = temp;
				_output_file.write(reinterpret_cast<char*>(&_input_color), color_size);
			}
			);
		}
		else
		{
			if (chunk_size > 128)
				chunk_size = 128;

			nPixels -= chunk_size;

			chunk_header += 128 + chunk_size-1; //-1 To map 1~256 to 0~255 

			_output_file.put(chunk_header);


			Color color = *chunck_begin;
			std::swap(color.red, color.blue);

			_output_file.write(reinterpret_cast<char*>(&color), color_size);
		}


		chunck_begin = chunck_begin + chunk_size;
		//std::cout << (static_cast<float>(nPixels) / maximum) * 100.f << "% remaining." << std::endl;
	}
	

	return true;
}

TGAImage::TGAImage(std::string _filename) : filename_(_filename)
{
	LoadFromTGAFile(_filename);
}

TGAImage::TGAImage(int _width, int _height, int _byte_per_pixel)
	: width_(_width), height_(_height), byte_per_pixel(_byte_per_pixel)
{
	header_.image_type_ = _byte_per_pixel;
	header_.image_spec_.bits_per_pixel_ = _byte_per_pixel << 3;
	header_.image_spec_.height_ = height_;
	header_.image_spec_.width_ = width_;

	int nPixels = width_ * height_;
	for (int i = 0; i < nPixels; i++)
		data_.push_back(Color());
}

TGAImage::~TGAImage()
{
	data_.clear();
}

bool TGAImage::LoadFromTGAFile(std::string _filename)
{
	data_.clear();

	std::ifstream input_file(_filename, std::ios_base::binary);

	if (!input_file.is_open())
	{
		std::cerr << "Can't open file : " << _filename.c_str() << "\n";
		input_file.close();
		return false;
	}


	input_file.read((char *)&header_, sizeof(header_));
	if (!input_file.good()) 
	{
		std::cerr << "Can't read header from file : " << _filename.c_str() << ".\n";
		input_file.close();
		return false;
	}

	width_ = header_.image_spec_.width_;
	height_ = header_.image_spec_.height_;
	byte_per_pixel = header_.image_spec_.bits_per_pixel_ >> 3;

	if (width_ <= 0)
	{
		std::cerr << "Width is <= 0 from file : " << _filename.c_str() << ".\n";
		input_file.close();
		return false;
	}

	if (height_ <= 0)
	{
		std::cerr << "Height is <= 0 from file : " << _filename.c_str() << ".\n";
		input_file.close();
		return false;
	}

	if (height_ <= 0)
	{
		std::cerr << "Height is <= 0 from file : " << _filename.c_str() << ".\n";
		input_file.close();
		return false;
	}

	if (byte_per_pixel != TGA_Format::GRAYSCALE
	 && byte_per_pixel != TGA_Format::RGB
	 && byte_per_pixel != TGA_Format::RGBA)
	{
		std::cerr << "Undefined bit per pixel from file : " << _filename.c_str() << ".\n";
		input_file.close();
		return false;
	}

	unsigned long nPixels = width_ * height_;

	switch (header_.image_type_)
	{
		case static_cast<char>(TGA_Type::NO_IMAGE) :
			std::cout << "No image data included." << std::endl;
			break;
		case static_cast<char>(TGA_Type::UNCOMP_COLOR_MAPPED) :
			// TODO
			break;
		case static_cast<char>(TGA_Type::UNCOMP_TRUE_COLOR) :
		case static_cast<char>(TGA_Type::UNCOMP_BLACK_WHITE) :

			if (header_.image_spec_.bits_per_pixel_ == 32)
			{
				while (nPixels)
				{
					Color pixel;

					input_file.read((char*)&pixel.blue, sizeof(unsigned char));
					input_file.read((char*)&pixel.green, sizeof(unsigned char));
					input_file.read((char*)&pixel.red, sizeof(unsigned char));
					input_file.read((char*)&pixel.alpha, sizeof(unsigned char));

					data_.push_back(pixel);
					nPixels--;
				}
			}
			else if (header_.image_spec_.bits_per_pixel_ == 24)
			{
				while (nPixels)
				{
					Color pixel;

					input_file.read((char*)&pixel.blue, sizeof(unsigned char));
					input_file.read((char*)&pixel.green, sizeof(unsigned char));
					input_file.read((char*)&pixel.red, sizeof(unsigned char));
					pixel.alpha = 255;

					data_.push_back(pixel);
					nPixels--;
				}
			}
			
			

			break;
		case static_cast<char>(TGA_Type::RLE_COLOR_MAPPED) :
		// TODO
		break;
		case static_cast<char>(TGA_Type::RLE_TRUE_COLOR) :
		case static_cast<char>(TGA_Type::RLE_BLACK_WHITE) :
			UncompressRLE(input_file);
		break;
	}

	// Default data is already filped vertically, so we have to flip this when setting is bottom.
	if (!(static_cast<int>(header_.image_spec_.image_origin_) & TGA_Origin::TOP))
		FlipVertically();

		
	if ((static_cast<int>(header_.image_spec_.image_origin_) & TGA_Origin::RIGHT))
		FlipHorizontally();



	filename_ = _filename;

	input_file.close();
	return true;
}

bool TGAImage::SaveToTGAFile(std::string _filename, bool is_compress)
{
	unsigned char developer_area[] = { 0, 0, 0, 0 }; // Let dev area empty.
	unsigned char extension_area[] = { 0, 0, 0, 0 }; // Let ext area empty.
	unsigned char signature_string[] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };

	std::ofstream output_file(_filename, std::ios_base::binary);

	if (!output_file.is_open())
	{
		std::cerr << "Can't save file : " << _filename.c_str() << ".\n";
		output_file.close();
		return false;
	}
	TGA_Header header;
	memset(&header, 0, sizeof(header));
	header.image_spec_.bits_per_pixel_ = byte_per_pixel << 3;
	header.image_spec_.width_ = width_;
	header.image_spec_.height_ = height_;
	header.image_type_ = (static_cast<int>(byte_per_pixel) == TGA_Format::GRAYSCALE ?
						 (is_compress ? TGA_Type::RLE_BLACK_WHITE : TGA_Type::UNCOMP_BLACK_WHITE) 
					   : (is_compress ? TGA_Type::RLE_TRUE_COLOR : TGA_Type::UNCOMP_TRUE_COLOR));
	header.image_spec_.image_origin_ = 0b100000; // top-left origin

	output_file.write((char *)&header, sizeof(header));
	if (!output_file.good()) 
	{
		output_file.close();
		std::cerr << "Can't save tga file header.\n";
		return false;
	}

	if (!is_compress)
	{
		auto color_size = sizeof(Color); // default = 32bits

		if (header.image_spec_.bits_per_pixel_ == 24)
			color_size -= 1;

		unsigned int count = 0;
		unsigned int maximum = static_cast<unsigned int>(data_.size());
		for (auto& itr : data_)
		{
			// TGA pixel color is in form of : BGRA.
			// So swap RGBA to BGRA
			std::swap(itr.red, itr.blue);

			std::cout << "Saving " << count++ << " of " << maximum << "datas." << std::endl;

			output_file.write(reinterpret_cast<char*>(&itr), color_size);
			if (!output_file.good())
			{
				std::cerr << "Can't save raw data\n";
				output_file.close();
				return false;
			}
		}
	}
	else
		CompressRLE(output_file);


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

Color TGAImage::GetPixel(unsigned int _x_coord, unsigned int _y_coord)
{
	if (!IsInsideBoundary(_x_coord, _y_coord))
	{
		std::cerr << "Coordinate is out of boundary, return initial color.\n";
		return Color();
	}

	return data_[((width_ * _y_coord) - 1) + _x_coord];
}

void TGAImage::SetPixel(unsigned int _x_coord, unsigned int _y_coord, Color _color)
{
	if (!IsInsideBoundary(_x_coord, _y_coord))
		return;

	data_[(width_ * _y_coord) + _x_coord] = _color;
}

short TGAImage::GetWidth()
{
	return width_;
}
short TGAImage::GetHeight()
{
	return height_;
}

bool TGAImage::IsInsideBoundary(unsigned int _x_coord, unsigned int _y_coord)
{
	if (_x_coord >= static_cast<unsigned int>(width_))
	{
		std::cerr << "X coord is out of boundary\n";
		return false;
	}
		
	if (_y_coord >= static_cast<unsigned int>(height_))
	{
		std::cerr << "Y coord is out of boundary\n";
		return false;
	}

	return true;
}