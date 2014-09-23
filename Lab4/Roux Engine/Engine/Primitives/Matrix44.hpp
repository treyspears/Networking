#ifndef MATRIX44
#define MATRIX44

#include <math.h>
#include <memory.h>
#include <vector>
#include <cassert>

#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

template <class T>
class Matrix44
{

public:

	T data[16];

	Matrix44<T>()
	{
		memset(data, 0, sizeof(data));
		data[0] = data[5] = data [10] = data[15] = 1;
	}

	inline T& operator()(unsigned int row, unsigned int col) const;
	inline Matrix44<T> operator * (const T& scalar) const;
	inline Matrix44<T>& operator *=(const T& scalar) const;
	static Matrix44<T> MultMatrix (const Matrix44<T>& lhs, const Matrix44<T>& rhs);
	static Matrix44<T> AddMatrix (const Matrix44<T>& lhs, const Matrix44<T>& rhs);
	static Matrix44<T> GetIdentity ();
	inline Matrix44<T> GetTranspose() const;

	static Matrix44<T> GetZRotationMatrix(float angleInDegrees);
	static Matrix44<T> GetYRotationMatrix(float angleInDegrees);
	static Matrix44<T> GetXRotationMatrix(float angleInDegrees);
};

typedef Matrix44<float> Matrix44f;
typedef Matrix44<int> Matrix44i;
typedef Matrix44<double> Matrix44d;

class MatrixStack44f
{
public:

	MatrixStack44f()
	{
		m_modelStack.push_back(Matrix44f::GetIdentity());
		m_viewStack.push_back(Matrix44f::GetIdentity());
	}

	enum StackTarget
	{
		MODEL_STACK,
		VIEW_STACK
	};

	inline Matrix44f GetProjection();
	inline Matrix44f GetModelViewMatrix();
	inline Matrix44f GetMVWithProjection();
	inline Matrix44f GetCurrent(StackTarget target);
	inline const Matrix44f GetCurrent(StackTarget target) const;

	inline void Push(StackTarget target);
	inline void Pop(StackTarget target);
	inline void MakeIdentity(StackTarget target);
	inline void LoadMatrix(StackTarget target, const Matrix44f& m);
	inline void ApplyOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
	inline void ApplyPerspective(float fovy, float aspect, float zNear, float zFar);
	inline void ApplyYawDegreesAboutZRotation(StackTarget target, float angleInDegrees);
	inline void ApplyPitchDegreesAboutYRotation(StackTarget target, float angleInDegrees);
	inline void ApplyRollDegreesAboutXRotation(StackTarget target, float angleInDegrees);
	inline void ApplyTranslation(StackTarget target, const Vector3f& angleInDegrees);
	inline void ApplyTranslation(StackTarget target, const Vector2f& translation);
	inline void ApplyTranslation(StackTarget target, float x, float y, float z);
	inline void ApplyUniformScale(StackTarget target, float scale);
	inline void ApplyNonUniformScale(StackTarget target, float scale, float x, float y, float z);

private:
	Matrix44f m_projectionMatrix;
	std::vector<Matrix44f> m_viewStack;
	std::vector<Matrix44f> m_modelStack;
};

template <class T>
inline T& Matrix44<T>::operator()(unsigned int row, unsigned int col) const
{
	return data[col + 4 * row];
}

template <class T>
inline Matrix44<T> Matrix44<T>::operator*(const T& scalar) const
{
	for( int i = 0; i < 16; ++ i)
	{
		data[i] *= scalar;
	}

	return *this;
}

template <class T>
inline Matrix44<T>& Matrix44<T>::operator*=(const T& scalar) const
{
	for( int i = 0; i < 16; ++ i)
	{
		data[i] *= scalar;
	}

	return *this;
}

