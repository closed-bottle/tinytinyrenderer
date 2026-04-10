#pragma once
#include "../ImageFormats/TGAImage.h"
#include "Line.h"

namespace Bresenhams
{
	struct Line
	{
		Line(Vec2i _start, Vec2i _end);
		Line(::Line _line);

		void GetComponents(::Line& _line);

		Vec2i start;
		Vec2i end;

		int x = 0;
		int y = 0;

		int determinant = 0;
		bool(*determining_function)(int const&) = nullptr;

		int* major_axis = nullptr;
		int* minor_axis = nullptr;

		int major_step = 0;
		int minor_step = 0;

		int major_sign = 0;
		int minor_sign = 0;

		int num_of_pixel = 0;
	};
	void LineRasterize(TGAImage& _target, const Vec2i& _start, const Vec2i& _end, const Color& _color);
	void LineRasterize(TGAImage& _target, ::Line _line, const Color& _color);
}