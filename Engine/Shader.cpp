#include "pch.h"
#include "Shader.h"

#include <filesystem>
#include <fstream>

const winrt::hstring Vortex::Shader::s_shaderFolder = L"Assets/Shaders";

const winrt::hstring Vortex::Shader::s_cacheFolder = L"Assets/ShaderCache";

namespace fs = std::filesystem;

Vortex::Shader::Shader(const winrt::hstring& name)
	:m_entryPoint(L"main")
{
	std::wstring type = std::wstring(name.c_str()).substr(name.size() - 2, 2);
	if (type == L"VS")
	{
		m_type = Type::Vertex;
	}
	else if (type == L"AS")
	{
		m_type = Type::Amplification;
	}
	else if (type == L"MS")
	{
		m_type = Type::Mesh;
	}
	else if (type == L"PS")
	{
		m_type = Type::Pixel;
	}
	else if (type == L"CS")
	{
		m_type = Type::Compute;
	}
	else
	{
		throw winrt::hresult_error(E_FAIL, L"Unknown shader type");
	}

	fs::path filePath = fs::path(s_shaderFolder.c_str()).append(name.c_str()).replace_extension("hlsl");
	m_sourceFilename = filePath.c_str();
	fs::path cachePath = fs::path(s_cacheFolder.c_str()).append(name.c_str()).replace_extension("cso");
	if (!fs::exists(cachePath.parent_path()))
	{
		fs::create_directories(cachePath.parent_path());
	}
	m_cacheFilename = cachePath.c_str();

	//if (!fs::exists(cachePath) || (fs::last_write_time(cachePath) < fs::last_write_time(filePath)))
	{
		Compile();
	}

	LoadBinary();
}

D3D12_SHADER_BYTECODE Vortex::Shader::GetBytecode() const
{
	return CD3DX12_SHADER_BYTECODE(reinterpret_cast<ID3DBlob*>(m_cacheBinary.get()));
}

void Vortex::Shader::LoadBinary()
{
	// Create DXC objects
	winrt::com_ptr<IDxcUtils> dxcUtils;
	winrt::check_hresult(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.put())));
	
	// Load file
	winrt::check_hresult(dxcUtils->LoadFile(m_cacheFilename.c_str(), nullptr, m_cacheBinary.put()));
}

void Vortex::Shader::Compile()
{
	// Create DXC objects
	winrt::com_ptr<IDxcUtils> dxcUtils;
	winrt::check_hresult(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.put())));
	winrt::com_ptr<IDxcCompiler3> dxcCompiler;
	winrt::check_hresult(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler.put())));
	
	winrt::com_ptr<IDxcIncludeHandler> dxcIncludeHandler;
	dxcUtils->CreateDefaultIncludeHandler(dxcIncludeHandler.put());

	// Load file
	winrt::com_ptr<IDxcBlobEncoding> dxcBlobEncoding;
	UINT32 codePage;
	winrt::check_hresult(dxcUtils->LoadFile(m_sourceFilename.c_str(), &codePage, dxcBlobEncoding.put()));
	DxcBuffer dxcBuffer = {
		dxcBlobEncoding->GetBufferPointer(),
		dxcBlobEncoding->GetBufferSize(),
		DXC_CP_ACP
	};

	// Target profile
	LPCWSTR target = nullptr;
	switch (m_type)
	{
	case Shader::Vertex:
		target = L"vs_6_5";
		break;
	case Shader::Amplification:
		target = L"as_6_5";
		break;
	case Shader::Mesh:
		target = L"ms_6_5";
		break;
	case Shader::Pixel:
		target = L"ps_6_5";
		break;
	case Shader::Compute:
		target = L"cs_6_5";
		break;
	}
	// Build arguments
    LPCWSTR args[]{
        L"-Fo", m_cacheFilename.c_str(),
#if defined(_DEBUG)
        DXC_ARG_DEBUG,
		L"-Qembed_debug",
        DXC_ARG_SKIP_OPTIMIZATIONS,
#endif
    };
	winrt::com_ptr<IDxcCompilerArgs> compilerArgs;
	winrt::check_hresult(dxcUtils->BuildArguments(
		m_sourceFilename.c_str(),
		m_entryPoint.c_str(),
		target, args, _countof(args),
		nullptr, 0,
		compilerArgs.put())
	);

	// Compile
	winrt::com_ptr<IDxcResult> dxcResult;
	winrt::check_hresult(dxcCompiler->Compile(
		&dxcBuffer,
		compilerArgs->GetArguments(), compilerArgs->GetCount(),
		dxcIncludeHandler.get(),
		IID_PPV_ARGS(dxcResult.put()))
	);


	// Check for errors
	HRESULT hrStatus;
	winrt::check_hresult(dxcResult->GetStatus(&hrStatus));
	if (FAILED(hrStatus))
	{
		winrt::com_ptr<IDxcBlobUtf8> pErrors;
		winrt::check_hresult(dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));

		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
			throw winrt::hresult_error(hrStatus, winrt::to_hstring(pErrors->GetStringPointer()));
	}

	winrt::com_ptr<IDxcBlob> dxcBinary;
	winrt::com_ptr<IDxcBlobWide> dxcBinaryName;
	winrt::check_hresult(dxcResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(dxcBinary.put()), dxcBinaryName.put()));
	if (dxcBinary.get() != nullptr)
	{
		std::ofstream binaryfile;
		auto data = dxcBinary->GetBufferPointer();
		binaryfile.open(dxcBinaryName->GetStringPointer(), std::ios_base::out | std::ios_base::binary);
		binaryfile.write(reinterpret_cast<const char*>(data), dxcBinary->GetBufferSize());
		binaryfile.close();
	}
}

