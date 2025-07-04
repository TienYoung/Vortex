#include "pch.h"

#include "Device.h"
#include "SwapChain.h"

Vortex::SwapChain::SwapChain(const winrt::com_ptr<ID3D12CommandQueue>& commandQueue, HWND hwnd, uint32_t width, uint32_t height) :
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_swapChain(VX_DEVICE0->CreateSwapChain(hwnd, width, height, commandQueue))
{
	for (uint32_t i = 0; i < VX_DOUBLE_BUFFER; i++)
	{
		winrt::com_ptr<ID3D12Resource> buffer;
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		m_renderTargets[i] = RenderTarget(buffer);
		m_presentBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[i].GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_renderBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[i].GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}