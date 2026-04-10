#include <algorithm>
#include "Triangle.h"
#include "../lineAlgorithm/LineRasterizeAlgorithm.h"

void Triangle::IdentifyTopMiddleBottom(Vec3f const & _a, Vec3f const & _b, Vec3f const & _c)
{
	auto top_y = std::max(std::max(_a.y, _b.y), std::max(_b.y, _c.y));

	if (top_y == _a.y)
	{
		top_ = _a;

		auto middle_y = std::max(_b.y, _c.y);
		if (middle_y == _b.y)
		{
			middle_ = _b;
			bottom_ = _c;
		}
		else if (middle_y == _c.y)
		{
			middle_ = _c;
			bottom_ = _b;
		}
	}
	else if (top_y == _b.y)
	{
		top_ = _b;

		auto middle_y = std::max(_a.y, _c.y);
		if (middle_y == _a.y)
		{
			middle_ = _a;
			bottom_ = _c;
		}
		else if (middle_y == _c.y)
		{
			middle_ = _c;
			bottom_ = _a;
		}
	}
	else if (top_y == _c.y)
	{
		top_ = _c;

		auto middle_y = std::max(_a.y, _b.y);
		if (middle_y == _a.y)
		{
			middle_ = _a;
			bottom_ = _b;
		}
		else if (middle_y == _b.y)
		{
			middle_ = _b;
			bottom_ = _a;
		}
	}
}

void Triangle::IdentifyLeftRight()
{
	if (middle_.x < bottom_.x)
	{
		left_ = &middle_;
		right_ = &bottom_;
	}
	else
	{
		left_ = &bottom_;
		right_ = &middle_;
	}
		
}

Triangle::Triangle(Vec2i const& _a, Vec2i const& _b, Vec2i const& _c)
	: top_(0.0f, 0.0f, 0.0f), middle_(0.0f, 0.0f, 0.0f), bottom_(0.0f, 0.0f, 0.0f), left_(&middle_), right_(&bottom_),
	  face_normal_(0.0f, 0.0f, 0.0f)
{
	IdentifyTopMiddleBottom(Vec3f(static_cast<float>(_a.x), static_cast<float>(_a.y), 0.0f),
							Vec3f(static_cast<float>(_b.x), static_cast<float>(_b.y), 0.0f),
							Vec3f(static_cast<float>(_c.x), static_cast<float>(_c.y), 0.0f));
	IdentifyLeftRight();
}

Triangle::Triangle(Vec3f const& _a, Vec3f const& _b, Vec3f const& _c)
	: top_(0.0f, 0.0f, 0.0f), middle_(0.0f, 0.0f, 0.0f), bottom_(0.0f, 0.0f, 0.0f), left_(&middle_), right_(&bottom_),
	  face_normal_(0.0f, 0.0f, 0.0f)
{
	IdentifyTopMiddleBottom(_a, _b, _c);
	IdentifyLeftRight();
}

Triangle::Triangle(const Triangle & _triangle)
	: top_(_triangle.top_), middle_(_triangle.middle_), bottom_(_triangle.bottom_), left_(nullptr), right_(nullptr),
	  face_normal_(_triangle.face_normal_)
{
	IdentifyLeftRight();
}

Vec3f Triangle::GetLeft()
{
	return *left_;
}

Vec3f Triangle::GetRight()
{
	return *right_;
}

Vec3f Triangle::GetTop()
{
	return top_;
}

Vec3f Triangle::GetMiddle()
{
	return middle_;
}

Vec3f Triangle::GetNormal()
{
	return face_normal_;
}

void Triangle::SetNormal(Vec3f& _normal)
{
	face_normal_ = _normal;
}

void Triangle::SetNormal(Vec3f _normal)
{
	face_normal_ = _normal;
}

