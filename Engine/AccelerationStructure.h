#pragma once

namespace Vortex
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure()
		{
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputDesc =
			{
				.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
				.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
				.NumDescs = 1,
				.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			};

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasPrebuildInfo = {};
			VX_DEVICE0->Get()->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputDesc, &tlasPrebuildInfo);
			winrt::check_bool(tlasPrebuildInfo.ResultDataMaxSizeInBytes > 0);

			// Create Scratch buffer
			{
				D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				winrt::check_hresult(VX_DEVICE0->Get()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(&m_scratchBuffer)
				));
			}

			// Create TLAS buffer
			{
				D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				winrt::check_hresult(VX_DEVICE0->Get()->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
					nullptr,
					IID_PPV_ARGS(&m_tlasBuffer)
				));
			}
		}

 		inline winrt::com_ptr<ID3D12Resource> GetTLAS() const
		{
			return m_tlasBuffer;
		}

	private:
		winrt::com_ptr<ID3D12Resource> m_scratchBuffer;
		winrt::com_ptr<ID3D12Resource> m_tlasBuffer;
	};
}

