#include "Color.hpp"

//created by Trey Spears

const float INVERSE_255 = 1.f / 255.f;
std::map<uchar, Colors> Color::colorMap;
int Color::NUMBER_OF_COLORS = 138;

//-----------------------------------------------------------------------------------------------
Color::Color()
{
	r = 0xFF;
	g = 0xFF;
	b = 0xFF;
	a = 0XFF;
}

//-----------------------------------------------------------------------------------------------
Color::Color(Colors color)
{
	FromName(color);
}

//-----------------------------------------------------------------------------------------------
Color::Color(uint color)
{
	FromRGBA(color);
}

//-----------------------------------------------------------------------------------------------
Color::Color(uchar ID)
{
	FromID(ID);
}

//-----------------------------------------------------------------------------------------------
void Color::Initialize()
{
	colorMap[0] = Red;		
	colorMap[1] =	Green			;
	colorMap[2] =	Blue			;
	colorMap[3] =	White			;
	colorMap[4] =	Black			;
	colorMap[5] =	Yellow			;
	colorMap[6] =	Magenta			;
	colorMap[7] =	Cyan			;
	colorMap[8] =	AliceBlue		;
	colorMap[9] =	AntiqueWhite	;
	colorMap[10] =	Aquamarine		;
	colorMap[11] =	Azure			;
	colorMap[12] =	Beige			;
	colorMap[13] =	Bisque			;
	colorMap[14] =	BlanchedAlmond	;
	colorMap[15] =	BlueViolet		;
	colorMap[16] =	Brown			;
	colorMap[17] =	BurlyWood		;
	colorMap[18] =	CadetBlue		;
	colorMap[19] =	Chartreuse		;
	colorMap[20] =	Chocolate		;
	colorMap[21] =	Coral			;
	colorMap[22] =	CornflowerBlue	;
	colorMap[23] =	Cornsilk		;
	colorMap[24] =	Crimson			;
	colorMap[25] =	DarkBlue		;
	colorMap[26] =	DarkCyan		;
	colorMap[27] =	DarkGoldenrod	;
	colorMap[28] =	DarkGray		;
	colorMap[29] =	DarkGreen		;
	colorMap[30] =	DarkKhaki		;
	colorMap[31] =	DarkMagenta		;
	colorMap[32] =	DarkOliveGreen	;
	colorMap[33] =	DarkOrange		;
	colorMap[34] =	DarkOrchid		;
	colorMap[35] =	DarkRed			;
	colorMap[36] =	DarkSalmon		;
	colorMap[37] =	DarkSeaGreen	;
	colorMap[38] =	DarkSlateBlue	;
	colorMap[39] =	DarkSlateGray	;
	colorMap[40] =	DarkTurquoise	;
	colorMap[41] =	DarkViolet		;
	colorMap[42] =	DeepPink		;
	colorMap[43] =	DeepSkyBlue		;
	colorMap[44] =	DimGray			;
	colorMap[45] =	DodgerBlue		;
	colorMap[46] =	FireBrick		;
	colorMap[47] =	FloralWhite		;
	colorMap[48] =	ForestGreen		;
	colorMap[49] =	Gainsboro		;
	colorMap[50] =	GhostWhite		;
	colorMap[51] =	Gold			;
	colorMap[52] =	Goldenrod		;
	colorMap[53] =	Gray			;
	colorMap[54] =	TrueGreen		;
	colorMap[55] =	GreenYellow		;
	colorMap[56] =	Honeydew		;
	colorMap[57] =	HotPink			;
	colorMap[58] =	IndianRed		;
	colorMap[59] =	Indigo			;
	colorMap[60] =	Ivory			;
	colorMap[61] =	Khaki			;
	colorMap[62] =	Lavender		;
	colorMap[63] =	LavenderBlush	;
	colorMap[64] =	LawnGreen		;
	colorMap[65] =	LemonChiffon	;
	colorMap[66] =	LightBlue		;
	colorMap[67] =	LightCoral		;
	colorMap[68] =	LightCyan		;
	colorMap[69] =	LightGoldenrodYellow;
	colorMap[70] =	LightGreen		;
	colorMap[71] =	LightGray		;
	colorMap[72] =	LightPink		;
	colorMap[73] =	LightSalmon		;
	colorMap[74] =	LightSeaGreen	;
	colorMap[75] =	LightSkyBlue	;
	colorMap[76] =	LightSlateGray	;
	colorMap[77] =	LightSteelBlue	;
	colorMap[78] =	LightYellow		;
	colorMap[79] =	LimeGreen		;
	colorMap[80] =	Linen			;
	colorMap[81] =	Maroon			;
	colorMap[82] =	MediumAquamarine;
	colorMap[83] =	MediumBlue		;
	colorMap[84] =	MediumOrchid	;
	colorMap[85] =	MediumPurple	;
	colorMap[86] =	MediumSeaGreen	;
	colorMap[87] =	MediumSlateBlue	;
	colorMap[88] =	MediumSpringGreen;
	colorMap[89] =	MediumTurquoise	;
	colorMap[90] =	MediumVioletRed	;
	colorMap[91] =	MidnightBlue	;
	colorMap[92] =	MintCream		;
	colorMap[93] =	MistyRose		;
	colorMap[94] =	Moccasin		;
	colorMap[95] =	NavajoWhite		;
	colorMap[96] =	Navy			;
	colorMap[97] =	OldLace			;
	colorMap[98] =	Olive			;
	colorMap[99] =	OliveDrab		;
	colorMap[100] =	Orange			;
	colorMap[101] =	OrangeRed		;
	colorMap[102] =	Orchid			;
	colorMap[103] =	PaleGoldenrod	;
	colorMap[104] =	PaleGreen		;
	colorMap[105] =	PaleTurquoise	;
	colorMap[106] =	PaleVioletRed	;
	colorMap[107] =	PapayaWhip		;
	colorMap[108] =	PeachPuff		;
	colorMap[109] =	Peru			;
	colorMap[110] =	Pink			;
	colorMap[111] =	Plum			;
	colorMap[112] =	PowderBlue		;
	colorMap[113] =	Purple			;
	colorMap[114] =	RosyBrown		;
	colorMap[115] =	RoyalBlue		;
	colorMap[116] =	SaddleBrown		;
	colorMap[117] =	Salmon			;
	colorMap[118] =	SandyBrown		;
	colorMap[119] =	SeaGreen		;
	colorMap[120] =	SeaShell		;
	colorMap[121] =	Sienna			;
	colorMap[122] =	Silver			;
	colorMap[123] =	SkyBlue			;
	colorMap[124] =	SlateBlue		;
	colorMap[125] =	SlateGray		;
	colorMap[126] =	Snow			;
	colorMap[127] =	SpringGreen		;
	colorMap[128] =	SteelBlue		;
	colorMap[129] =	Tan				;
	colorMap[130] =	Teal			;
	colorMap[131] =	Thistle			;
	colorMap[132] =	Tomato			;
	colorMap[133] =	Turquoise		;
	colorMap[134] =	Violet			;
	colorMap[135] =	Wheat			;
	colorMap[136] =	WhiteSmoke		;
	colorMap[137] =	YellowGreen		;
}