void Triangle::TriangleRasterize(TGAImage& _image, Triangle& _triangle, Color& _color)
{
	// Some pixel dulplicated.
	Line topToMid(Vec2i(static_cast<int>(_triangle.top_.x), static_cast<int>(_triangle.top_.y)),
				  Vec2i(static_cast<int>(_triangle.middle_.x), static_cast<int>(_triangle.middle_.y)));

	Line topToBot(Vec2i(static_cast<int>(_triangle.top_.x), static_cast<int>(_triangle.top_.y)),
				  Vec2i(static_cast<int>(_triangle.bottom_.x), static_cast<int>(_triangle.bottom_.y)));

	Line midToBot(Vec2i(static_cast<int>(_triangle.middle_.x), static_cast<int>(_triangle.middle_.y)),
				  Vec2i(static_cast<int>(_triangle.bottom_.x), static_cast<int>(_triangle.bottom_.y)));

	float left_x = static_cast<float>(topToMid.GetStartPoint().x);
	float right_x = static_cast<float>(topToBot.GetStartPoint().x);
	float left_slope = topToMid.GetSlope();
	float right_slope = topToBot.GetSlope();
	int y = static_cast<int>(_triangle.top_.y);


	auto x_value_of_top_to_bot_on_middles_y = (_triangle.top_.y - _triangle.middle_.y) * -(1.0f / topToBot.GetSlope());

	if (std::isinf(-(1.0f / topToBot.GetSlope())))
		x_value_of_top_to_bot_on_middles_y = (_triangle.top_.y - _triangle.middle_.y);

	if (_triangle.middle_.x < _triangle.top_.x + x_value_of_top_to_bot_on_middles_y)
		_triangle.triangle_type_ = TriangleType::Left;
	else
	{
		_triangle.triangle_type_ = TriangleType::Right;
		left_x = static_cast<float>(topToBot.GetStartPoint().x);
		right_x = static_cast<float>(topToMid.GetStartPoint().x);
		left_slope = topToBot.GetSlope();
		right_slope = topToMid.GetSlope();
	}


	float left_step = -1.0f / left_slope;
	float right_step = -1.0f / right_slope;

	if (std::isinf(left_step) || std::isnan(left_step))
		left_step = 0;
	if (std::isinf(right_step) || std::isnan(right_step))
		right_step = 0;

	// Top half
	while (y > _triangle.middle_.y)
	{
		for(int x = static_cast<int>(left_x); x <= right_x; x++)
			_image.SetPixel(x, y, _color);

		left_x += left_step;
		right_x += right_step;

		y--;
	}



	if (_triangle.triangle_type_ == TriangleType::Left)
	{
		left_x = static_cast<float>(midToBot.GetStartPoint().x);
		left_slope = midToBot.GetSlope();
		left_step = -1.0f / left_slope;
	}
	else
	{
		right_x = static_cast<float>(midToBot.GetStartPoint().x);
		right_slope = midToBot.GetSlope();
		right_step = -1.0f / right_slope;
	}

	if (std::isinf(left_step) || std::isnan(left_step))
		left_step = 0;
	if (std::isinf(right_step) || std::isnan(right_step))
		right_step = 0;


	// Bottom half
	while (y >= _triangle.bottom_.y)
	{
		for (int x = static_cast<int>(left_x); x <= right_x; x++)
			_image.SetPixel(x, y, _color);

		left_x += left_step;
		right_x += right_step;

		y--;
	}
}


void Triangle::TriangleOutlineRasterize(TGAImage& _image, Triangle& _triangle, Color& _color)
{
	// TODO : Optimizing. 
	// Some pixel dulplicated.
	Line topToMid(Vec2i(static_cast<int>(_triangle.top_.x), static_cast<int>(_triangle.top_.y)),
		Vec2i(static_cast<int>(_triangle.middle_.x), static_cast<int>(_triangle.middle_.y)));

	Line topToBot(Vec2i(static_cast<int>(_triangle.top_.x), static_cast<int>(_triangle.top_.y)),
		Vec2i(static_cast<int>(_triangle.bottom_.x), static_cast<int>(_triangle.bottom_.y)));

	Line midToBot(Vec2i(static_cast<int>(_triangle.middle_.x), static_cast<int>(_triangle.middle_.y)),
		Vec2i(static_cast<int>(_triangle.bottom_.x), static_cast<int>(_triangle.bottom_.y)));

	float left_x = static_cast<float>(topToMid.GetStartPoint().x);
	float right_x = static_cast<float>(topToBot.GetStartPoint().x);
	float left_slope = topToMid.GetSlope();
	float right_slope = topToBot.GetSlope();
	int y = static_cast<int>(_triangle.top_.y);

	if (topToMid.GetEndPoint().x < topToBot.GetEndPoint().x)
		_triangle.triangle_type_ = TriangleType::Left;
	else
	{
		_triangle.triangle_type_ = TriangleType::Right;
		left_x = static_cast<float>(topToBot.GetStartPoint().x);
		right_x = static_cast<float>(topToMid.GetStartPoint().x);
		left_slope = topToBot.GetSlope();
		right_slope = topToMid.GetSlope();
	}


	float left_step = -1.0f / left_slope;
	float right_step = -1.0f / right_slope;

	if (std::isinf(left_step) || std::isnan(left_step))
		left_step = 0;
	if (std::isinf(right_step) || std::isnan(right_step))
		right_step = 0;

	// Top half
	while (y > _triangle.middle_.y)
	{
		_image.SetPixel(left_x, y, _color);
		_image.SetPixel(right_x, y, _color);

		left_x += left_step;
		right_x += right_step;

		y--;
	}



	if (_triangle.triangle_type_ == TriangleType::Left)
	{
		left_x = static_cast<float>(midToBot.GetStartPoint().x);
		left_slope = midToBot.GetSlope();
		left_step = -1.0f / left_slope;
	}
	else
	{
		right_x = static_cast<float>(midToBot.GetStartPoint().x);
		right_slope = midToBot.GetSlope();
		right_step = -1.0f / right_slope;
	}

	if (std::isinf(left_step) || std::isnan(left_step))
		left_step = 0;
	if (std::isinf(right_step) || std::isnan(right_step))
		right_step = 0;


	// Bottom half
	while (y >= _triangle.bottom_.y)
	{
		_image.SetPixel(left_x, y, _color);
		_image.SetPixel(right_x, y, _color);

		left_x += left_step;
		right_x += right_step;

		y--;
	}
}