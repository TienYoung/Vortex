#pragma once

#include "RenderTarget.h"

namespace Vortex
{
	class SwapChain
	{
	public:
		SwapChain() = delete;
		SwapChain(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		SwapChain(const winrt::com_ptr<ID3D12CommandQueue>& commandQueue, HWND hwnd, uint32_t width, uint32_t height);

		inline const CD3DX12_VIEWPORT* GetViewport() const { return &m_viewport; }
		inline const CD3DX12_RECT* GetScissorRect() const { return &m_scissorRect; }

		const RenderTarget* GetRenderTarget() const
		{
			return &m_renderTarget;
		}

		inline void Flip() {
			winrt::check_hresult(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
		}
	private:
		// Window relatives.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		// Render targets
		winrt::com_ptr<IDXGISwapChain3> m_swapChain;

		RenderTarget m_renderTarget;
	};
}
