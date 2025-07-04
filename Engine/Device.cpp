#include "pch.h"

#include "Renderer.h"
#include "Device.h"

#include <CompiledShaders/Raytracing.hlsl.h>

winrt::com_ptr<IDXGIFactory6> Vortex::Device::s_dxgiFactory;
std::vector<Vortex::Device> Vortex::Device::s_deviceList;

void Vortex::Device::Initialize()
{
	// The device only can be initialized once!
	WINRT_ASSERT(s_dxgiFactory == nullptr);

	uint32_t dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	{
		winrt::com_ptr<ID3D12Debug1> debugController;
		winrt::check_hresult(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
		//debugController->SetEnableGPUBasedValidation(true);
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	winrt::check_hresult(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&s_dxgiFactory)));

	winrt::com_ptr<IDXGIAdapter3> hardwareAdapter;
	for (uint32_t i = 0; SUCCEEDED(s_dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&hardwareAdapter))); ++i)
	{
		s_deviceList.push_back(hardwareAdapter);
	}
}

Vortex::Device::Device(const winrt::com_ptr<IDXGIAdapter3>& adaptor) :
	m_rtvHandleIndex(0)
{
	winrt::check_hresult(adaptor->GetDesc2(&m_adapterDesc));
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_12_2;
	if (m_adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) // Windows Advanced Rasterization Platform (WARP)
		level = D3D_FEATURE_LEVEL_12_1;
	winrt::check_hresult(D3D12CreateDevice(adaptor.get(), level, IID_PPV_ARGS(&m_d3d12Device)));

	m_rtvHandleSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

winrt::com_ptr<ID3D12Fence1> Vortex::Device::CreateFence(uint64_t value) const
{
	winrt::com_ptr<ID3D12Fence1> fence;
	winrt::check_hresult(m_d3d12Device->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	return fence;
}

winrt::com_ptr<ID3D12CommandQueue> Vortex::Device::CreateGraphicsCommandQueue() const
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	winrt::com_ptr<ID3D12CommandQueue> commandQueue;
	winrt::check_hresult(m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	return commandQueue;
}

winrt::com_ptr<ID3D12CommandQueue> Vortex::Device::CreateComputeCommandQueue() const
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	winrt::com_ptr<ID3D12CommandQueue> commandQueue;
	winrt::check_hresult(m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	return commandQueue;
}

winrt::com_ptr<ID3D12CommandQueue> Vortex::Device::CreateCopyCommandQueue() const
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	winrt::com_ptr<ID3D12CommandQueue> commandQueue;
	winrt::check_hresult(m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	return commandQueue;
}

winrt::com_ptr<ID3D12CommandAllocator> Vortex::Device::CreateGraphicsCommandAllocator() const
{
	winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
	winrt::check_hresult(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

winrt::com_ptr<ID3D12CommandAllocator> Vortex::Device::CreateComputeCommandAllocator() const
{
	winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
	winrt::check_hresult(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

winrt::com_ptr<ID3D12CommandAllocator> Vortex::Device::CreateBundleCommandAllocator() const
{
	winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
	winrt::check_hresult(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

winrt::com_ptr<ID3D12CommandAllocator> Vortex::Device::CreateCopyCommandAllocator() const
{
	winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
	winrt::check_hresult(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

winrt::com_ptr<ID3D12GraphicsCommandList6> Vortex::Device::CreateGraphicsCommandList() const
{
	winrt::com_ptr<ID3D12GraphicsCommandList6> commandList;
	winrt::check_hresult(m_d3d12Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)));
	return commandList;
}

winrt::com_ptr<ID3D12GraphicsCommandList6> Vortex::Device::CreateComputeCommandList() const
{
	winrt::com_ptr<ID3D12GraphicsCommandList6> commandList;
	winrt::check_hresult(m_d3d12Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)));
	return commandList;
}

winrt::com_ptr<ID3D12GraphicsCommandList6> Vortex::Device::CreateBundleCommandList() const
{
	winrt::com_ptr<ID3D12GraphicsCommandList6> commandList;
	winrt::check_hresult(m_d3d12Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)));
	return commandList;
}

winrt::com_ptr<ID3D12GraphicsCommandList6> Vortex::Device::CreateCopyCommandList() const
{
	winrt::com_ptr<ID3D12GraphicsCommandList6> commandList;
	winrt::check_hresult(m_d3d12Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)));
	return commandList;
}

winrt::com_ptr<IDXGISwapChain3> Vortex::Device::CreateSwapChain(HWND hWnd, const winrt::com_ptr<ID3D12CommandQueue>& commandQueue)
{
	// Create a swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = VX_DOUBLE_BUFFER;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	winrt::com_ptr<IDXGISwapChain1> swapChain;
	winrt::check_hresult(s_dxgiFactory->CreateSwapChainForHwnd(commandQueue.get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChain.put()));

	// This sample does not support full screen transitions.
	winrt::check_hresult(s_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = VX_DOUBLE_BUFFER,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0,
	};
	winrt::check_hresult(m_d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	return swapChain.as<IDXGISwapChain3>();
}

winrt::com_ptr<ID3D12RootSignature> Vortex::Device::CreateRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc) const
{
	winrt::com_ptr<ID3DBlob> signature;
	winrt::com_ptr<ID3DBlob> error;
	winrt::check_hresult(D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, signature.put(), error.put()));

	winrt::com_ptr<ID3D12RootSignature> rootSignature;
	winrt::check_hresult(m_d3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
	return rootSignature;
}

winrt::com_ptr<ID3D12PipelineState> Vortex::Device::CreateMeshPSO(
	const winrt::com_ptr<ID3D12RootSignature>& rootSignature,
	const D3D12_SHADER_BYTECODE& mesh, const D3D12_SHADER_BYTECODE& pixel,
	const D3D12_SHADER_BYTECODE& amplification/* = { NULL, 0 }*/) const
{
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.get();
	psoDesc.AS = amplification;
	psoDesc.MS = mesh;
	psoDesc.PS = pixel;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
	psoDesc.SampleMask = DefaultSampleMask();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    // CW front; cull back
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//psoDesc.DSVFormat;
	psoDesc.SampleDesc = DefaultSampleDesc();
	psoDesc.NodeMask = 0;
	psoDesc.CachedPSO = { NULL, 0 };
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);

	winrt::com_ptr<ID3D12PipelineState> pipelineState;
	winrt::check_hresult(m_d3d12Device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipelineState)));
	return pipelineState;
}

