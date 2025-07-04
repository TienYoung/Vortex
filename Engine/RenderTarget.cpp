#include "pch.h"

#include "Device.h"
#include "RenderTarget.h"

Vortex::RenderTarget::RenderTarget(winrt::com_ptr<ID3D12Resource> buffer) :
	m_bufferResource(buffer)
{
	m_cpuDescriptorHandle = VX_DEVICE0->CreateRTV(m_bufferResource);
}
