#include "LineRasterizeAlgorithm.h"
#include <cassert>

namespace Bresenhams
{
	Line::Line(Vec2i _start, Vec2i _end)
		: start(_start), end(_end)
	{
		::Line line(_start, _end);
		GetComponents(line);
	}

	Line::Line(::Line _line)
		: start(_line.GetStartPoint()), end(_line.GetEndPoint())
	{
		GetComponents(_line);
	}

	void Line::GetComponents(::Line& _line)
	{
		// Rate of changes.
		int dy = _line.GetDy();
		int dx = _line.GetDx();

		// Pixel position x, y.
		x = start.x;
		y = start.y;

		if (_line.GetSlope() >= 0 && start.x > end.x) // if starting position is right hand side.
		{
			x = end.x;
			y = end.y;

			dx = -dx;
			dy = -dy;
		}
		else if(_line.GetSlope() < 0 && start.x > end.x)
		{
			x = end.x;
			y = end.y;

			dx = -dx;
			dy = -dy;
		}

		num_of_pixel = abs(dx) +1; // +1 for starting pixel itself.

		const int x_step_sign = (dx > 0) ? 1 : -1;
		const int y_step_sign = (dy > 0) ? 1 : -1;

		const int x_step = (2 * (dy * x_step_sign));
		const int y_step = (2 * (-dx * y_step_sign));

		determinant = 2 * (dy * x_step_sign) + (-dx * y_step_sign);

		// Plot initial pixel on starting point.


		auto even_octant = [](int const& _determinant) { return _determinant >= 0; };
		auto odd_octant = [](int const& _determinant) { return _determinant <= 0; };

		// Default setting is for octant 0.
		determining_function = even_octant;


		major_axis = &x;
		minor_axis = &y;

		major_step = x_step;
		minor_step = y_step;

		major_sign = x_step_sign;
		minor_sign = y_step_sign;


		if (abs(dy) > abs(dx))
		{
			num_of_pixel = abs(dy) +1; // +1 for starting pixel itself.

			if (dx > 0)
			{
				if (dy > 0)
					determining_function = odd_octant;
				else
					determining_function = even_octant;
			}
			else
			{
				if (dy > 0)
					determining_function = even_octant;
				else
					determining_function = odd_octant;
			}


			major_axis = &y;
			minor_axis = &x;

			std::swap(major_step, minor_step);

			major_sign = y_step_sign;
			minor_sign = x_step_sign;

			determinant = (minor_step / 2) + major_step;
		}
		else
		{
			if (dx > 0)
			{
				if (dy > 0)
					determining_function = even_octant;
				else
					determining_function = odd_octant;
			}
			else
			{
				if (dy > 0)
					determining_function = odd_octant;
				else
					determining_function = even_octant;
			}
		}
	}

	void LineRasterize(TGAImage& _target, const Vec2i& _start, const Vec2i& _end, const Color& _color)
	{
		Line line(_start, _end);

		_target.SetPixel(line.x, line.y, _color);
		// Actual pixel plotting.
		while (--line.num_of_pixel > 0)
		{
			if (line.determining_function(line.determinant))
			{
				line.determinant += line.minor_step;
				(*line.minor_axis) += line.minor_sign;
			}

			line.determinant += line.major_step;
			(*line.major_axis) += line.major_sign;
			_target.SetPixel(line.x, line.y, _color);
		}
	}

	void LineRasterize(TGAImage & _target, ::Line _line, const Color & _color)
	{
		Line line(_line);

		_target.SetPixel(line.x, line.y, _color);
		// Actual pixel plotting.
		while (--line.num_of_pixel > 0)
		{
			if (line.determining_function(line.determinant))
			{
				line.determinant += line.minor_step;
				(*line.minor_axis) += line.minor_sign;
			}

			line.determinant += line.major_step;
			(*line.major_axis) += line.major_sign;
			_target.SetPixel(line.x, line.y, _color);
		}
	}
}