#pragma once

#include "Renderer.h"
#include "AccelerationStructure.h"
#include "ShaderTable.h"

namespace Vortex
{
	class RayTracingRenderPass : public Renderer::IRenderPass
	{
	public:
		RayTracingRenderPass()
		{
			m_commandAllocator = VX_DEVICE0->CreateGraphicsCommandAllocator();
			m_commandList = VX_DEVICE0->CreateGraphicsCommandList();


			// Create output texture
			{
				winrt::com_ptr<ID3D12DescriptorHeap> tempDescriptorHeap = VX_DEVICE0->CreateResourceHeapOnCPU(1);

				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				winrt::check_hresult(VX_DEVICE0->Get()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(&m_outputTexture)
				));

				D3D12_CPU_DESCRIPTOR_HANDLE tempDescriptorHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM);
				VX_DEVICE0->Get()->CreateUnorderedAccessView(m_outputTexture.get(), nullptr, &uavDesc, tempDescriptorHandle);

				m_descriptorHeap = VX_DEVICE0->CreateResourceHeapOnGPU(1);
				D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
				VX_DEVICE0->Get()->CopyDescriptorsSimple(1, descriptorHandle, tempDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			// Create root signature.
			{
				CD3DX12_DESCRIPTOR_RANGE1 descRange[1] = {};
				descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

				CD3DX12_ROOT_PARAMETER1 rootParameter[2] = {};
				rootParameter[0].InitAsShaderResourceView(0); // t0
				rootParameter[1].InitAsDescriptorTable(1, &descRange[0]); // u0

				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
				rootSignatureDesc.Init_1_1(_countof(rootParameter), rootParameter);

				m_rootSignature = VX_DEVICE0->CreateRootSignature(rootSignatureDesc);
			}

			// Create shader tables
			{
				D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(2 * sizeof(ShaderTable));
				winrt::com_ptr<ID3D12Resource> resource;
				winrt::check_hresult(VX_DEVICE0->Get()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&resource)
				));

				D3D12_GPU_VIRTUAL_ADDRESS a = resource->GetGPUVirtualAddress();

				void* ptr = nullptr;
				CD3DX12_RANGE range = {};
				resource->Map(0, &range, &ptr);
				ShaderRecord* shaderTable = new(ptr) ShaderRecord[2];
				memcpy(shaderTable[0].ShaderIdentifier, m_stateObject.as<ID3D12StateObjectProperties>()->GetShaderIdentifier(L"RayGen"), 32);
				memcpy(shaderTable[1].ShaderIdentifier, m_stateObject.as<ID3D12StateObjectProperties>()->GetShaderIdentifier(L"Miss"), 32);
			}
		}

		inline ID3D12GraphicsCommandList10* GetCommandList(const Renderer* renderer) const override
		{
			winrt::check_hresult(m_commandAllocator->Reset());
			winrt::check_hresult(m_commandList->Reset(m_commandAllocator.get(), nullptr));

			// TODO: build acceleration structure
			auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_accelerationStructure->GetTLAS().get());
			m_commandList->ResourceBarrier(1, &uavBarrier);

			m_commandList->SetPipelineState1(m_stateObject.get());
			m_commandList->SetComputeRootSignature(m_rootSignature.get());

			ID3D12DescriptorHeap* descriptorHeap = m_descriptorHeap.get();
			m_commandList->SetDescriptorHeaps(1, &descriptorHeap);
			m_commandList->SetComputeRootUnorderedAccessView(0, m_accelerationStructure->GetTLAS()->GetGPUVirtualAddress());
			m_commandList->SetComputeRootDescriptorTable(1, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

			D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc =
			{
				.Width = renderer->GetRenderTarget()->GetWidth(),
				.Height = renderer->GetRenderTarget()->GetHeight(),
			};
			m_commandList->DispatchRays(&dispatchRaysDesc);
		}

	private:
		// Pipeline
		winrt::com_ptr<ID3D12StateObject> m_stateObject;
		winrt::com_ptr<ID3D12RootSignature> m_rootSignature;
		
		// Command
		winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
		winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandList;

		// Resources
		winrt::com_ptr<ID3D12DescriptorHeap> m_descriptorHeap;

		std::unique_ptr<AccelerationStructure> m_accelerationStructure;
		winrt::com_ptr<ID3D12Resource> m_outputTexture;

		winrt::com_ptr<ID3D12Resource> m_raygenshaderTable;
		winrt::com_ptr<ID3D12Resource> m_missShaderTable;
	};
}