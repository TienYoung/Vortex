#pragma once

#include "Device.h"
#include "Shader.h"
#include "Camera.h"
#include "RenderTarget.h"

namespace Vortex
{
	class Renderer
	{
	public:
		class IRenderPass
		{
		public:
			virtual ~IRenderPass() = default;

            inline virtual ID3D12GraphicsCommandList10* GetCommandList(const Renderer& renderer) const = 0;
            //inline virtual ID3D12DescriptorHeap* GetDescriptorHeap() const = 0;
		};

		//class RenderPass : public IRenderPass
		//{
		//public:
		//	RenderPass()
		//	{
		//		Setup();
		//		Execution();
		//	}
		//protected:
		//	virtual void Setup();
		//	virtual void Equip() const;
		//	virtual void Execution() const;
		//};

	public:
        Renderer() = delete;
        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) = delete;

		Renderer(HWND hWnd);
		~Renderer();

		void WaitForPreviousFrame();

		template<typename T>
        inline void AddPass() { 
			m_passes.push_back(std::make_unique<T>()); 
		}

		void Execute();
		//void PopulateCommandList();

	private:
		void Update();

	private:
		// Synchronization objects.
		winrt::handle m_fenceEvent;
        uint64_t m_fenceValue;
        winrt::com_ptr<ID3D12Fence1> m_fence;


		winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
		winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
		winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandListBegin;
		winrt::com_ptr<ID3D12GraphicsCommandList10> m_commandListEnd;

		std::unique_ptr<RenderTarget> m_renderTarget;

		std::vector<std::unique_ptr<IRenderPass>> m_passes;

		// Viewport dimensions.
		//uint32_t m_width;
		//uint32_t m_height;

		std::shared_ptr<Camera> m_camera;

		// GPU Resources.
		winrt::com_ptr<ID3D12Resource> m_constantResource;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_cbvGpuHandle;
		// CPU Resources.
		std::shared_ptr<GlobalParameters> m_globalParams;
		std::chrono::time_point<std::chrono::steady_clock> m_timeSinceStart;
		// Inputs.
		winrt::com_ptr<IGameInput> m_gameInput;
		winrt::com_ptr<IGameInputDevice> m_gameDevice;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

	public:
        inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGlobalParamsHandle() const { return m_cbvGpuHandle; }
		inline const RenderTarget* GetRenderTarget() const { return m_renderTarget.get(); }
		inline const D3D12_VIEWPORT* GetViewport() const { return &m_viewport; }
		inline const D3D12_RECT* GetScissorRect() const { return &m_scissorRect; }
	};
}