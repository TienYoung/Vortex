#pragma once

#define VX_DOUBLE_BUFFER 2

#define VX_DEVICE0 Vortex::Device::GetDevice0()
#define VX_0 0

namespace Vortex
{
	class Device
	{
	public:
		// Functions
		static void Initialize();
		static inline bool IsInitialized() { return s_dxgiFactory != nullptr; }
        static inline size_t Count() { return s_deviceList.size(); }
        static inline Device* GetDevice0() { WINRT_ASSERT(s_deviceList.size() > 0); return &s_deviceList[0]; }
	private:
		// Variables
		static winrt::com_ptr<IDXGIFactory6> s_dxgiFactory;
		static std::vector<Device> s_deviceList;

	private:
		// Constructors
		Device() = delete;
		Device(const winrt::com_ptr<IDXGIAdapter3>& adaptor);
	public:
		// Getters
        inline winrt::hstring GetDescription() const { return m_adapterDesc.Description; }
	private:
		// Variables
		DXGI_ADAPTER_DESC2 m_adapterDesc;
		winrt::com_ptr<ID3D12Device5> m_d3d12Device;
	public:
		// Wrapped functions
		winrt::com_ptr<ID3D12Fence1> CreateFence(uint64_t value) const;

		winrt::com_ptr<ID3D12CommandQueue> CreateGraphicsCommandQueue() const;
		winrt::com_ptr<ID3D12CommandQueue> CreateComputeCommandQueue() const;
		winrt::com_ptr<ID3D12CommandQueue> CreateCopyCommandQueue() const;
		winrt::com_ptr<ID3D12CommandAllocator> CreateGraphicsCommandAllocator() const;
		winrt::com_ptr<ID3D12CommandAllocator> CreateComputeCommandAllocator() const;
		winrt::com_ptr<ID3D12CommandAllocator> CreateBundleCommandAllocator() const;
		winrt::com_ptr<ID3D12CommandAllocator> CreateCopyCommandAllocator() const;
        winrt::com_ptr<ID3D12GraphicsCommandList6> CreateGraphicsCommandList() const;
        winrt::com_ptr<ID3D12GraphicsCommandList6> CreateComputeCommandList() const;
        winrt::com_ptr<ID3D12GraphicsCommandList6> CreateBundleCommandList() const;
        winrt::com_ptr<ID3D12GraphicsCommandList6> CreateCopyCommandList() const;

		winrt::com_ptr<IDXGISwapChain3> CreateSwapChain(HWND hWnd, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue);

		winrt::com_ptr<ID3D12RootSignature> CreateRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc) const;

        winrt::com_ptr<ID3D12PipelineState> CreateMeshPSO(
            const winrt::com_ptr<ID3D12RootSignature>& rootSignature,
            const D3D12_SHADER_BYTECODE& mesh, const D3D12_SHADER_BYTECODE& pixel,
            const D3D12_SHADER_BYTECODE& amplification = { NULL, 0 }) const;

        winrt::com_ptr<ID3D12PipelineState> CreateComputePSO(
            const winrt::com_ptr<ID3D12RootSignature>& rootSignature,
            const D3D12_SHADER_BYTECODE& compute) const;

		winrt::com_ptr<ID3D12PipelineState> CreateRayTracingPSO(
			const winrt::com_ptr<ID3D12RootSignature>& rootSignature,
			const D3D12_SHADER_BYTECODE& raygen) const;


		winrt::com_ptr<ID3D12DescriptorHeap> CreateResourceHeap(uint32_t num) const;
		
		static void CreateResourceHeap(uint32_t deviceId, uint32_t num);

		winrt::com_ptr<ID3D12Resource> CreateConstantResource(uint32_t sizeInBytes) const;
		
		winrt::com_ptr<ID3D12Resource> CreateTextureResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const;
		
		winrt::com_ptr<ID3D12Resource> CreateTextureCubeResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const;

		winrt::com_ptr<ID3D12Resource> CreateUnorderedResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE CreateCBV(
			const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
			const winrt::com_ptr<ID3D12Resource>& resource, uint32_t sizeInBytes) const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE CreateCBV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, uint32_t sizeInBytes) const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE CreateSRV(
			const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
            const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const;

        CD3DX12_GPU_DESCRIPTOR_HANDLE CreateSRV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE CreateUAV(
			const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
			const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const;

        CD3DX12_GPU_DESCRIPTOR_HANDLE CreateUAV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const;

        D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(const winrt::com_ptr<ID3D12Resource>& resource);


		winrt::com_ptr<ID3D12DescriptorHeap> CreateSamplerHeap();
		winrt::com_ptr<ID3D12DescriptorHeap> CreateDSVHeap();

	private:
		// GPU Resources
		winrt::com_ptr<ID3D12DescriptorHeap> m_rtvHeap;
		uint32_t m_rtvHandleSize;
		int32_t m_rtvHandleIndex;

		winrt::com_ptr<ID3D12DescriptorHeap> m_resourceHeap;
		winrt::com_ptr<ID3D12DescriptorHeap> m_samplerHeap;
		winrt::com_ptr<ID3D12DescriptorHeap> m_computeHeap;

	public:
		// Accessors
        inline std::vector<ID3D12DescriptorHeap*> GetHeaps() {
			std::vector<ID3D12DescriptorHeap*> heaps;
			heaps.push_back(m_resourceHeap.get());
			return heaps;
		}
		inline winrt::com_ptr<ID3D12DescriptorHeap> GetResourceHeap() { return m_resourceHeap; }
	};
}