winrt::com_ptr<ID3D12PipelineState> Vortex::Device::CreateComputePSO(const winrt::com_ptr<ID3D12RootSignature>& rootSignature, const D3D12_SHADER_BYTECODE& compute) const
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
	computeDesc.pRootSignature = rootSignature.get();
	computeDesc.CS = compute;
	computeDesc.NodeMask = 0;
	computeDesc.CachedPSO = { NULL, 0 };
	computeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	winrt::com_ptr<ID3D12PipelineState> pipelineState;
	winrt::check_hresult(m_d3d12Device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&pipelineState)));
	return pipelineState;
}

//winrt::com_ptr<ID3D12PipelineState> Vortex::Device::CreateRayTracingPSO(const winrt::com_ptr<ID3D12RootSignature>& rootSignature, const D3D12_SHADER_BYTECODE& raygen) const
//{
//    CD3DX12_STATE_OBJECT_DESC dxrStateObjectDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
//	// Add compiled shaders
//    auto lib = dxrStateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
//    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
//    lib->SetDXILLibrary(&libdxil);
//    lib->DefineExport(L"MyRaygenShader");
//    lib->DefineExport(L"MyMissShader");
//
//    // Create DXR PSO
//	winrt::com_ptr<ID3D12StateObject> dxrStateObject;
//	winrt::check_hresult(m_d3d12Device->CreateStateObject(dxrStateObjectDesc, IID_PPV_ARGS(&dxrStateObject)));
//
//    // Build Acceleration Struct
//
//	winrt::com_ptr<ID3D12StateObjectProperties> rtpso;
//	rtpso = dxrStateObject.as<ID3D12StateObjectProperties>();
//	rtpso->GetShaderIdentifier(L"raygen_main");
//	rtpso->GetShaderIdentifier(L"miss_main");
//}

