#ifndef COLOR
#define COLOR

#include <map>

#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Vector4.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

//created by Trey Spears

//go to http://msdn.microsoft.com/en-us/library/aa358802.aspx for reference

enum Colors {

	Red				= 0xFF0000FF,
	Green			= 0x00FF00FF,
	Blue			= 0x0000FFFF,
	Yellow			= 0xFFFF00FF,
	Magenta			= 0xFF00FFFF,
	Cyan			= 0x00FFFFFF,
	AliceBlue		= 0xF0F8FFFF,
	AntiqueWhite	= 0xFAEBD7FF,
	Aquamarine		= 0x7FFFD4FF,
	Azure			= 0xF0FFFFFF,
	Beige			= 0xF5F5DCFF,
	Bisque			= 0xFFE4C4FF,
	BlanchedAlmond	= 0xFFEBCDFF,
	BlueViolet		= 0x8A2BE2FF,
	Brown			= 0xA52A2AFF,
	BurlyWood		= 0xDEB887FF,
	CadetBlue		= 0x5F9EA0FF,
	Chartreuse		= 0x7FFF00FF,
	Chocolate		= 0xD2691EFF,
	Coral			= 0xFF7F50FF,
	CornflowerBlue	= 0x6495EDFF,
	Cornsilk		= 0xFFF8DCFF,
	Crimson			= 0xDC143CFF,
	DarkBlue		= 0x00008BFF,
	DarkCyan		= 0x008B8BFF,
	DarkGoldenrod	= 0xB8860BFF,
	DarkGray		= 0xA9A9A9FF,
	DarkGreen		= 0x006400FF,
	DarkKhaki		= 0xBDB76BFF,
	DarkMagenta		= 0x8B008BFF,
	DarkOliveGreen	= 0x556B2FFF,
	DarkOrange		= 0xFF8C00FF,
	DarkOrchid		= 0x9932CCFF,
	DarkRed			= 0x8B0000FF,
	DarkSalmon		= 0xE9967AFF,
	DarkSeaGreen	= 0x8FBC8FFF,
	DarkSlateBlue	= 0x483D8BFF,
	DarkSlateGray	= 0x2F4F4FFF,
	DarkTurquoise	= 0x00CED1FF,
	DarkViolet		= 0x9400D3FF,
	DeepPink		= 0xFF1493FF,
	DeepSkyBlue		= 0x00BFFFFF,
	DimGray			= 0x696969FF,
	DodgerBlue		= 0x1E90FFFF,
	FireBrick		= 0xB22222FF,
	FloralWhite		= 0xFFFAF0FF,
	ForestGreen		= 0x228B22FF,
	Gainsboro		= 0xDCDCDCFF,
	GhostWhite		= 0xF8F8FFFF,
	Gold			= 0xFFD700FF,
	Goldenrod		= 0xDAA520FF,
	Gray			= 0x808080FF,
	TrueGreen		= 0x008000FF,
	GreenYellow		= 0xADFF2FFF,
	Honeydew		= 0xF0FFF0FF,
	HotPink			= 0xFF69B4FF,
	IndianRed		= 0xCD5C5CFF,
	Indigo			= 0x4B0082FF,
	Ivory			= 0xFFFFF0FF,
	Khaki			= 0xF0E68CFF,
	Lavender		= 0xE6E6FAFF,
	LavenderBlush	= 0xFFF0F5FF,
	LawnGreen		= 0x7CFC00FF,
	LemonChiffon	= 0xFFFACDFF,
	LightBlue		= 0xADD8E6FF,
	LightCoral		= 0xF08080FF,
	LightCyan		= 0xE0FFFFFF,
	LightGoldenrodYellow = 0xFAFAD2FF,
	LightGreen		= 0x90EE90FF,
	LightGray		= 0xD3D3D3FF,
	LightPink		= 0xFFB6C1FF,
	LightSalmon		= 0xFFA07AFF,
	LightSeaGreen	= 0x20B2AAFF,
	LightSkyBlue	= 0x87CEFAFF,
	LightSlateGray	= 0x778899FF,
	LightSteelBlue	= 0xB0C4DEFF,
	LightYellow		= 0xFFFFE0FF,
	LimeGreen		= 0x32CD32FF,
	Linen			= 0xFAF0E6FF,
	Maroon			= 0x800000FF,
	MediumAquamarine = 0x66CDAAFF,
	MediumBlue		= 0x0000CDFF,
	MediumOrchid	= 0xBA55D3FF,
	MediumPurple	= 0x9370DBFF,
	MediumSeaGreen	= 0x3CB371FF,
	MediumSlateBlue	= 0x7B68EEFF,
	MediumSpringGreen = 0x00FA9AFF,
	MediumTurquoise	= 0x48D1CCFF,
	MediumVioletRed	= 0xC71585FF,
	MidnightBlue	= 0x191970FF,
	MintCream		= 0xF5FFFAFF,
	MistyRose		= 0xFFE4E1FF,
	Moccasin		= 0xFFE4B5FF,
	NavajoWhite		= 0xFFDEADFF,
	Navy			= 0x000080FF,
	OldLace			= 0xFDF5E6FF,
	Olive			= 0x808000FF,
	OliveDrab		= 0x6B8E23FF,
	Orange			= 0xFFA500FF,
	OrangeRed		= 0xFF4500FF,
	Orchid			= 0xDA70D6FF,
	PaleGoldenrod	= 0xEEE8AAFF,
	PaleGreen		= 0x98FB98FF,
	PaleTurquoise	= 0xAFEEEEFF,
	PaleVioletRed	= 0xDB7093FF,
	PapayaWhip		= 0xFFEFD5FF,
	PeachPuff		= 0xFFDAB9FF,
	Peru			= 0xCD853FFF,
	Pink			= 0xFFC0CBFF,
	Plum			= 0xDDA0DDFF,
	PowderBlue		= 0xB0E0E6FF,
	Purple			= 0x800080FF,
	RosyBrown		= 0xBC8F8FFF,
	RoyalBlue		= 0x4169E1FF,
	SaddleBrown		= 0x8B4513FF,
	Salmon			= 0xFA8072FF,
	SandyBrown		= 0xF4A460FF,
	SeaGreen		= 0x2E8B57FF,
	SeaShell		= 0xFFF5EEFF,
	Sienna			= 0xA0522DFF,
	Silver			= 0xC0C0C0FF,
	SkyBlue			= 0x87CEEBFF,
	SlateBlue		= 0x6A5ACDFF,
	SlateGray		= 0x708090FF,
	Snow			= 0xFFFAFAFF,
	SpringGreen		= 0x00FF7FFF,
	SteelBlue		= 0x4682B4FF,
	Tan				= 0xD2B48CFF,
	Teal			= 0x008080FF,
	Thistle			= 0xD8BFD8FF,
	Tomato			= 0xFF6347FF,
	Turquoise		= 0x40E0D0FF,
	Violet			= 0xEE82EEFF,
	Wheat			= 0xF5DEB3FF,
	WhiteSmoke		= 0xF5F5F5FF,
	YellowGreen		= 0x9ACD32FF,

