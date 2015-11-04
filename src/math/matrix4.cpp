#include "matrix4.h"
#include <cmath>
#include <limits>

#include <iostream>
using namespace std;

Matrix4::Matrix4()
{
    identity();
}

Matrix4::Matrix4(const Matrix4 &mat)
{
    set(mat.array[0], mat.array[1], mat.array[2], mat.array[3],
        mat.array[4], mat.array[5], mat.array[6], mat.array[7],
        mat.array[8], mat.array[9], mat.array[10], mat.array[11],
        mat.array[12], mat.array[13], mat.array[14], mat.array[15]);
}

Matrix4& Matrix4::operator=(const Matrix4 &mat)
{
    set(mat.array[0], mat.array[1], mat.array[2], mat.array[3],
        mat.array[4], mat.array[5], mat.array[6], mat.array[7],
        mat.array[8], mat.array[9], mat.array[10], mat.array[11],
        mat.array[12], mat.array[13], mat.array[14], mat.array[15]);

    return *this;
}

Matrix4& Matrix4::set(float n11, float n12, float n13, float n14,
                      float n21, float n22, float n23, float n24,
                      float n31, float n32, float n33, float n34,
                      float n41, float n42, float n43, float n44 )
{

    array[0] = n11; array[1] = n12; array[2] = n13; array[3] = n14;
    array[4] = n21; array[5] = n22; array[6] = n23; array[7] = n24;
    array[8] = n31; array[9] = n32; array[10] = n33; array[11] = n34;
    array[12] = n41; array[13] = n42; array[14] = n43; array[15] = n44;

    return *this;
}

Matrix4& Matrix4::set(const float* values)
{
    for(int i=0; i<16; i++)
        array[i] = values[i];
    return *this;
}

Matrix4& Matrix4::copy(const Matrix4 &mat)
{
    set(mat.array[0], mat.array[1], mat.array[2], mat.array[3],
        mat.array[4], mat.array[5], mat.array[6], mat.array[7],
        mat.array[8], mat.array[9], mat.array[10], mat.array[11],
        mat.array[12], mat.array[13], mat.array[14], mat.array[15]);

    return *this;
}

Matrix4& Matrix4::transpose()
{
    float tmp;

    tmp = data[1][0]; data[1][0] = data[0][1]; data[0][1] = tmp;
    tmp = data[2][0]; data[2][0] = data[0][2]; data[0][2] = tmp;
    tmp = data[2][1]; data[2][1] = data[1][2]; data[1][2] = tmp;

    tmp = data[3][0]; data[3][0] = data[0][3]; data[0][3] = tmp;
    tmp = data[3][1]; data[3][1] = data[1][3]; data[1][3] = tmp;
    tmp = data[3][2]; data[3][2] = data[2][3]; data[2][3] = tmp;

    return *this;
}

Matrix4& Matrix4::setPosition(const Vector3 &xyz)
{
    data[0][3] = xyz.x;
    data[1][3] = xyz.y;
    data[2][3] = xyz.z;

    return *this;
}

Matrix4& Matrix4::translate(const Vector3 &xyz)
{

    /*data[0][3] = data[0][0] * xyz.x + data[0][1] * xyz.y + data[0][2] * xyz.z + data[0][3];
    data[1][3] = data[1][0] * xyz.x + data[1][1] * xyz.y + data[1][2] * xyz.z + data[1][3];
    data[2][3] = data[2][0] * xyz.x + data[2][1] * xyz.y + data[2][2] * xyz.z + data[2][3];
    data[3][3] = data[3][0] * xyz.x + data[3][1] * xyz.y + data[3][2] * xyz.z + data[3][3];*/

    data[0][3] += xyz.x;
    data[1][3] += xyz.y;
    data[2][3] += xyz.z;

    return *this;
}

Matrix4& Matrix4::scale(const Vector3 &xyz)
{
    data[0][0] *= xyz.x; data[0][1] *= xyz.x; data[0][2] *= xyz.x; data[0][3] *= xyz.x;
    data[1][0] *= xyz.y; data[1][1] *= xyz.y; data[1][2] *= xyz.y; data[1][3] *= xyz.y;
    data[2][0] *= xyz.z; data[2][1] *= xyz.z; data[2][2] *= xyz.z; data[2][3] *= xyz.z;

    return *this;
}

