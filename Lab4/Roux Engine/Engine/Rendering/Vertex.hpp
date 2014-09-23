#ifndef VERTEX
#define VERTEX

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Vector4.hpp"

struct VertexPositionNTBColorTexture
{
	VertexPositionNTBColorTexture()
	{

	}
	VertexPositionNTBColorTexture(const Vector3f& pos, const Vector3f& n,const Vector3f& t, const Vector3f& b, const Vector4f& color, const Vector2f& texCoords)
		: position(pos)
		, normal(n)
		, tangent(t)
		, bitangent(b)
		, diffuseColor(color)
		, textureCoordinates(texCoords)
	{

	}

	Vector3f position;
	Vector3f normal;
	Vector3f tangent;
	Vector3f bitangent;
	Vector4f diffuseColor;
	Vector2f textureCoordinates;

};

struct VertexPositionNormalColorTexture
{
	VertexPositionNormalColorTexture()
	{

	}
	VertexPositionNormalColorTexture(const Vector3f& pos, const Vector3f& n, const Vector4f& color, const Vector2f& texCoords)
		: position(pos)
		, normal(n)
		, diffuseColor(color)
		, textureCoordinates(texCoords)
	{

	}

	Vector3f position;
	Vector3f normal;
	Vector4f diffuseColor;
	Vector2f textureCoordinates;

};

struct VertexPositionColorTexture
{
	VertexPositionColorTexture()
	{

	}
	VertexPositionColorTexture(const Vector3f& pos, const Vector4f& color, const Vector2f& texCoords)
		: position(pos)
		, diffuseColor(color)
		, textureCoordinates(texCoords)
	{

	}

	Vector3f position;
	Vector4f diffuseColor;
	Vector2f textureCoordinates;

};

struct VertexPositionColor
{
	VertexPositionColor()
	{

	}
	VertexPositionColor(const Vector3f& pos, const Vector4f& color)
		: position(pos)
		, diffuseColor(color)
	{

	}

	Vector3f position;
	Vector4f diffuseColor;
};
#endif