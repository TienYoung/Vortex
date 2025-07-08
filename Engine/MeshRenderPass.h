#pragma once

#include "Renderer.h"
#include "Shader.h"
#include "CompiledShaders/ProceduralAS.hlsl.h"
#include "CompiledShaders/ProceduralMS.hlsl.h"
#include "CompiledShaders/ProceduralPS.hlsl.h"

namespace Vortex
{
	class MeshRenderPass : public Renderer::IRenderPass
	{
	public:
		MeshRenderPass() : m_materialParams(std::make_shared<MaterialParameters>())
		{
			// Create resources.
			{
				m_materialResource = VX_DEVICE0->CreateConstantResource((sizeof(MaterialParameters) + 255) & ~255);
				m_gpuHandle1 = VX_DEVICE0->CreateCBV(1, m_materialResource, (sizeof(MaterialParameters) + 255) & ~255);

				m_textureResource = VX_DEVICE0->CreateTextureResource(DXGI_FORMAT_R8_UNORM, 32 * 32, 32 * 32);
				m_gpuHandle2 = VX_DEVICE0->CreateSRV(2, m_textureResource, DXGI_FORMAT_R8_UNORM);

				m_gpuHandle3 = VX_DEVICE0->CreateUAV(3, m_textureResource, DXGI_FORMAT_R8_UNORM);
			}
			// Create root signature.
			{
				CD3DX12_DESCRIPTOR_RANGE1 descRange[3] = {};
				descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // b0 ~ b1
				descRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
				descRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

				CD3DX12_ROOT_PARAMETER1 rootParameter[3] = {};
				rootParameter[0].InitAsDescriptorTable(1, &descRange[0]); // b0 ~ b1
				rootParameter[1].InitAsDescriptorTable(1, &descRange[1]); // t0
				rootParameter[2].InitAsDescriptorTable(1, &descRange[2]); // u0

				CD3DX12_STATIC_SAMPLER_DESC staticSamplerDesc[2] = {};
				staticSamplerDesc[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_POINT); // s0
				staticSamplerDesc[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR); // s1

				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc;
				versionedRootSignatureDesc.Init_1_1(_countof(rootParameter), rootParameter, _countof(staticSamplerDesc), staticSamplerDesc);

				m_rootSignature = VX_DEVICE0->CreateRootSignature(versionedRootSignatureDesc);
			}
			// Create pipeline states.
			{
				auto computeShader = std::make_shared<Shader>(L"NoiseCS");
				m_computePSO = VX_DEVICE0->CreateComputePSO(m_rootSignature, computeShader->GetBytecode());

				m_pipelineStateObject = VX_DEVICE0->CreateMeshStateObject
				(
					m_rootSignature,
					CD3DX12_SHADER_BYTECODE(g_pProceduralMS, _countof(g_pProceduralMS)),
					CD3DX12_SHADER_BYTECODE(g_pProceduralPS, _countof(g_pProceduralPS)),
					CD3DX12_SHADER_BYTECODE(g_pProceduralAS, _countof(g_pProceduralAS))
				);

				m_commandAllocator = VX_DEVICE0->CreateGraphicsCommandAllocator();
				m_commandList = VX_DEVICE0->CreateGraphicsCommandList();
			}
		}

		inline ID3D12GraphicsCommandList10* GetCommandList(const Renderer* renderer) const override
		{
			uint8_t* gpuPtr = nullptr;
			CD3DX12_RANGE range(0, 0);
			winrt::check_hresult(m_materialResource->Map(0, &range, reinterpret_cast<void**>(&gpuPtr)));
			m_materialParams->offset = DirectX::XMVectorScale(DirectX::XMVectorSplatOne(), 0.1f);
			m_materialParams->scale = 0.1f;
			memcpy(gpuPtr, m_materialParams.get(), sizeof(MaterialParameters));
			m_materialResource->Unmap(0, nullptr);

			winrt::check_hresult(m_commandAllocator->Reset());
			winrt::check_hresult(m_commandList->Reset(m_commandAllocator.get(), nullptr));

			m_commandList->RSSetViewports(1, renderer->GetViewport());
			m_commandList->RSSetScissorRects(1, renderer->GetScissorRect());

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderer->GetRenderTarget()->GetCPUDescriptorHandle();
			m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			std::vector<ID3D12DescriptorHeap*> heaps = VX_DEVICE0->GetHeaps();
			m_commandList->SetDescriptorHeaps(static_cast<uint32_t>(heaps.size()), heaps.data());

			// Compute
			auto transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_textureResource.get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			m_commandList->ResourceBarrier(1, &transitionBarrier);

			m_commandList->SetPipelineState(m_computePSO.get());
			m_commandList->SetComputeRootSignature(m_rootSignature.get());
			m_commandList->SetComputeRootDescriptorTable(2, m_gpuHandle3);
			m_commandList->Dispatch(32, 32, 1);

			// Graphics
			transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_textureResource.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			m_commandList->ResourceBarrier(1, &transitionBarrier);
			m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
			D3D12_SET_PROGRAM_DESC programDesc =
			{
				.Type = D3D12_PROGRAM_TYPE_GENERIC_PIPELINE,
				.GenericPipeline =
				{
					.ProgramIdentifier = m_pipelineStateObject.as<ID3D12StateObjectProperties1>()->GetProgramIdentifier(L"proceduralMesh"),
				},
			};
			m_commandList->SetProgram(&programDesc);
			m_commandList->SetGraphicsRootDescriptorTable(0, renderer->GetGlobalParamsHandle());
			m_commandList->SetGraphicsRootDescriptorTable(1, m_gpuHandle2);
			m_commandList->DispatchMesh(1, 1, 1);

			winrt::check_hresult(m_commandList->Close());
			return m_commandList.get();
		}

		//inline ID3D12DescriptorHeap* GetDescriptorHeap() const override { return m_descriptorHeap.get(); }
		//inline uint32_t GetResourceBarrier(const CD3DX12_RESOURCE_BARRIER*& barrier) const override
		//{
		//    barrier = new CD3DX12_RESOURCE_BARRIER(CD3DX12_RESOURCE_BARRIER::Transition(m_textureResource.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		//    return 1;
		//}

	private:
		// GPU Resource
		winrt::com_ptr<ID3D12Resource> m_materialResource;
		winrt::com_ptr<ID3D12Resource> m_textureResource;

		CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuHandle1;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuHandle2;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuHandle3;

		// CPU Resource
		std::shared_ptr<MaterialParameters> m_materialParams;

		// Pipeline
		winrt::com_ptr<ID3D12RootSignature> m_rootSignature;
		winrt::com_ptr<ID3D12PipelineState> m_computePSO;
		winrt::com_ptr<ID3D12StateObject> m_pipelineStateObject;

		// Command
		winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
		winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandList;
	};
}