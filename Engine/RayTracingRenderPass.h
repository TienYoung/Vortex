#pragma once

#include "Renderer.h"
#include "AccelerationStructure.h"

namespace Vortex
{
	class RayTracingRenderPass : public Renderer::IRenderPass
	{
	public:
		RayTracingRenderPass()
		{
			m_commandAllocator = VX_DEVICE0->CreateGraphicsCommandAllocator();
			m_commandList = VX_DEVICE0->CreateGraphicsCommandList();
		}

		inline ID3D12GraphicsCommandList10* GetCommandList(const Renderer* renderer) const override
		{
			winrt::check_hresult(m_commandAllocator->Reset());
			winrt::check_hresult(m_commandList->Reset(m_commandAllocator.get(), nullptr));

			auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure->GetTLAS().get());
			m_commandList->ResourceBarrier(1, &uavBarrier);

			m_commandList->SetComputeRootUnorderedAccessView(0, m_accelerationStructure->GetTLAS()->GetGPUVirtualAddress());
		}

	private:
		// Command
		winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
		winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandList;

		// Resources
		std::unique_ptr<AccelerationStructure> m_accelerationStructure;
	};
}