template <class T>
Matrix44<T> Matrix44<T>::MultMatrix(const Matrix44<T>& lhs, const Matrix44<T>& rhs)
{
	Matrix44<T> result;

	result.data[0] = lhs.data[0]*rhs.data[0] + lhs.data[1]*rhs.data[4] + lhs.data[2]*rhs.data[8] + lhs.data[3]*rhs.data[12];
	result.data[1] = lhs.data[0]*rhs.data[1] + lhs.data[1]*rhs.data[5] + lhs.data[2]*rhs.data[9] + lhs.data[3]*rhs.data[13];
	result.data[2] = lhs.data[0]*rhs.data[2] + lhs.data[1]*rhs.data[6] + lhs.data[2]*rhs.data[10] + lhs.data[3]*rhs.data[14];
	result.data[3] = lhs.data[0]*rhs.data[3] + lhs.data[1]*rhs.data[7] + lhs.data[2]*rhs.data[11] + lhs.data[3]*rhs.data[15];

	result.data[4] = lhs.data[4]*rhs.data[0] + lhs.data[5]*rhs.data[4] + lhs.data[6]*rhs.data[8] + lhs.data[7]*rhs.data[12];
	result.data[5] = lhs.data[4]*rhs.data[1] + lhs.data[5]*rhs.data[5] + lhs.data[6]*rhs.data[9] + lhs.data[7]*rhs.data[13];
	result.data[6] = lhs.data[4]*rhs.data[2] + lhs.data[5]*rhs.data[6] + lhs.data[6]*rhs.data[10] + lhs.data[7]*rhs.data[14];
	result.data[7] = lhs.data[4]*rhs.data[3] + lhs.data[5]*rhs.data[7] + lhs.data[6]*rhs.data[11] + lhs.data[7]*rhs.data[15];

	result.data[8] = lhs.data[8]*rhs.data[0] + lhs.data[9]*rhs.data[4] + lhs.data[10]*rhs.data[8] + lhs.data[11]*rhs.data[12];
	result.data[9] = lhs.data[8]*rhs.data[1] + lhs.data[9]*rhs.data[5] + lhs.data[10]*rhs.data[9] + lhs.data[11]*rhs.data[13];
	result.data[10] = lhs.data[8]*rhs.data[2] + lhs.data[9]*rhs.data[6] + lhs.data[10]*rhs.data[10] + lhs.data[11]*rhs.data[14]; 
	result.data[11] = lhs.data[8]*rhs.data[3] + lhs.data[9]*rhs.data[7] + lhs.data[10]*rhs.data[11] + lhs.data[11]*rhs.data[15]; 

	result.data[12] = lhs.data[12]*rhs.data[0] + lhs.data[13]*rhs.data[4] + lhs.data[14]*rhs.data[8] + lhs.data[15]*rhs.data[12]; 
	result.data[13] = lhs.data[12]*rhs.data[1] + lhs.data[13]*rhs.data[5] + lhs.data[14]*rhs.data[9] + lhs.data[15]*rhs.data[13]; 
	result.data[14] = lhs.data[12]*rhs.data[2] + lhs.data[13]*rhs.data[6] + lhs.data[14]*rhs.data[10] + lhs.data[15]*rhs.data[14]; 
	result.data[15] = lhs.data[12]*rhs.data[3] + lhs.data[13]*rhs.data[7] + lhs.data[14]*rhs.data[11] + lhs.data[15]*rhs.data[15];

	return result;
}

template <class T>
Matrix44<T> Matrix44<T>::AddMatrix (const Matrix44<T>& lhs, const Matrix44<T>& rhs)
{
	Matrix44<T> result;

	for( int i = 0; i < 16; ++ i)
	{
		result.data[i] = lhs.data[i] + rhs.data[i];
	}

	return result;
}

template <class T>
Matrix44<T> Matrix44<T>::GetIdentity()
{
	Matrix44<T> identity;
	return identity;
}

template <class T>
inline Matrix44<T> Matrix44<T>::GetTranspose() const
{
	Matrix44<T> transpose = *this;

	std::swap(transpose.data[1], transpose.data[4] );
	std::swap( transpose.data[2], transpose.data[8] );
	std::swap( transpose.data[3], transpose.data[12] );
	std::swap( transpose.data[6], transpose.data[9] );
	std::swap( transpose.data[7], transpose.data[13] );
	std::swap( transpose.data[11], transpose.data[14] );

	return transpose;
}

