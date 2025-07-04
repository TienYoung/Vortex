#include "pch.h"

#include "Device.h"
#include "RenderTarget.h"

Vortex::RenderTarget::RenderTarget(HWND window, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue) :
	m_swapChain(VX_DEVICE0->CreateSwapChain(window, commandQueue))
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
