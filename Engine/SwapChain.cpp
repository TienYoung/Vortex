#include "pch.h"

#include "Device.h"
#include "SwapChain.h"

Vortex::SwapChain::SwapChain(const winrt::com_ptr<ID3D12CommandQueue>& commandQueue, HWND hwnd, uint32_t width, uint32_t height) :
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_swapChain(VX_DEVICE0->CreateSwapChain(hwnd, width, height, commandQueue)),
	m_renderTarget(m_swapChain)
{
}