	White			= 0xFFFFFFFF,
	Black			= 0X000000FF,
};

enum ColorID
{
	RedID				,
	GreenID			,
	BlueID			,
	YellowID			,
	MagentaID			,
	CyanID			,
	AliceBlueID		,
	AntiqueWhiteID	,
	AquamarineID		,
	AzureID			,
	BeigeID			,
	BisqueID			,
	BlanchedAlmondID	,
	BlueVioletID		,
	BrownID			,
	BurlyWoodID		,
	CadetBlueID		,
	ChartreuseID		,
	ChocolateID		,
	CoralID			,
	CornflowerBlueID	,
	CornsilkID		,
	CrimsonID			,
	DarkBlueID		,
	DarkCyanID		,
	DarkGoldenrodID	,
	DarkGrayID		,
	DarkGreenID		,
	DarkKhakiID		,
	DarkMagentaID		,
	DarkOliveGreenID	,
	DarkOrangeID		,
	DarkOrchidID		,
	DarkRedID			,
	DarkSalmonID		,
	DarkSeaGreenID	,
	DarkSlateBlueID	,
	DarkSlateGrayID	,
	DarkTurquoiseID	,
	DarkVioletID		,
	DeepPinkID		,
	DeepSkyBlueID		,
	DimGrayID		,
	DodgerBlueID		,
	FireBrickID		,
	FloralWhiteID		,
	ForestGreenID		,
	GainsboroID		,
	GhostWhiteID		,
	GoldID			,
	GoldenrodID		,
	GrayID			,
	TrueGreenID		,
	GreenYellowID		,
	HoneydewID		,
	HotPinkID			,
	IndianRedID		,
	IndigoID			,
	IvoryID			,
	KhakiID			,
	LavenderID		,
	LavenderBlushID	,
	LawnGreenID		,
	LemonChiffonID	,
	LightBlueID		,
	LightCoralID		,
	LightCyanID		,
	LightGoldenrodYellowID,
	LightGreenID		,
	LightGrayID		,
	LightPinkID		,
	LightSalmonID		,
	LightSeaGreenID	,
	LightSkyBlueID	,
	LightSlateGrayID	,
	LightSteelBlueID	,
	LightYellowID		,
	LimeGreenID		,
	LinenID			,
	MaroonID			,
	MediumAquamarineID,
	MediumBlueID		,
	MediumOrchidID	,
	MediumPurpleID	,
	MediumSeaGreenID	,
	MediumSlateBlueID	,
	MediumSpringGreenID,
	MediumTurquoiseID	,
	MediumVioletRedID	,
	MidnightBlueID	,
	MintCreamID		,
	MistyRoseID		,
	MoccasinID		,
	NavajoWhiteID		,
	NavyID			,
	OldLaceID			,
	OliveID			,
	OliveDrabID		,
	OrangeID			,
	OrangeRedID		,
	OrchidID			,
	PaleGoldenrodID	,
	PaleGreenID		,
	PaleTurquoiseID	,
	PaleVioletRedID	,
	PapayaWhipID		,
	PeachPuffID		,
	PeruID			,
	PinkID			,
	PlumID			,
	PowderBlueID		,
	PurpleID			,
	RosyBrownID		,
	RoyalBlueID		,
	SaddleBrownID		,
	SalmonID			,
	SandyBrownID		,
	SeaGreenID		,
	SeaShellID		,
	SiennaID			,
	SilverID			,
	SkyBlueID			,
	SlateBlueID		,
	SlateGrayID		,
	SnowID			,
	SpringGreenID		,
	SteelBlueID		,
	TanID				,
	TealID			,
	ThistleID			,
	TomatoID			,
	TurquoiseID		,
	VioletID			,
	WheatID			,
	WhiteSmokeID		,
	YellowGreenID	,

