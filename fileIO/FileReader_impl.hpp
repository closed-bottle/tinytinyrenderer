#include "FileReader.h"
#include <fstream>

#include "special-lamp/lampMath.h++"


namespace {
    bool LoadOBJ(const Lamp::String& _path, Geometry &_out_geom, Mesh& _out_mesh) {
        std::ifstream input_file(_path.c_str());
        if (!input_file.is_open())
        {
            std::cerr << "Failed to open file : " << _path.c_str() << '\n';
            input_file.close();
            return false;
        }

        // TODO : Possibly remake this with mmap in the future.
        //        Doesn't need to for simple OBJ, but I'd like to try.

    	Lamp::Vector<Lamp::Vec3f> vertices;
    	Lamp::Vector<Lamp::Vec3f> vnormals_unsorted;
    	Lamp::Vector<Lamp::Vec3f> vnormals;
    	Lamp::Vector<Lamp::Vec3f> uvs_unsorted;
    	Lamp::Vector<Lamp::Vec3f> uvs;
    	Lamp::Vector<uint32_t> indices;
        std::string buffer;

        Lamp::Vec3f minimum = {std::numeric_limits<float>::max(),
        						std::numeric_limits<float>::max(),
        						std::numeric_limits<float>::max()};
    	Lamp::Vec3f maximum = -minimum;

        while (getline(input_file, buffer)) {
            // If line is comment, skip.
            if ((*std::begin(buffer)) == '#')
                continue;

			std::stringstream buffer_stream(buffer);
			std::string stream_segment;
			while (buffer_stream >> stream_segment)
			{
				if (stream_segment == "mtllib") // Material library
				{
					// Assume that there are no space character in file name.
					buffer_stream >> stream_segment;
					_out_mesh.material_name_ = stream_segment.c_str();
					// TODO : LoadFromMTLFile() or something.
				}
				else if (stream_segment == "o") // Object name
				{
					// Assume that there are no space character in object name.
					buffer_stream >> stream_segment;
					_out_mesh.object_name_ = stream_segment.c_str();
				}
				else if (stream_segment == "v") // Geometric vertex
				{
					Lamp::Vec3f vertex(0.0f, 0.0f, 0.0f);

					buffer_stream >> stream_segment;
					vertex.x = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.y = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.z = std::stof(stream_segment);

					_out_mesh.aabb.max.x = std::max(maximum.x, vertex.x);
					_out_mesh.aabb.max.y = std::max(maximum.y, vertex.y);
					_out_mesh.aabb.max.z = std::max(maximum.z, vertex.z);
					_out_mesh.aabb.min.x = std::min(minimum.x, vertex.x);
					_out_mesh.aabb.min.y = std::min(minimum.y, vertex.y);
					_out_mesh.aabb.min.z = std::min(minimum.z, vertex.z);


					// Push back dummies for attributes, so number of element for each vertex attributes
					// matches the number of vertices even if there are duplicates.
					vertices.push_back(vertex);
					vnormals.push_back({});
					uvs.push_back({});
				}
				else if (stream_segment == "vt") // Texture vertex
				{
					Lamp::Vec3f vertex(0.0f, 0.0f, 0.0f);

					buffer_stream >> stream_segment;
					vertex.x = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.y = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.z = std::stof(stream_segment);

					uvs_unsorted.push_back(vertex);
				}
				else if (stream_segment == "vn") // Vertex normal
				{
					Lamp::Vec3f vertex(0.0f, 0.0f, 0.0f);

					buffer_stream >> stream_segment;
					vertex.x = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.y = std::stof(stream_segment);

					buffer_stream >> stream_segment;
					vertex.z = std::stof(stream_segment);

					vnormals_unsorted.push_back(vertex);
				}
				else if (stream_segment == "f") // face, process only triangles.
				{
					std::getline(buffer_stream, stream_segment, ' '); // Skip first whitespace
					while (std::getline(buffer_stream, stream_segment, ' ')) {
						std::stringstream buffer_stream(stream_segment);
						std::getline(buffer_stream, stream_segment, '/');
						uint32_t v = std::stoi(stream_segment) -1;
						indices.push_back(v);

						// Post process vertices, so index for "v" matches 1:1:1 with "vn" and "vt"
						// TODO : UVs are most likely 2d vectors instead of 3d.
						std::getline(buffer_stream, stream_segment, '/');
						uint32_t vt = 0;
						if (!stream_segment.empty()) {
							vt = std::stoi(stream_segment) -1;
							uvs[v] = uvs_unsorted[vt];
						}


						std::getline(buffer_stream, stream_segment, '/');
						uint32_t vn = 0;
						if (!stream_segment.empty()) {
							vn = std::stoi(stream_segment) -1;
							vnormals[v] = vnormals_unsorted[vn];
						}
					}
				}
			}
        }


    	// Don't forget to consider alignment when merging multiple memories.
    	_out_geom.vertex_ = Memory(sizeof(Lamp::Vec3f) * vertices.size() + alignof(Lamp::Vec3f));
    	_out_geom.vertex_normal_ = Memory(sizeof(Lamp::Vec3f) * vnormals.size() + alignof(Lamp::Vec3f));
    	_out_geom.uv_ = Memory(sizeof(Lamp::Vec3f) * uvs.size() + alignof(Lamp::Vec3f));
    	_out_geom.index_ = Memory(sizeof(uint32_t) * indices.size() + alignof(uint32_t));

    	_out_mesh.vertex_count_ = vertices.size();
    	_out_mesh.vertex_offset_
    	= alignof(Lamp::Vec3f) - (reinterpret_cast<uint64_t>(_out_geom.vertex_.Data()) % alignof(Lamp::Vec3f));
    	_out_mesh.vnormal_count_ = vnormals.size();
    	_out_mesh.vnormal_offset_
		= alignof(Lamp::Vec3f) - (reinterpret_cast<uint64_t>(_out_geom.vertex_normal_.Data()) % alignof(Lamp::Vec3f));
    	_out_mesh.index_count_ = indices.size();
    	_out_mesh.index_offset_
    	= alignof(uint32_t) - (reinterpret_cast<uint64_t>(_out_geom.index_.Data()) % alignof(uint32_t));

    	memcpy(_out_geom.vertex_.Data() + _out_mesh.vertex_offset_,
			vertices.data(),
			sizeof(Lamp::Vec3f) * vertices.size());

    	if (!vnormals.empty()) {
    		memcpy(_out_geom.vertex_normal_.Data() + _out_mesh.vnormal_offset_,
				vnormals.data(),
				sizeof(Lamp::Vec3f) * vnormals.size());
    	}

    	if (!uvs.empty()) {
    		memcpy(_out_geom.uv_.Data() + _out_mesh.vertex_offset_,
				uvs.data(),
				sizeof(Lamp::Vec3f) * uvs.size());
    	}

    	if (!indices.empty()) {
    		memcpy(_out_geom.index_.Data() + _out_mesh.index_offset_,
				indices.data(),
				sizeof(uint32_t) * indices.size());
    	}


        return true;
    }
}



template<FFormat FF>
bool FileReader::LoadGeometryFile(const Lamp::String& _path, Geometry &_out_geom, Mesh& _out_mesh) {
    if (FF == FFormat::OBJ)
        return LoadOBJ(_path, _out_geom, _out_mesh);

    return false;
}