template <class T>
Matrix44<T> Matrix44<T>::GetZRotationMatrix(float angleInDegrees)
{
	Matrix44<T> rotationAboutZMatrix;

	float angleInRadians = ConvertDegreesToRadians(angleInDegrees);

	rotationAboutZMatrix.data[0] = (T)cos(angleInRadians);
	rotationAboutZMatrix.data[1] = (T)sin(angleInRadians);
	rotationAboutZMatrix.data[4] = (T)-sin(angleInRadians);
	rotationAboutZMatrix.data[5] = (T)cos(angleInRadians);

	return rotationAboutZMatrix;
}
template <class T>
Matrix44<T> Matrix44<T>::GetYRotationMatrix(float angleInDegrees)
{
	Matrix44<T> rotationAboutYMatrix;

	float angleInRadians = ConvertDegreesToRadians(angleInDegrees);

	rotationAboutYMatrix.data[0] = (T)cos(angleInRadians);
	rotationAboutYMatrix.data[2] = (T)-sin(angleInRadians);
	rotationAboutYMatrix.data[8] = (T)sin(angleInRadians);
	rotationAboutYMatrix.data[10] = (T)cos(angleInRadians);

	return rotationAboutYMatrix;
}

template <class T>
Matrix44<T> Matrix44<T>::GetXRotationMatrix(float angleInDegrees)
{
	Matrix44<T> rotationAboutXMatrix;

	float angleInRadians = ConvertDegreesToRadians(angleInDegrees);

	rotationAboutXMatrix.data[5] = (T)cos(angleInRadians);
	rotationAboutXMatrix.data[6] = (T)sin(angleInRadians);
	rotationAboutXMatrix.data[9] = (T)-sin(angleInRadians);
	rotationAboutXMatrix.data[10] = (T)cos(angleInRadians);

	return rotationAboutXMatrix;
}

//Matrix Stack Definitions
//-----------------------------------------------------------------------------------------------

Matrix44f MatrixStack44f::GetProjection()
{
	return m_projectionMatrix;
}

Matrix44f MatrixStack44f::GetModelViewMatrix()
{
	return Matrix44f::MultMatrix(m_modelStack.back(), m_viewStack.back());
}

Matrix44f MatrixStack44f::GetMVWithProjection()
{
	return Matrix44f::MultMatrix(GetModelViewMatrix(), m_projectionMatrix); 
}

Matrix44f MatrixStack44f::GetCurrent(StackTarget target)
{
	if(target == MODEL_STACK)
		return m_modelStack.back();
	else
		return m_viewStack.back();
}

void MatrixStack44f::Push(StackTarget target)
{
	if(target == MODEL_STACK)
		m_modelStack.push_back(GetCurrent(target));	
	else
		m_viewStack.push_back(GetCurrent(target));
}

const Matrix44f MatrixStack44f::GetCurrent(StackTarget target) const
{
	if(target == MODEL_STACK)
		return m_modelStack.back();
	else
		return m_viewStack.back();
}

void MatrixStack44f::MakeIdentity(StackTarget target)
{
	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::GetIdentity();
	else
		m_viewStack.back() = Matrix44f::GetIdentity();
}

void MatrixStack44f::LoadMatrix(StackTarget target, const Matrix44f& m)
{
	if(target == MODEL_STACK)
		m_modelStack.back() = m;
	else
		m_viewStack.back() = m;
}


void MatrixStack44f::Pop(StackTarget target)
{
	if(target == MODEL_STACK)
	{
		m_modelStack.pop_back();
		assert(m_modelStack.size() > 0);
	}
	else
	{
		m_viewStack.pop_back();
		assert(m_viewStack.size() > 0);
	}
}