winrt::com_ptr<ID3D12DescriptorHeap> Vortex::Device::CreateResourceHeap(uint32_t num) const
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = num;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	winrt::com_ptr<ID3D12DescriptorHeap> resourceHeap;
	winrt::check_hresult(m_d3d12Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&resourceHeap)));
	return resourceHeap;
}

void Vortex::Device::CreateResourceHeap(uint32_t deviceId, uint32_t num)
{
	WINRT_ASSERT(deviceId == 0);
	s_deviceList[deviceId].m_resourceHeap = s_deviceList[deviceId].CreateResourceHeap(num);
}

winrt::com_ptr<ID3D12Resource> Vortex::Device::CreateConstantResource(uint32_t sizeInBytes) const
{
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);
	winrt::com_ptr<ID3D12Resource> resource;
	winrt::check_hresult(m_d3d12Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resource)
	));
	return resource;
}

winrt::com_ptr<ID3D12Resource> Vortex::Device::CreateTextureResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const
{
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	winrt::com_ptr<ID3D12Resource> resource;
	winrt::check_hresult(m_d3d12Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&resource)
	));
	return resource;
}


winrt::com_ptr<ID3D12Resource> Vortex::Device::CreateTextureCubeResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const
{
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 6, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	winrt::com_ptr<ID3D12Resource> resource;
	winrt::check_hresult(m_d3d12Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&resource)
	));
	return resource;
}

winrt::com_ptr<ID3D12Resource> Vortex::Device::CreateUnorderedResource(DXGI_FORMAT format, uint64_t width, uint32_t height) const
{
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	winrt::com_ptr<ID3D12Resource> resource;
	winrt::check_hresult(m_d3d12Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&resource)
	));
	return resource;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateCBV(
	const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
	const winrt::com_ptr<ID3D12Resource>& resource, uint32_t sizeInBytes) const
{
	uint32_t descriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeInBytes;
	m_d3d12Device->CreateConstantBufferView(&cbvDesc, descriptorHandle);

	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);
}


CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateCBV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, uint32_t sizeInBytes) const
{
	return CreateCBV(m_resourceHeap, index, resource, sizeInBytes);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateSRV(
	const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
	const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const
{
	uint32_t descriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_d3d12Device->CreateShaderResourceView(resource.get(), &srvDesc, descriptorHandle);

	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateSRV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const
{
	return CreateSRV(m_resourceHeap, index, resource, format);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateUAV(
	const winrt::com_ptr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t index,
	const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const
{
	uint32_t descriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;
	m_d3d12Device->CreateUnorderedAccessView(resource.get(), nullptr, &uavDesc, descriptorHandle);

	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Vortex::Device::CreateUAV(uint32_t index, const winrt::com_ptr<ID3D12Resource>& resource, DXGI_FORMAT format) const
{
	return CreateUAV(m_resourceHeap, index, resource, format);
}

D3D12_CPU_DESCRIPTOR_HANDLE Vortex::Device::CreateRTV(const winrt::com_ptr<ID3D12Resource>& resource)
{
	WINRT_ASSERT(m_rtvHandleIndex < VX_DOUBLE_BUFFER);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_rtvHandleIndex, m_rtvHandleSize };
	m_d3d12Device->CreateRenderTargetView(resource.get(), nullptr, rtvHandle);
	m_rtvHandleIndex++;
	return rtvHandle;
}