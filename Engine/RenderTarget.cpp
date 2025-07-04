#include "pch.h"

#include "Device.h"
#include "RenderTarget.h"

Vortex::RenderTarget::RenderTarget(winrt::com_ptr<IDXGISwapChain3> swapChain) :
	m_swapChain(swapChain)
{
	m_buffers.resize(VX_DOUBLE_BUFFER);
	m_cpuDescriptorHandles.resize(VX_DOUBLE_BUFFER);
	m_presentBarriers.resize(VX_DOUBLE_BUFFER);
	m_renderBarriers.resize(VX_DOUBLE_BUFFER);


	for (uint32_t i = 0; i < VX_DOUBLE_BUFFER; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]));
		m_cpuDescriptorHandles[i] = VX_DEVICE0->CreateRTV(m_buffers[i]);

		m_presentBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_buffers[i].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_renderBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_buffers[i].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}
