#pragma once
#pragma pack(push,1) // for tight pack for struct.

struct Color
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;

	Color();
	Color(int _red, int _green, int _blue, int _alpha = 255);

	bool operator==(Color _input);
	bool operator!=(Color _input);
};

#pragma pack(pop)