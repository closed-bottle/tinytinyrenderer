#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <random>
#include <utility>
#include <algorithm>
#include "OBJGeometry.h"

void OBJGeometry::SetWidth(float _new_width)
{
	width_ = _new_width;
}

void OBJGeometry::SetHeight(float _new_height)
{
	height_ = _new_height;
}

void OBJGeometry::SetWidthHeight(float _new_width, float _new_height)
{
	SetWidth(_new_width);
	SetHeight(_new_height);
}

void OBJGeometry::SetZoom(float _new_zoom)
{
	zoom_ = _new_zoom;
}

OBJGeometry::OBJGeometry(std::string _filename)
{
	LoadFromOBJFile(_filename);
}

OBJGeometry::OBJGeometry()
{
}

bool OBJGeometry::LoadFromOBJFile(std::string _filename)
{
	std::ifstream input_file(_filename);

	if (!input_file.is_open())
	{
		std::cerr << "Can't open file : " << _filename.c_str() << "\n";
		input_file.close();
		return false;
	}
	
	std::string buffer;
	while (getline(input_file, buffer))
	{
		// If line is comment, skip.
		if ((*std::begin(buffer)) == '#')
			continue;

		std::stringstream buffer_stream(buffer);
		std::string stream_segment;
		while (buffer_stream >> stream_segment)
		{
			if (stream_segment == "mtllib") // Material library
			{
				buffer_stream >> stream_segment; // Assume that there are no space character in file name.
				material_library_name_ = stream_segment;
				// TODO : LoadFromMTLFile() or something.
				continue;
			}
			else if (stream_segment == "o") // Object name
			{
				buffer_stream >> stream_segment; // Assume that there are no space character in object name.
				// TODO
				continue;
			}
			else if (stream_segment == "v") // Geometric vertex
			{
				Vec3f vertex(0.0f, 0.0f, 0.0f);

				buffer_stream >> stream_segment;
				vertex.x = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.y = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.z = std::stof(stream_segment);

				geometric_vertices_.push_back(vertex * zoom_);
				
				continue;
			}
			else if (stream_segment == "vt") // Texture vertex
			{
				Vec3f vertex(0.0f, 0.0f, 0.0f);

				buffer_stream >> stream_segment;
				vertex.x = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.y = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.z = std::stof(stream_segment);

				texture_vertices_.push_back(vertex);

				continue;
			}
			else if (stream_segment == "vn") // Vertex normal
			{
				Vec3f vertex(0.0f, 0.0f, 0.0f);

				buffer_stream >> stream_segment;
				vertex.x = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.y = std::stof(stream_segment);

				buffer_stream >> stream_segment;
				vertex.z = std::stof(stream_segment);

				vertex_normals_.push_back(vertex);

				continue;
			}
			else if (stream_segment == "f") // Vertex normal
			{
				/*
				float half_width = width_ / 2;
				float half_height = height_ / 2;
				// First line.
				buffer_stream >> stream_segment;
				Vec3f startVec3 = geometric_vertices_[std::stoi(stream_segment) - 1];
				Vec3f first_vertex = geometric_vertices_[std::stoi(stream_segment) - 1];
				const Vec3f veryFirstVec3 = startVec3;

				buffer_stream >> stream_segment;
				Vec3f endVec3 = geometric_vertices_[std::stoi(stream_segment) - 1];
				Vec3f second_vertex = geometric_vertices_[std::stoi(stream_segment) - 1];
				wire_frame_.push_back(Line(Vec2i(static_cast<int>(startVec3.x + half_width), static_cast<int>(startVec3.y + half_height)), 
										   Vec2i(static_cast<int>(endVec3.x + half_width), static_cast<int>(endVec3.y + half_height))));
				Vec3f third_vertex = second_vertex;
				bool isTri = false;
				while (buffer_stream >> stream_segment)
				{
					startVec3 = endVec3;

					endVec3 = geometric_vertices_[std::stoi(stream_segment) - 1];
					third_vertex = geometric_vertices_[std::stoi(stream_segment) - 1];
					wire_frame_.push_back(Line(Vec2i(static_cast<int>(startVec3.x + half_width), static_cast<int>(startVec3.y + half_height)),
											   Vec2i(static_cast<int>(endVec3.x + half_width), static_cast<int>(endVec3.y + half_height))));
					isTri = true;
				}

				if (isTri)
				{
					Vec3f first((first_vertex.x + half_width), (first_vertex.y + half_height), first_vertex.z);
					Vec3f second((second_vertex.x + half_width), (second_vertex.y + half_height), second_vertex.z);
					Vec3f third((third_vertex.x + half_width), (third_vertex.y + half_height), third_vertex.z);

					Triangle tri(first, second, third);
					tri.SetNormal(Vec3f::CrossProduct((second -= first), (third -= first)));
					faces_.push_back(tri);
				}
					
				wire_frame_.push_back(Line(Vec2i(static_cast<int>(endVec3.x + half_width), static_cast<int>(endVec3.y + half_height)), 
										   Vec2i(static_cast<int>(veryFirstVec3.x + half_width), static_cast<int>(veryFirstVec3.y + half_height))));
*/
				continue;
			}
		}
			
	}
	


	return true;
}