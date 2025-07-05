#pragma once

#include "Math.h"

namespace Vortex
{
    class Camera
    {
    public:
        Camera()
        {
        }
        
        inline Matrix GetView() const
        {
            Matrix view = DirectX::XMMatrixRotationQuaternion(m_rotation);
            return view;
        }

        inline Matrix GetProjection(float aspect) const
        {
            Matrix projection = DirectX::XMMatrixPerspectiveFovLH(90.0f, aspect, 0.1f, 10000.0f);
            return projection;
        }

        inline void MoveForward(float offset)
        {
            DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::g_XMNegIdentityR1, m_rotation);
            m_position += DirectX::XMVectorScale(forward, offset);
        }
        
        inline void MoveRight(float offset)
        {
            DirectX::XMVECTOR up = DirectX::XMVector3Rotate(DirectX::g_XMIdentityR2, m_rotation);
            m_position += DirectX::XMVectorScale(up, offset);
        }

        inline void MoveUp(float offset)
        {
            DirectX::XMVECTOR right = DirectX::XMVector3Rotate(DirectX::g_XMIdentityR0, m_rotation);
            m_position += DirectX::XMVectorScale(right, offset);
        }

        //inline void Yaw(float delta)
        //{
        //    DirectX::XMVECTOR yawRotation = DirectX::XMQuaternionRotationAxis(DirectX::g_XMIdentityR2, delta);
        //    m_rotation = DirectX::XMQuaternionMultiply(yawRotation, m_rotation);
        //}

        //inline void Roll(float delta)
        //{
        //    DirectX::XMVECTOR rollRotation = DirectX::XMQuaternionRotationRollPitchYaw(0, delta, 0);
        //    m_rotation = DirectX::XMQuaternionMultiply(m_rotation, rollRotation);
        //}

        //inline void Pitch(float delta)
        //{
        //    DirectX::XMVECTOR pitchRotation = DirectX::XMQuaternionRotationAxis(DirectX::g_XMIdentityR0, delta);
        //    m_rotation = DirectX::XMQuaternionMultiply(m_rotation, pitchRotation);
        //}

        inline void Rotate(float yaw, float pitch)
        {
            // m_rotation = yaw * m_rotation * pitch;
            m_rotation = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, yaw), m_rotation);
            m_rotation = DirectX::XMQuaternionMultiply(m_rotation, DirectX::XMQuaternionRotationRollPitchYaw(pitch, 0.0f, 0.0f));
        }

    private:
        Vector3 m_position;
        Quaternion m_rotation;
    };
}