//-----------------------------------------------------------------------------------------------
void Color::FromName(Colors color)
{
	r = (uchar)(color >> 24);
	g = (uchar)(color >> 16);
	b = (uchar)(color >> 8);
	a = (uchar)color;
}

//-----------------------------------------------------------------------------------------------
void Color::FromRGBA(uint color)
{
	r = (uchar)(color >> 24);
	g = (uchar)(color >> 16);
	b = (uchar)(color >> 8);
	a = (uchar)color;
}

//-----------------------------------------------------------------------------------------------
void Color::FromID(uchar ID)
{
	Colors color = colorMap[ID];

	r = (uchar)(color >> 24);
	g = (uchar)(color >> 16);
	b = (uchar)(color >> 8);
	a = (uchar)color;
}

//-----------------------------------------------------------------------------------------------
void Color::FromVector3fNormalizedAndFullyOpaque(const Vector3f& colorNormalized)
{
	r = (uchar)(colorNormalized.x * 255.f);
	g = (uchar)(colorNormalized.y * 255.f);
	b = (uchar)(colorNormalized.z * 255.f);
	a = (uchar)1;
}

//-----------------------------------------------------------------------------------------------
uint Color::ToRGBA()
{
	uint color = (r << 24) | (g << 16) | (b << 8) | a;

	return color;
}

//-----------------------------------------------------------------------------------------------
Vector3f Color::ToVector3fAndFullyOpaque() const
{
	Vector3f color;

	color.x = (float)r;
	color.y = (float)g;
	color.z = (float)b;

	return color;
}

//-----------------------------------------------------------------------------------------------
Vector3f Color::ToVector3fNormalizedAndFullyOpaque() const
{
	Vector3f color;

	color.x = (float)r * INVERSE_255;
	color.y = (float)g * INVERSE_255;
	color.z = (float)b * INVERSE_255;

	return color;
}

//-----------------------------------------------------------------------------------------------
Vector4f Color::ToVector4fNormalized() const
{
	Vector4f color;

	color.x = (float)r * INVERSE_255;
	color.y = (float)g * INVERSE_255;
	color.z = (float)b * INVERSE_255;
	color.w = (float)a * INVERSE_255;

	return color;
}

//-----------------------------------------------------------------------------------------------
Vector4f Color::ToVector4f() const
{
	Vector4f color;

	color.x = (float)r * INVERSE_255;
	color.y = (float)g * INVERSE_255;
	color.z = (float)b * INVERSE_255;
	color.w = (float)a * INVERSE_255;

	return color;
}

//-----------------------------------------------------------------------------------------------
Color Color::RandomColor()
{
	Color color;
	uchar ID = (uchar)(rand() % Color::NUMBER_OF_COLORS);
	color.FromID(ID);

	return color;
}