float Matrix4::determinant() const
{
    float m0= data[0][0], m1  = data[1][0], m2  = data[2][0], m3  = data[3][0],
         m4 = data[0][1], m5  = data[1][1], m6  = data[2][1], m7  = data[3][1],
         m8 = data[0][2], m9  = data[1][2], m10 = data[2][2], m11 = data[3][2],
        m12 = data[0][3], m13 = data[1][3], m14 = data[2][3], m15 = data[3][3];

    return (
        m12 * m9 * m6 * m3  - m8 * m13 * m6 * m3    - m12 * m5 * m10 * m3   + m4 * m13 * m10 * m3   +
        m8 * m5 * m14 * m3  - m4 * m9 * m14 * m3    - m12 * m9 * m2 * m7    + m8 * m13 * m2 * m7    +
        m12 * m1 * m10 * m7 - m0 * m13 * m10 * m7   - m8 * m1 * m14 * m7    + m0 * m9 * m14 * m7    +
        m12 * m5 * m2 * m11 - m4 * m13 * m2 * m11   - m12 * m1 * m6 * m11   + m0 * m13 * m6 * m11   +
        m4 * m1 * m14 * m11 - m0 * m5 * m14 * m11   - m8 * m5 * m2 * m15    + m4 * m9 * m2 * m15    +
        m8 * m1 * m6 * m15  - m0 * m9 * m6 * m15    - m4 * m1 * m10 * m15   + m0 * m5 * m10 * m15
    );
}

Matrix4 Matrix4::inverse() const
{
    Matrix4 t;

    float m0= data[0][0], m1  = data[1][0], m2  = data[2][0], m3  = data[3][0],
         m4 = data[0][1], m5  = data[1][1], m6  = data[2][1], m7  = data[3][1],
         m8 = data[0][2], m9  = data[1][2], m10 = data[2][2], m11 = data[3][2],
        m12 = data[0][3], m13 = data[1][3], m14 = data[2][3], m15 = data[3][3];

    t.data[0][0] = (m9*m14*m7    -m13*m10*m7     +m13*m6*m11     -m5*m14*m11     -m9*m6*m15      +m5*m10*m15);
    t.data[1][0] = (m13*m10*m3   -m9*m14*m3      -m13*m2*m11     +m1*m14*m11     +m9*m2*m15      -m1*m10*m15);
    t.data[2][0] = (m5*m14*m3    -m13*m6*m3      +m13*m2*m7      -m1*m14*m7      -m5*m2*m15      +m1*m6*m15);
    t.data[3][0] = (m9*m6*m3     -m5*m10*m3      -m9*m2*m7       +m1*m10*m7      +m5*m2*m11      -m1*m6*m11);

    t.data[0][1] = (m12*m10*m7   -m8*m14*m7      -m12*m6*m11     +m4*m14*m11     +m8*m6*m15      -m4*m10*m15);
    t.data[1][1] = (m8*m14*m3    -m12*m10*m3     +m12*m2*m11     -m0*m14*m11     -m8*m2*m15      +m0*m10*m15);
    t.data[2][1] = (m12*m6*m3    -m4*m14*m3      -m12*m2*m7      +m0*m14*m7      +m4*m2*m15      -m0*m6*m15);
    t.data[3][1] = (m4*m10*m3    -m8*m6*m3       +m8*m2*m7       -m0*m10*m7      -m4*m2*m11      +m0*m6*m11);

    t.data[0][2] = (m8*m13*m7    -m12*m9*m7      +m12*m5*m11     -m4*m13*m11     -m8*m5*m15      +m4*m9*m15);
    t.data[1][2] = (m12*m9*m3    -m8*m13*m3      -m12*m1*m11     +m0*m13*m11     +m8*m1*m15      -m0*m9*m15);
    t.data[2][2] = (m4*m13*m3    -m12*m5*m3      +m12*m1*m7      -m0*m13*m7      -m4*m1*m15      +m0*m5*m15);
    t.data[3][2] = (m8*m5*m3     -m4*m9*m3       -m8*m1*m7       +m0*m9*m7       +m4*m1*m11      -m0*m5*m11);

    t.data[0][3] = (m12*m9*m6    -m8*m13*m6      -m12*m5*m10     +m4*m13*m10     +m8*m5*m14      -m4*m9*m14);
    t.data[1][3] = (m8*m13*m2    -m12*m9*m2      +m12*m1*m10     -m0*m13*m10     -m8*m1*m14      +m0*m9*m14);
    t.data[2][3] = (m12*m5*m2    -m4*m13*m2      -m12*m1*m6      +m0*m13*m6      +m4*m1*m14      -m0*m5*m14);
    t.data[3][3] = (m4*m9*m2     -m8*m5*m2       +m8*m1*m6       -m0*m9*m6       -m4*m1*m10      +m0*m5*m10);

    float d = determinant();

    if(d == 0.0f)
    {
        return t.zero();
    }

    float s = 1.0 / d;
    t *= s;

    return t;
}

