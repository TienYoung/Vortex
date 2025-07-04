#include "pch.h"

#include "Device.h"
#include "RenderTarget.h"

Vortex::RenderTarget::RenderTarget(HWND hWnd, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue) :
	m_swapChain(VX_DEVICE0->CreateSwapChain(hWnd, commandQueue))
{
	m_cpuDescriptorHandles.resize(VX_DOUBLE_BUFFER);
	m_presentBarriers.resize(VX_DOUBLE_BUFFER);
	m_renderBarriers.resize(VX_DOUBLE_BUFFER);

	for (uint32_t i = 0; i < VX_DOUBLE_BUFFER; i++)
	{
		winrt::com_ptr<ID3D12Resource> buffer;
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		m_cpuDescriptorHandles[i] = VX_DEVICE0->CreateRTV(buffer);

		m_presentBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(buffer.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_renderBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(buffer.get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}
