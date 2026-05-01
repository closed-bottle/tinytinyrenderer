#include "renderUtils/Memory.h"
#include <iostream>
#include <string>
#include <chrono>
#include "renderUtils/SIMD.h"

#include "fileIO/FileReader.h"
#include "fileIO/FileWriter.h"
#include "renderUtils/CommandBuff.h"
#include "renderUtils/Image.h"
#include "renderUtils/Pipeline.h"
#include "renderUtils/Render.h"
#include "renderUtils/RenderCmd.h"
#include "renderUtils/Viewport.h"
#include "renderUtils/ImageFormat.h"


// Borrowed from special-lamp library.
class TimeStamp {
	static std::chrono::steady_clock::time_point start_;
	static std::chrono::steady_clock::time_point end_;

public:
	static void Start() {
		start_ = std::chrono::steady_clock::now();
	}

	static void End() {
		end_ = std::chrono::steady_clock::now();
	}

	static std::chrono::duration<double> Duration() {
		return end_ - start_;
	}
	static TimeStamp& instance;
};

std::chrono::steady_clock::time_point TimeStamp::start_;
std::chrono::steady_clock::time_point TimeStamp::end_;
TimeStamp gTimeStamp;
TimeStamp& TimeStamp::instance = gTimeStamp;

//constexpr uint32_t x_resolution = 2048;
//constexpr uint32_t y_resolution = 2048;
//constexpr uint8_t x_count = 4;
//constexpr uint8_t y_count = 4;
constexpr uint32_t x_resolution = 512;
constexpr uint32_t y_resolution = 512;
constexpr uint8_t x_count = 1;
constexpr uint8_t y_count = 1;
constexpr uint64_t width = x_resolution * x_count;
constexpr uint64_t height = y_resolution * y_count;
constexpr float near = 0.1f;
constexpr float far  = 1000;
constexpr int channel = 3;

int main(int argc, const char* argv[]) {
	TimeStamp::Start();

	Geometry geom;
	Mesh mesh;
	FileReader::LoadGeometryFile<FFormat::OBJ>("suzanne.obj", geom, mesh);

	Lamp::Mat4f model = Lamp::Mat4f::Scale(5, 5, 5);
	Lamp::Mat4f view = Lamp::Mat4f::LookAt({0, 0, 10}, {}, {0, 1, 0}, false);
	Lamp::Mat4f proj = Lamp::Mat4f::Perspective(3.141592/180.0f * 60.5f,
		(float)width / height, 0.1f, 1000.0f);
	auto mvp = proj * view * model;


	// TODO : Do SoA for device buffers.
	VertexBuffer pos_buffer(sizeof(Lamp::Vec3f),geom.vertex_.Data() + mesh.vertex_offset_, mesh.vertex_count_);
	IndexBuffer index_buffer = {geom.index_.Data() + mesh.index_offset_, mesh.index_count_};

	Memory image_memory(16 * (width * height * channel));
	memset(image_memory.Data(), 0, 16 * (width * height * channel));
	Image color_att(image_memory, PixelFormat::B8G8R8, 0, width, height);
	// TODO : Handle alignment? Why it works?
	//Image depth_att(image_memory, PixelFormat::D16, (width * height * channel), width, height);

	B8G8R8 clear_color = {0, 0, 0};
	D16 clear_depth = {0};

	AttInfo color_att_info = {
		color_att,LoadOp::LOAD_OP_CLEAR,StoreOp::STORE_OP_STORE,&clear_color
	};

	//AttInfo depth_att_info = {
	//	depth_att,LoadOp::LOAD_OP_CLEAR,StoreOp::STORE_OP_STORE, &clear_depth
	//};

	RenderInfo render_info = {
		//1, &color_att_info, &depth_att_info
		1, &color_att_info, nullptr
	};
	Pipeline render_pipeline = {WindingOrder::CCW, ShaderName::RasterShader};

	for (uint8_t i = 0; i < x_count; ++i) {
		for (uint8_t j = 0; j < y_count; ++j) {
			Viewport viewport = {static_cast<float>(x_resolution) * i,
								 static_cast<float>(y_resolution) * j, x_resolution, y_resolution, near, far};

			float rad;
			rad = (static_cast<float>(i * y_count + j) / (static_cast<float>(x_count) * y_count));
			rad *= 2.0f * 3.141592f;

			model = Lamp::Mat4f::Translate(0, 0, 0) *
					Lamp::Mat4f::Pitch(rad) *
				    Lamp::Mat4f::Scale(1, -1, 1);


			mvp = view * model;

			CommandBuff cmd_buff;
			RenderCmd::BeginCmd(cmd_buff);
			// Barrier is not implemented yet.
			// Scissor is not implemented at this moment, and most likely will not implement.

			RenderCmd::SetViewport(cmd_buff, viewport);
			RenderCmd::SetRenderInfo(cmd_buff, render_info);
			RenderCmd::BeginRender(cmd_buff);
			RenderCmd::BindPipeline(cmd_buff, render_pipeline);

			RenderCmd::BindVertexBuffer(cmd_buff, pos_buffer, 0);
			RenderCmd::BindIndexBuffer(cmd_buff, index_buffer, 0);

			Render::UMvp u_mvp0 = {ShaderName::RasterShader, mvp};
			RenderCmd::BindUniform(cmd_buff, sizeof(u_mvp0), u_mvp0);
			RenderCmd::DrawIndexed(cmd_buff, 0);

			RenderCmd::EndRender(cmd_buff);
			RenderCmd::EndCmd(cmd_buff);

			// Implement queue
			// Submit queue

			// Remove templates later for runtime format specification.
			if (cmd_buff.IsExecutable())
				cmd_buff.Execute();
			cmd_buff.Clear();
		}
	}



	//FileWriter::WriteImageToFile<FFormat::TGACompressed>("color.tga", color_att);
	FileWriter::WriteImageToFile<FFormat::TGACompressed>("color.tga", color_att);
	//FileWriter::WriteImageToFile<FFormat::TGACompressed>("depth.tga", depth_att);

	TimeStamp::End();
	std::cout << "Total : " << TimeStamp::Duration() << std::endl;
	//getchar();
	return 0;
}