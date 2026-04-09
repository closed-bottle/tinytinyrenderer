#include "imageFormats/TGAImage.h"
#include "geometryFormats/OBJGeometry.h"
#include <iostream>
#include <string>
#include <chrono>

constexpr int width = 3000;
constexpr int height = 2000;


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

int main(int argc, const char* argv[])
{
	TimeStamp::Start();
	TGA_Image image_light(width, height, 3);

	OBJ_Geometry obj;
	obj.SetZoom(1000.0f);
	obj.SetWidthHeight(width, height);
	obj.LoadFromOBJFile("suzanne.obj");
	obj.DrawWithFlatLight(image_light);

	image_light.FlipVertically();
	image_light.SaveToTGAFile("suzanne_light.tga", true);

	TimeStamp::End();
	std::cout << "Total : " << TimeStamp::Duration() << std::endl;
	//getchar();
	return 0;
}