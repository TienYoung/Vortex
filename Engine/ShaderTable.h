#pragma once

namespace Vortex
{
	struct alignas(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) ShaderRecord
	{
		uint8_t ShaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
		uint8_t RootArguments[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
	};

	struct alignas(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) ShaderTable
	{
		//ShaderRecord shaderRecord[1];
	};
}