Matrix4 Matrix4::operator*(const Matrix4 &mat) const
{
    Matrix4 result;

    for(int idx = 0; idx < 4; idx++)
    {
        for(int idy = 0; idy < 4; idy++)
        {
            result.data[idy][idx] = 0.0f;
            for(int idz = 0; idz < 4; idz++)
            {
                result.data[idy][idx] += data[idy][idz] * mat.data[idz][idx];
            }
        }
    }
    return result;
}

void Matrix4::operator*=(const Matrix4 &mat)
{
    copy(mat * *this);
}

Matrix4 Matrix4::operator*(const float scalar) const
{
    Matrix4 result;

    for(int idx = 0; idx < 4; idx++)
    {
        for(int idy = 0; idy < 4; idy++)
        {
            result.data[idx][idy] = data[idx][idy] * scalar;
        }
    }
    return result;
}

void Matrix4::operator*=(const float scalar)
{
    for(int idx = 0; idx < 4; idx++)
    {
        for(int idy = 0; idy < 4; idy++)
        {
            data[idx][idy] *= scalar;
        }
    }
}

Matrix4& Matrix4::identity()
{
    data[0][0] = 1.0f; data[0][1] = 0.0f; data[0][2] = 0.0f; data[0][3] = 0.0f;
    data[1][0] = 0.0f; data[1][1] = 1.0f; data[1][2] = 0.0f; data[1][3] = 0.0f;
    data[2][0] = 0.0f; data[2][1] = 0.0f; data[2][2] = 1.0f; data[2][3] = 0.0f;
    data[3][0] = 0.0f; data[3][1] = 0.0f; data[3][2] = 0.0f; data[3][3] = 1.0f;

    return *this;
}

Matrix4& Matrix4::zero()
{
    data[0][0] = 0.0f; data[0][1] = 0.0f; data[0][2] = 0.0f; data[0][3] = 0.0f;
    data[1][0] = 0.0f; data[1][1] = 0.0f; data[1][2] = 0.0f; data[1][3] = 0.0f;
    data[2][0] = 0.0f; data[2][1] = 0.0f; data[2][2] = 0.0f; data[2][3] = 0.0f;
    data[3][0] = 0.0f; data[3][1] = 0.0f; data[3][2] = 0.0f; data[3][3] = 0.0f;

    return *this;
}

void Matrix4::log()
{
    for(int idx = 0; idx < 4; idx++)
    {
        for(int idy = 0; idy < 4; idy++)
            cout << data[idx][idy] << " ";
        cout << endl;
    }
}

Vector3 Matrix4::operator *(const Vector3 vec3) const
{
    return Vector3(data[0][0] * vec3.x + data[0][1] * vec3.y + data[0][2] * vec3.z + data[0][3],
                   data[1][0] * vec3.x + data[1][1] * vec3.y + data[1][2] * vec3.z + data[1][3],
                   data[2][0] * vec3.x + data[2][1] * vec3.y + data[2][2] * vec3.z + data[2][3]);
}
