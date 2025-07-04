#pragma once

namespace Vortex
{
	class RenderTarget
	{
	public:
		RenderTarget(HWND window, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue);

		const CD3DX12_RESOURCE_BARRIER* PrepareForPresent() const
		{
			return &m_presentBarriers[m_swapChain->GetCurrentBackBufferIndex()];
		}

		const CD3DX12_RESOURCE_BARRIER* PrepareForRender() const
		{
			return &m_renderBarriers[m_swapChain->GetCurrentBackBufferIndex()];
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const
		{
			return m_cpuDescriptorHandles[m_swapChain->GetCurrentBackBufferIndex()];
		}

		void Flip() const
		{
			winrt::check_hresult(m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
		}

	private:
		winrt::com_ptr<IDXGISwapChain3> m_swapChain;

		std::vector<winrt::com_ptr<ID3D12Resource>> m_buffers;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_cpuDescriptorHandles;
		std::vector<CD3DX12_RESOURCE_BARRIER> m_presentBarriers;
		std::vector<CD3DX12_RESOURCE_BARRIER> m_renderBarriers;
	};
}