void MatrixStack44f::ApplyOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Matrix44f orthoMatrix;

	orthoMatrix.data[0] = 2.f / (right - left);
	orthoMatrix.data[5] = 2.f / (top - bottom);
	orthoMatrix.data[10] = -2.f / (zFar - zNear);
	orthoMatrix.data[12] = - (right + left) / (right - left);
	orthoMatrix.data[13] = -(top + bottom) / (top - bottom);
	orthoMatrix.data[14] = -(zFar + zNear) / (zFar - zNear);

	m_projectionMatrix = orthoMatrix;
	//m_stack.back() = Matrix44f::MultMatrix(orthoMatrix, m_stack.back());
}
void MatrixStack44f::ApplyPerspective(float fovy, float aspect, float zNear, float zFar)
{
	Matrix44f perspectiveMatrix;

	float cot_fovy_over_2 = 1.f / (float)tan(ConvertDegreesToRadians(fovy) / 2.f);

	perspectiveMatrix.data[0] = cot_fovy_over_2 / aspect;
	perspectiveMatrix.data[5] = cot_fovy_over_2;
	perspectiveMatrix.data[10] = (zFar + zNear) / (zNear - zFar);
	perspectiveMatrix.data[11] = -1.f;
	perspectiveMatrix.data[14] = (2.f * zFar * zNear) / (zNear - zFar);

	m_projectionMatrix = perspectiveMatrix;
	//m_stack.back() = Matrix44f::MultMatrix(perspectiveMatrix, m_stack.back());
}

void MatrixStack44f::ApplyYawDegreesAboutZRotation(StackTarget target, float angleInDegrees)
{
	Matrix44f rotationMatrix = Matrix44f::GetZRotationMatrix(angleInDegrees);

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_viewStack.back());
}

void MatrixStack44f::ApplyPitchDegreesAboutYRotation(StackTarget target, float angleInDegrees)
{
	Matrix44f rotationMatrix = Matrix44f::GetYRotationMatrix(angleInDegrees);

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_viewStack.back());
}
void MatrixStack44f::ApplyRollDegreesAboutXRotation(StackTarget target, float angleInDegrees)
{
	Matrix44f rotationMatrix = Matrix44f::GetXRotationMatrix(angleInDegrees);

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(rotationMatrix, m_viewStack.back());
}

void MatrixStack44f::ApplyTranslation(StackTarget target, const Vector3f& translationVector)
{
	Matrix44f translationMatrix;

	translationMatrix.data[12] = translationVector.x;
	translationMatrix.data[13] = translationVector.y;
	translationMatrix.data[14] = translationVector.z;

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(translationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(translationMatrix, m_viewStack.back());
}

//-----------------------------------------------------------------------------------------------
void MatrixStack44f::ApplyTranslation(StackTarget target, const Vector2f& translation)
{
	Matrix44f translationMatrix;

	translationMatrix.data[12] = translation.x;
	translationMatrix.data[13] = translation.y;
	translationMatrix.data[14] = 0.f;

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(translationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(translationMatrix, m_viewStack.back());
}

void MatrixStack44f::ApplyTranslation(StackTarget target, float x, float y, float z)
{
	Matrix44f translationMatrix;

	translationMatrix.data[12] = x;
	translationMatrix.data[13] = y;
	translationMatrix.data[14] = z;

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(translationMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(translationMatrix, m_viewStack.back());
}

void MatrixStack44f::ApplyUniformScale(StackTarget target, float scale)
{
	Matrix44f scaleMatrix;

	scaleMatrix.data[0] = scale;
	scaleMatrix.data[5] = scale;
	scaleMatrix.data[10] = scale;

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(scaleMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(scaleMatrix, m_viewStack.back());
}
void MatrixStack44f::ApplyNonUniformScale(StackTarget target, float scale, float x, float y, float z)
{
	Matrix44f scaleMatrix;

	if(x != 0)
		scaleMatrix.data[0] = scale * x;
	if(y != 0)
		scaleMatrix.data[5] = scale * y;
	if(z != 0)
		scaleMatrix.data[10] = scale * z;

	if(target == MODEL_STACK)
		m_modelStack.back() = Matrix44f::MultMatrix(scaleMatrix, m_modelStack.back());
	else
		m_viewStack.back() = Matrix44f::MultMatrix(scaleMatrix, m_viewStack.back());
}

#endif