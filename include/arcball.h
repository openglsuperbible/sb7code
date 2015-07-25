#ifndef __ARCBALL_H__
#define __ARCBALL_H__

#include <vmath.h>

namespace sb7
{

namespace utils
{

class arcball
{
public:
    arcball()
        : normalizedDims(-1.0f, 1.0f),
          width(1.0f),
          height(1.0f),
          dragging(false)
    {

    }

    void reset(void)
    {
        mouseStart = vmath::vec3(0.0f, 0.0f, 0.0f);
        mouseCurrent = vmath::vec3(0.0f, 0.0f, 0.0f);

        lastRotation = vmath::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
        currentRotation = vmath::quaternion(0.0f, 0.0f, 0.0f, 1.0f);

        quaternionToMatrix(currentRotation, rotationMatrix);

        lastRotationMatrix = vmath::mat4::identity();
    }

    void setDimensions(float w, float h)
    {
        width = w;
        height = h;

        normalizedDims[0] = 1.0f / w;
        normalizedDims[1] = 1.0f / h;
    }

    void onMouseDown(float x, float y)
    {
        vmath::vec2 mouseClip(2.0f * x * normalizedDims[0] - 1.0f, 1.0f - 2.0f * y * normalizedDims[1]);

        pointToSphere(mouseClip, mouseStart);
        lastRotation = currentRotation;
        lastRotationMatrix = rotationMatrix;

        dragging = true;
    }

    void onMouseUp(void)
    {
        dragging = false;
    }

    void onMouseMove(float x, float y)
    {
        if (dragging)
        {
            vmath::vec2 mouseClip(2.0f * x * normalizedDims[0] - 1.0f, 1.0f - 2.0f * y * normalizedDims[1]);

            pointToSphere(mouseClip, mouseCurrent);

            vmath::vec3 perpVector = vmath::normalize(vmath::cross(mouseStart, mouseCurrent));

            if (dot(perpVector, perpVector) > 0.00001f)
            {
                // vmath::quaternion q = vmath::vec4(perpVector, cosf(acosf(dot(mouseStart, mouseCurrent)) * 0.5f));
                // vmath::quaternion q = vmath::vec4(2.0f * 3.14159267f * cosf(x * 0.01f), vmath::vec3(0.0f, 0.0f, 1.0f)); // cosf(acosf(dot(mouseStart, mouseCurrent)) * 0.5f));

                // q = vmath::normalize(q);
                // currentRotation = q * lastRotation;

                // quaternionToMatrix(q, rotationMatrix);
                rotationMatrix = vmath::rotate(acosf(dot(mouseStart, mouseCurrent)) * 200.0f, -perpVector) * lastRotationMatrix;
            }
            else
            {
                dragging = true;
            }
        }
    }

    const vmath::quaternion getRotation() const
    {
        return currentRotation;
    }

    const vmath::mat4 getRotationMatrix() const
    {
        return rotationMatrix;
    }

private:
    float               width;
    float               height;

    float               start_x;
    float               start_y;
    bool                dragging;

    vmath::vec2         normalizedDims;

    vmath::quaternion   lastRotation;
    vmath::quaternion   currentRotation;
    vmath::vec3         mouseStart;
    vmath::vec3         mouseCurrent;
    vmath::mat4         lastRotationMatrix;
    vmath::mat4         rotationMatrix;

    void pointToSphere(const vmath::vec2& pt, vmath::vec3& vec);
};

void arcball::pointToSphere(const vmath::vec2& pt, vmath::vec3& vec)
{
    float length2 = vmath::dot(pt, pt);

    if (length2 <= 1.0f)
    {
        vec = vmath::vec3(pt, sqrt(1.0f - length2));
    }
    else
    {
        float norm = 1.0f / sqrt(length2);
        vec = vmath::vec3(pt * norm, 0.0f);
    }
}

}

}

#endif /* __ARCBALL_H__ */
