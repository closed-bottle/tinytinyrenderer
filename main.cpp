#include "renderUtils/Memory.h"
//#include "fileFormats/TGA.h"
#include "geometryFormats/OBJGeometry.h"
#include <iostream>
#include <string>
#include <chrono>

#include "fileFormats/FileWriter.h"
#include "renderUtils/Image.h"


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

constexpr int width = 3000;
constexpr int height = 2500;
constexpr int channel = 3;

int main(int argc, const char* argv[])
{
	TimeStamp::Start();


	Memory memory(width * height * channel);
	Image<PixelFormat::R8G8B8> render_target(memory, width, height);

	FileWriter::WriteImageToFile<FileWriter::FFormat::TGACompressed>("fwriter.tga", render_target);

	TimeStamp::End();
	std::cout << "Total : " << TimeStamp::Duration() << std::endl;
	//getchar();
	return 0;
}