	WhiteID			,
	BlackID			,

	NUM_COLORS
};

class Color
{

public:

	Color();
	Color(Colors color);
	Color(uint color);
	Color(uchar ID);

	static void Initialize();

	void FromName(Colors color);
	void FromRGBA(uint color);
	void FromID(uchar color);
	void FromVector3fNormalizedAndFullyOpaque(const Vector3f& colorNormalized);
	static Color RandomColor();
	inline bool operator <(const Color &rhs) const;
	inline bool operator ==(const Color &rhs) const;

	uint ToRGBA();

	Vector3f ToVector3fNormalizedAndFullyOpaque() const;
	Vector3f ToVector3fAndFullyOpaque() const;
	Vector4f ToVector4fNormalized() const;
	Vector4f ToVector4f() const;

	uchar r;
	uchar g;
	uchar b;
	uchar a;

	static int NUMBER_OF_COLORS;

private:
	static std::map<uchar,Colors> colorMap;
};

//-----------------------------------------------------------------------------------------------
inline bool Color::operator <(const Color &rhs) const
{
	float rHash = r * 18353.f;
	float rhsRHash = rhs.r * 18353.f;

	float gHash = g * 4021.f;
	float rhsGHash = rhs.g * 4021.f;

	float bHash = b * 367.f;
	float rhsBHash = rhs.b * 367.f;

	float myHashSum = rHash + gHash + bHash + a;
	float rhsHashSum = rhsRHash + rhsGHash  + rhsBHash + rhs.a;

	if(myHashSum < rhsHashSum)
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------
inline bool Color::operator ==(const Color &rhs) const
{
	if(r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a)
		return true;
	return false;
}


#endif