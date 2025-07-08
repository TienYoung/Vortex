#include "pch.h"
#include "Renderer.h"

Vortex::Renderer::Renderer(HWND hWnd) :
    m_fenceEvent(::CreateEvent(nullptr, FALSE, FALSE, nullptr)), m_fenceValue(0),
    m_timeSinceStart(std::chrono::steady_clock::now()),
    m_camera(std::make_shared<Camera>()),
    m_globalParams(std::make_shared<GlobalParameters>())
{
    winrt::check_bool(bool{ m_fenceEvent });

    if (!Device::IsInitialized())
        Device::Initialize();

    m_fence = VX_DEVICE0->CreateFence(m_fenceValue);
    
    m_commandQueue = VX_DEVICE0->CreateGraphicsCommandQueue();
    m_commandAllocator = VX_DEVICE0->CreateGraphicsCommandAllocator();
    m_commandListBegin = VX_DEVICE0->CreateGraphicsCommandList();
    m_commandListEnd = VX_DEVICE0->CreateGraphicsCommandList();

    m_renderTarget = std::make_unique<RenderTarget>(hWnd, m_commandQueue);
	uint32_t width = m_renderTarget->GetWidth();
	uint32_t height = m_renderTarget->GetHeight();
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(width), static_cast<FLOAT>(height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

    winrt::check_hresult(GameInputCreate(m_gameInput.put()));
    //winrt::check_hresult(RegisterReadingCallback(m_gameMouse, GameInputKindMouse, 0, ));
    
    Device::CreateResourceHeap(VX_0, 4);
    m_constantResource = VX_DEVICE0->CreateConstantResource((sizeof(GlobalParameters) + 255) & ~255);
    m_cbvGpuHandle = VX_DEVICE0->CreateCBV(0, m_constantResource, (sizeof(GlobalParameters) + 255) & ~255);

    WaitForPreviousFrame();
}

void Vortex::Renderer::Execute()
{
    Update();

    winrt::check_hresult(m_commandAllocator->Reset());

    std::vector<ID3D12CommandList*> commandLists;
    // Begin frame.
    {
        winrt::check_hresult(m_commandListBegin->Reset(m_commandAllocator.get(), nullptr));
        m_commandListBegin->ResourceBarrier(1, m_renderTarget->PrepareForRender());
        static const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_commandListBegin->ClearRenderTargetView(m_renderTarget->GetCPUDescriptorHandle(), clearColor, 0, nullptr);
        winrt::check_hresult(m_commandListBegin->Close());
    }
    commandLists.push_back(m_commandListBegin.get());
    
    // Passes frames.
    for (const std::unique_ptr<IRenderPass>& pass : m_passes)
    {
        commandLists.push_back(pass->GetCommandList(this));
    }

    // End frame.
    {
        winrt::check_hresult(m_commandListEnd->Reset(m_commandAllocator.get(), nullptr));
        m_commandListEnd->ResourceBarrier(1, m_renderTarget->PrepareForPresent());
        winrt::check_hresult(m_commandListEnd->Close());
    }
    commandLists.push_back(m_commandListEnd.get());

    // Execute
    m_commandQueue->ExecuteCommandLists(static_cast<uint32_t>(commandLists.size()), commandLists.data());

    m_renderTarget->Flip();

    WaitForPreviousFrame();
}

Vortex::Renderer::~Renderer()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    m_fenceEvent.close();
    //m_gameInput->Release();
    //m_gameInput.detach();
}

void Vortex::Renderer::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const uint64_t fence = m_fenceValue;
    winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        winrt::check_hresult(m_fence->SetEventOnCompletion(fence, m_fenceEvent.get()));
        ::WaitForSingleObject(m_fenceEvent.get(), INFINITE);
    }
}

void Vortex::Renderer::Update()
{
    // Get input.
    winrt::com_ptr<IGameInputReading> reading;
    if (SUCCEEDED(m_gameInput->GetCurrentReading(GameInputKindKeyboard | GameInputKindMouse, nullptr, reading.put())))
    {
        if (!m_gameDevice) reading->GetDevice(m_gameDevice.put());

        m_globalParams->model = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f)));

        std::vector<GameInputKeyState> keyState(reading->GetKeyCount());
        if (reading->GetKeyState(static_cast<uint32_t>(keyState.size()), keyState.data()))
        {
            //UploadTexture();
            if (keyState.front().virtualKey == ' ')
            {
                m_globalParams->model = DirectX::XMMatrixIdentity();
            }
            else if(keyState.front().virtualKey == 'A')
            {
                m_globalParams->model = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f)));
            }


        }

        static float sensitivity = 0.1f;
        GameInputMouseState mouseState;
        if (reading->GetMouseState(&mouseState))
        {
            static GameInputMouseState lastState;
            if (mouseState.buttons & GameInputMouseRightButton)
            {
                if (!(lastState.buttons & GameInputMouseRightButton))
                {
                    lastState = mouseState;
                }

                float yaw = (mouseState.positionX - lastState.positionX) * sensitivity;
                float pitch = (mouseState.positionY - lastState.positionY) * sensitivity;
                yaw = DirectX::XMConvertToRadians(yaw);
                pitch = DirectX::XMConvertToRadians(pitch);
                //m_camera->Yaw(yaw);
                //m_camera->Pitch(pitch);
                m_camera->Rotate(yaw, pitch);
            }
            lastState = mouseState;
        }

        m_globalParams->view = DirectX::XMMatrixTranspose(m_camera->GetView());
        float aspect = m_viewport.Width / m_viewport.Height;
        m_globalParams->projection = DirectX::XMMatrixTranspose(m_camera->GetProjection(aspect));
        m_globalParams->time = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_timeSinceStart).count();
        uint8_t* gpuPtr = nullptr;
        CD3DX12_RANGE range(0, 0);
        winrt::check_hresult(m_constantResource->Map(0, &range, reinterpret_cast<void**>(&gpuPtr)));
        memcpy(gpuPtr, m_globalParams.get(), sizeof(GlobalParameters));
        m_constantResource->Unmap(0, nullptr);
    }
    else if (m_gameDevice)
    {
        m_gameDevice->Release();
        m_gameDevice.detach();
    }
}