#pragma once
#include "../Math/Vector.h"
#include "../ImageFormats/TGAImage.h"


class Triangle
{
	Vec3f top_;
	Vec3f middle_;
	Vec3f bottom_;
	Vec3f* left_ = nullptr;
	Vec3f* right_ = nullptr;
	Vec3f face_normal_;
	
	enum TriangleType
	{
		Left,
		Right
	};
	TriangleType triangle_type_;

	void IdentifyTopMiddleBottom(Vec3f const& _a, Vec3f const& _b, Vec3f const& _c);
	void IdentifyLeftRight();

	

public:
	Triangle(Vec2i const& _a, Vec2i const& _b, Vec2i const& _c);
	Triangle(Vec3f const& _a, Vec3f const& _b, Vec3f const& _c);
	Triangle(const Triangle& _triangle);

	Vec3f GetLeft();
	Vec3f GetRight();
	Vec3f GetTop();
	Vec3f GetMiddle();
	Vec3f GetNormal();

	void SetNormal(Vec3f& _normal);
	void SetNormal(Vec3f _normal);

	static void TriangleRasterize(TGAImage& _image, Triangle& _triangle, Color& _color);
	static void TriangleOutlineRasterize(TGAImage& _image, Triangle& _triangle, Color& _color);
};