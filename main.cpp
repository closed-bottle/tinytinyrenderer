#include "renderUtils/Memory.h"
#include <iostream>
#include <string>
#include <chrono>

#include "fileIO/FileReader.h"
#include "fileIO/FileWriter.h"
#include "renderUtils/Image.h"
#include "renderUtils/Render.h"


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

constexpr int width = 512;
constexpr int height = 512;
constexpr int channel = 3;

int main(int argc, const char* argv[])
{
	TimeStamp::Start();
	Memory image_memory(width * height * channel);
	Image<PixelFormat::B8G8R8> render_target(image_memory, width, height);
	render_target.FillImage({0, 0, 0});

	Geometry geom;
	Mesh mesh;
	FileReader::LoadGeometryFile<FFormat::OBJ>("suzanne.obj", geom, mesh);

	Lamp::Mat4f model = Lamp::Mat4f::Scale(1, 1, 1);
	Lamp::Mat4f view = Lamp::Mat4f::LookAt({0, 0, 10}, {}, {0, 1, 0}, false);
	Lamp::Mat4f proj = Lamp::Mat4f::Perspective(3.141592/180.0f * 40.5f,
		width / height, 0.1f, 1000.0f, true);

	//auto mvp = model * view * proj;
	auto mvp = proj * view * model;
	Render::UPointShader uniform(ShaderName::PointShader, mvp);
	Render::Draw(render_target, geom, mesh, &uniform);

	FileWriter::WriteImageToFile<FFormat::TGACompressed>("fwriter.tga", render_target);

	TimeStamp::End();
	std::cout << "Total : " << TimeStamp::Duration() << std::endl;
	//getchar();
	return 0;
}