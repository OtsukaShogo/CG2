#pragma once
#include<d3d12.h>

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
