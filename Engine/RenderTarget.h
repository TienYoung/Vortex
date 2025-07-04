#pragma once

namespace Vortex
{
	class RenderTarget
	{
	public:
		RenderTarget() = default;
		RenderTarget(winrt::com_ptr<ID3D12Resource> buffer);

		ID3D12Resource* GetResource() const
		{
			return m_bufferResource.get();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const
		{
			return m_cpuDescriptorHandle;
		}

	private:
		winrt::com_ptr<ID3D12Resource> m_bufferResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle;
	};
}

