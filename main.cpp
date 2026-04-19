#include "renderUtils/Memory.h"
#include <iostream>
#include <string>
#include <chrono>

#include "fileIO/FileReader.h"
#include "fileIO/FileWriter.h"
#include "renderUtils/CommandBuff.h"
#include "renderUtils/Image.h"
#include "renderUtils/Pipeline.h"
#include "renderUtils/Render.h"
#include "renderUtils/RenderCmd.h"
#include "renderUtils/Viewport.h"


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

constexpr int width = 64;
constexpr int height = 64;
constexpr float near = 0.1f;
constexpr float far  = 1000;
constexpr int channel = 3;

int main(int argc, const char* argv[])
{
	TimeStamp::Start();

	Geometry geom;
	Mesh mesh;
	FileReader::LoadGeometryFile<FFormat::OBJ>("Tri.obj", geom, mesh);

	Lamp::Mat4f model = Lamp::Mat4f::Scale(2.8, 2.8, 2.8);
	Lamp::Mat4f view = Lamp::Mat4f::LookAt({0, 0, 10}, {}, {0, 1, 0}, false);
	Lamp::Mat4f proj = Lamp::Mat4f::Perspective(3.141592/180.0f * 40.5f,
		(float)width / height, 0.1f, 1000.0f);
	auto mvp = proj * view * model;
	Render::UMvp u_mvp = {ShaderName::LineShader, mvp};

	VertexBuffer pos_buffer = {geom.vertex_.Data() + mesh.vertex_offset_, mesh.vertex_count_};
	IndexBuffer index_buffer = {geom.index_.Data() + mesh.index_offset_, mesh.index_count_};

	Viewport viewport = {0, 0, width, height,near, far};

	Memory image_memory(2 * (width * height * channel));
	Image<PixelFormat::B8G8R8> color_att(image_memory, 0, width, height);
	Image<PixelFormat::D16> depth_att(image_memory, (width * height * channel), width, height);

	AttInfo color_att_info = {
		color_att,LoadOp::LOAD_OP_CLEAR,StoreOp::STORE_OP_STORE,{0,0,0}
	};

	AttInfo depth_att_info = {
		depth_att,LoadOp::LOAD_OP_CLEAR,StoreOp::STORE_OP_STORE,{0}
	};

	RenderInfo render_info = {
		1, &color_att_info, &depth_att_info
	};

	Pipeline render_pipeline = {WindingOrder::CCW,
					1, &color_att,
					1, &depth_att,
					ShaderName::LineShader};

	CommandBuff cmd_buff;
	RenderCmd::BeginCmd(cmd_buff);
	// Barrier is not implemented yet.
	// Scissor is not implemented at this moment, and most likely will not implement.
	RenderCmd::SetViewport(cmd_buff, viewport);
	RenderCmd::SetRenderInfo(cmd_buff, render_info);
	RenderCmd::BeginRender(cmd_buff);
	RenderCmd::BindPipeline(cmd_buff, render_pipeline);
	RenderCmd::BindUniform(cmd_buff, sizeof(u_mvp), u_mvp);

	RenderCmd::BindVertexBuffer(cmd_buff, pos_buffer, 0);
	RenderCmd::BindIndexBuffer(cmd_buff, index_buffer, 0);

	RenderCmd::DrawIndexed(cmd_buff, 0);

	RenderCmd::EndRender(cmd_buff);
	RenderCmd::EndCmd(cmd_buff);

	// Implement queue
	// Submit queue


	FileWriter::WriteImageToFile<FFormat::TGACompressed>("color.tga", color_att);
	FileWriter::WriteImageToFile<FFormat::TGACompressed>("depth.tga", depth_att);

	TimeStamp::End();
	std::cout << "Total : " << TimeStamp::Duration() << std::endl;
	//getchar();
	return 0;
}