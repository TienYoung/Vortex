#pragma once

#include "Renderer.h"
#include "Shader.h"

namespace Vortex
{
    class SkyboxRenderPass : public Renderer::IRenderPass
    {
    public:
        SkyboxRenderPass()
        {
            //m_textureResource = VX_DEVICE0->CreateTextureResource(DXGI_FORMAT_R8_UNORM, 32 * 32, 32 * 32);
            //m_textureHanle = VX_DEVICE0->CreateSRV(4, m_textureResource, DXGI_FORMAT_R8_UNORM);
            CD3DX12_DESCRIPTOR_RANGE1 descRange[1] = {};
            descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0

            CD3DX12_ROOT_PARAMETER1 rootParameter[1] = {};
            rootParameter[0].InitAsDescriptorTable(_countof(descRange), &descRange[0]);
            
            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc;
            versionedRootSignatureDesc.Init_1_1(_countof(rootParameter), &rootParameter[0]);

            m_rootSignature = VX_DEVICE0->CreateRootSignature(versionedRootSignatureDesc);

            auto skyboxMS = std::make_shared<Shader>(L"SkyboxMS");
            auto skyboxPS = std::make_shared<Shader>(L"SkyboxPS");
            m_skyboxPSO = VX_DEVICE0->CreateMeshPSO(m_rootSignature, skyboxMS->GetBytecode(), skyboxPS->GetBytecode());
            
            m_commandAllocator = VX_DEVICE0->CreateGraphicsCommandAllocator();
            m_commandList = VX_DEVICE0->CreateGraphicsCommandList();
        }

        inline ID3D12GraphicsCommandList10* GetCommandList(const Renderer& renderer) const override
        {
            winrt::check_hresult(m_commandAllocator->Reset());
            winrt::check_hresult(m_commandList->Reset(m_commandAllocator.get(), nullptr));

            m_commandList->RSSetViewports(1, renderer.GetViewport());
            m_commandList->RSSetScissorRects(1, renderer.GetScissorRect());

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderer.GetRenderTarget()->GetCPUDescriptorHandle();
            m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

            std::vector<ID3D12DescriptorHeap*> heaps = VX_DEVICE0->GetHeaps();
            m_commandList->SetDescriptorHeaps(static_cast<uint32_t>(heaps.size()), heaps.data());

            m_commandList->SetPipelineState(m_skyboxPSO.get());
            m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
            m_commandList->SetGraphicsRootDescriptorTable(0, renderer.GetGlobalParamsHandle());
            m_commandList->DispatchMesh(1, 1, 1);

            winrt::check_hresult(m_commandList->Close());
            return m_commandList.get();
        }

    private:
        //winrt::com_ptr<ID3D12Resource> m_textureResource;
        //CD3DX12_GPU_DESCRIPTOR_HANDLE m_textureHanle;
        winrt::com_ptr<ID3D12RootSignature> m_rootSignature;
        winrt::com_ptr<ID3D12PipelineState> m_skyboxPSO;
        // Command
        winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
        winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandList;
    };
}