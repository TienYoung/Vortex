#pragma once

namespace Vortex
{
	class RenderTarget
	{
	public:
		RenderTarget(HWND hWnd, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue);

		inline const CD3DX12_RESOURCE_BARRIER* PrepareForPresent() const
		{
			return &m_presentBarriers[m_swapChain->GetCurrentBackBufferIndex()];
		}

		inline const CD3DX12_RESOURCE_BARRIER* PrepareForRender() const
		{
			return &m_renderBarriers[m_swapChain->GetCurrentBackBufferIndex()];
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const
		{
			return m_cpuDescriptorHandles[m_swapChain->GetCurrentBackBufferIndex()];
		}

		inline void Flip() const
		{
			winrt::check_hresult(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
		}

		inline uint32_t GetWidth() const
		{
			DXGI_SWAP_CHAIN_DESC1 desc{};
			winrt::check_hresult(m_swapChain->GetDesc1(&desc));
			return desc.Width;
		}

		inline uint32_t GetHeight() const
		{
			DXGI_SWAP_CHAIN_DESC1 desc{};
			winrt::check_hresult(m_swapChain->GetDesc1(&desc));
			return desc.Height;
		}

	private:
		winrt::com_ptr<IDXGISwapChain3> m_swapChain;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cpuDescriptorHandles;
		std::vector<CD3DX12_RESOURCE_BARRIER> m_presentBarriers;
		std::vector<CD3DX12_RESOURCE_BARRIER> m_renderBarriers;
	};
}

