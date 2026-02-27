//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "Stage.h"

#define _WITH_Scene_ROOT_SIGNATURE

CShader::CShader()
{
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	if (m_pd3dPipelineState)
		m_pd3dPipelineState->Release();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char* pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{
	UINT nReadBytes = 0;
#ifdef _WITH_WFOPEN
	FILE* pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE* pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	nReadBytes = (UINT)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
#endif
#ifdef _WITH_STD_STREAM
	std::ifstream ifsFile;
	ifsFile.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	nReadBytes = (int)ifsFile.tellg();
	BYTE* pByteCode = new BYTE[*pnReadBytes];
	ifsFile.seekg(0);
	ifsFile.read((char*)pByteCode, nReadBytes);
	ifsFile.close();
#endif

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	if (ppd3dShaderBlob)
	{
		*ppd3dShaderBlob = NULL;
		HRESULT hResult = D3DCreateBlob(nReadBytes, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode, nReadBytes);
		d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	}
	else
	{
		d3dShaderByteCode.BytecodeLength = nReadBytes;
		d3dShaderByteCode.pShaderBytecode = pByteCode;
	}

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	//d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	//d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}


void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	m_d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	for (UINT i{}; i < nRenderTargets; ++i)
		m_d3dPipelineStateDesc.RTVFormats[i] = (pdxgiRtvFormats) ? pdxgiRtvFormats[i] : DXGI_FORMAT_R8G8B8A8_UNORM;

	m_d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

	if (m_pd3dVertexShaderBlob)
		m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob)
		m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}


void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	if (m_pd3dPipelineState)
		pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	OnPrepareRender(pd3dCommandList);
	UpdateShaderVariables(pd3dCommandList, pContext);
}

void CShader::CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	if (hResult == DXGI_ERROR_DEVICE_REMOVED)
	{
		// GPU 장치 인스턴스가 중단되었으므로, 정확한 이유를 얻기 위해 GetDeviceRemovedReason 함수 사용
		HRESULT removedReason = pd3dDevice->GetDeviceRemovedReason();

		// removedReason을 사용하여 적절한 작업 수행
		// 예: 오류 메시지 출력, 오류 로그 기록, 재시작 등

		printf("Error1: removedReason 0x%X\n", removedReason);
		// 또는 printf("Error: %s\n", DXGetErrorString(hResult)); 와 같이 DXGetErrorString 함수를 사용하여 HRESULT를 문자열로 변환할 수도 있습니다.
	}

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nShaderResourceViews);
	m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nShaderResourceViews);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	m_d3dUavGPUDescriptorNextHandle = m_d3dUavGPUDescriptorStartHandle;
}

void CShader::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
}

void CShader::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j{}; j < nConstantBufferViews; ++j)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i{}; i < nTextures; ++i)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		if (pShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i{}; i < nRootParameters; ++i) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)//0730
{
	for (int i{}; i < nResources; ++i)
	{
		if (ppd3dResources[i])
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pdxgiSrvFormats[i];
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			pd3dDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
}
void CShader::CreateComputeUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors)
{
	m_d3dUavCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dUavGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	for (UINT i{}; i < nDescriptors; ++i)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(nTextureIndex + i);
		if (pShaderResource)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = pTexture->GetUnorderedAccessViewDesc(nTextureIndex + i);
			pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL, &d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
			pTexture->SetComputeUavGpuDescriptorHandle(nHandleIndex + i, m_d3dUavGPUDescriptorNextHandle);
			m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardShader::CStandardShader()
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}


D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader()
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardObjectsShader::CStandardObjectsShader()
{
}

CStandardObjectsShader::~CStandardObjectsShader()
{
}

void CStandardObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
}

void CStandardObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j{}; j < m_nObjects; ++j)
			if (m_ppObjects[j])
				m_ppObjects[j]->Release();

		delete[] m_ppObjects;
	}
}

void CStandardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CStandardObjectsShader::ReleaseUploadBuffers()
{
	for (int j{}; j < m_nObjects; ++j)
		if (m_ppObjects[j])
			m_ppObjects[j]->ReleaseUploadBuffers();
}

void CStandardObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CStandardShader::Render(pd3dCommandList, pCamera, false);

	for (int j{}; j < m_nObjects; ++j)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime, false);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
	float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
	xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
	xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
	xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j{}; j < m_nObjects; ++j)
			if (m_ppObjects[j])
				m_ppObjects[j]->Release();

		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j{}; j < m_nObjects; ++j)
		if (m_ppObjects[j])
			m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera, false);

	for (int j{}; j < m_nObjects; ++j)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime, false);
			m_ppObjects[j]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
		}
	}
}


//===============================================================================================================================================

CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
#ifdef _WITH_Scene_ROOT_SIGNATURE
	m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	m_pd3dGraphicsRootSignature->AddRef();
#else
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
#endif

	CShader::CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat);
}

D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };



	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}


D3D12_SHADER_BYTECODE CObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256�� ���
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j{}; j < m_nObjects; ++j)
	{
		CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, DirectX::XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
#ifdef _WITH_BATCH_MATERIAL
		if (m_pMaterial) pbMappedcbGameObject->m_nMaterialID = m_pMaterial->m_nReflection;
		if (m_pMaterial) pbMappedcbGameObject->m_nObjectID = j;
#endif
	}
}

void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CShader::ReleaseShaderVariables();
}

vector<XMFLOAT3> CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	char* pstrFileName, CBoxShader* boxShader, void* pContext)
{
	int nSceneTextures = 0;

	m_ppObjects = ::LoadGameObjectsFromFile(pd3dDevice, pd3dCommandList, pstrFileName, &m_nObjects);

	mpObjVec.resize(0);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	if (0 == strcmp("Models/Scene.bin", pstrFileName))
	{
		for (int i{}; i < m_nObjects; ++i)
		{

			/*m_ppObjects[i]->m_xmOOBB = BoundingOrientedBox(XMFLOAT3(m_ppObjects[i]->GetPosition().x, m_ppObjects[i]->GetPosition().y,
				m_ppObjects[i]->GetPosition().z), XMFLOAT3(10, 10, 10), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));*/



			if (strcmp(m_ppObjects[i]->m_pstrName, "ForDoorcollider")
				&& strcmp(m_ppObjects[i]->m_pstrName, "Bedroom_wall_b_01_dense_mesh"))//Bedroom_wall_b_01_dense_mesh
			{
				boxShader->obj.push_back(m_ppObjects[i]);
			}



			if (0 == strcmp(m_ppObjects[i]->m_pstrName, "Candle1") ||
				0 == strcmp(m_ppObjects[i]->m_pstrName, "Candle2") ||
				0 == strcmp(m_ppObjects[i]->m_pstrName, "Candle3"))
			{

				XMFLOAT3 xmf3tmp = m_ppObjects[i]->GetPosition();
				mpObjVec.push_back(xmf3tmp);
			}
		}
	}


	m_nDoor = 7;
	door = new CGameObject * [m_nDoor];

	for (int h{}; h < m_nDoor; ++h)
	{
		door[h] = new CDoor(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, nullptr, 2);
		door[h]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		door[h]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);

		door[h]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		door[h]->m_pSkinnedAnimationController->SetTrackEnable(1, false);


		door[h]->SetScale(70.0f, 70.0f, 70.0f);
		memcpy_s(door[h]->m_pstrName, 64, "door", 64);

		switch (h)
		{
		case 0:
			/*door[h]->SetPosition(469, -64.6, 1235);*/
			door[h]->SetPosition(469.f, -43.f, 1240.f);
			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(469.f, -64.6f, 1240.f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f));
			/*door[h]->obBox = BoundingOrientedBox(XMFLOAT3(469.f, -64.6f, 1240.f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));*/
			break;
		case 1:
			door[h]->SetPosition(475.f, -43.f, 2591.5f);
			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(475.f, -64.6f, 2591.5f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			break;
		case 2:	// 2번 인덱스 문제 때문에 배치한 허수값. 실제론 없는 문
			door[h]->SetPosition(222.f, -28200.f, 3601.2004f);
			//door[h]->obBox = BoundingOrientedBox(XMFLOAT3(222.f, -300.f, 3601.2004f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(0.f, 0.f, 0.2004f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			break;
		case 3:
			door[h]->SetPosition(222.f, -282.f, 3601.2004f);

			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(222.f, -300.f, 3601.2004f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			break;
		case 4:
			door[h]->SetPosition(471.f, -282.f, 2588.7f);
			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(471.f, -300.f, 2588.7f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			break;
		case 5:
			door[h]->SetPosition(231.f, -282.f, 1239.2f);

			door[h]->obBox = BoundingOrientedBox(XMFLOAT3(231.f, -300.f, 1239.2f), XMFLOAT3(90.f, 50.f, 4.99f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			break;
		}

		if (-150 > door[h]->GetPosition().y)
		{
			door[h]->Rotate(0, 180, 0);
		}
		pDoor.push_back(door[h]);

		//cout << door[h]->obBox.Center.x << ",	" << door[h]->obBox.Center.y << ",	" << door[h]->obBox.Center.z << endl;

		boxShader->obj.push_back(door[h]);
	}


	//cout << "mnobj : " << m_nObjects << endl;

	return mpObjVec;
}

void CObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j{}; j < m_nObjects; ++j)
			if (m_ppObjects[j])
				delete m_ppObjects[j];

		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial)
		m_pMaterial->Release();
#endif
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (int j{}; j < m_nObjects; ++j)
		m_ppObjects[j]->Animate(fTimeElapsed, false);
}

void CObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j{}; j < m_nObjects; ++j)
			if (m_ppObjects[j])
				m_ppObjects[j]->ReleaseUploadBuffers();
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial)
		m_pMaterial->ReleaseUploadBuffers();
#endif
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	CShader::Render(pd3dCommandList, pCamera);//�� ���̴� ����

	for (int j{}; j < m_nObjects; ++j)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
		}
	}
}

//=====================================================================================================================
CShadowMapShader::CShadowMapShader(CBoxShader* pObjectsShader)
{
	m_pObjectsShader = pObjectsShader;
}

CShadowMapShader::~CShadowMapShader()
{
}

D3D12_DEPTH_STENCIL_DESC CShadowMapShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSShadowMapShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSShadowMapShadow", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CShadowMapShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShadowMapShader::ReleaseShaderVariables()
{
}

void CShadowMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	if (m_pDepthTexture)
		m_pDepthTexture->UpdateShaderVariables(pd3dCommandList);
}

void CShadowMapShader::ReleaseUploadBuffers()
{
}

void CShadowMapShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	m_pDepthTexture = (CTexture*)pContext;
	m_pDepthTexture->AddRef();

	CStage::CreateShaderResourceViews(pd3dDevice, m_pDepthTexture, 0, 10);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CShadowMapShader::ReleaseObjects()
{
	if (m_pDepthTexture)
		m_pDepthTexture->Release();
}

void CShadowMapShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, vector<CMonster*> Monsters, vector<CPlayer*> Players, LIGHT* light, bool ch)
{
	//CShader::Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if (ch)
	{
		for (auto& o : m_pObjectsShader->obj)
		{
			if (!o->m_bGetItem)
			{
				if (-70 < o->GetPosition().y)
				{
					if (-70 <= Players[0]->GetPosition().y)//2��
						light[0].m_xmf3Position = XMFLOAT3(562, 140.0f, 2300);
				}
				else
				{
					if (-70 > Players[0]->GetPosition().y)//1��
						light[0].m_xmf3Position = XMFLOAT3(562, -30.0f, 2300);
				}

				if (0 == strcmp(o->m_pstrName, "Dense_Floor_mesh"))
					o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);

				if (0 == strcmp(o->m_pstrName, "Stair_step_01_mesh"))
					o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
			}
		}

		for (const auto& monster : Monsters)
		{
			if (monster->c_id > -1)
			{
				if (-70 < monster->GetPosition().y)//몬스터 2층
				{
					if (-70 <= Players[0]->GetPosition().y)//플레이어 2층
					{
						monster->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
						monster->m_ppHat->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
				}
				else//몬스터 1층
				{
					if (-70 > Players[0]->GetPosition().y)//플레이어 1층
					{
						monster->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
						monster->m_ppHat->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
				}
			}
		}

		for (int i{}; i < Players.size(); ++i)
		{
			if (Players[i]->c_id > -1 && 0 != i)
			{
				Players[i]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				Players[i]->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
			}
		}
	}
	else if (Players[0]->c_id > -1)
	{
		Players[0]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
		Players[0]->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
	}
}
//===================================================================================================================

CIlluminatedShader::CIlluminatedShader()
{
}

CIlluminatedShader::~CIlluminatedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSLighting", "vs_5_1", &m_pd3dVertexShaderBlob));
}


D3D12_SHADER_BYTECODE CIlluminatedShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSLighting", "ps_5_1", &m_pd3dPixelShaderBlob));
}
//================================================================================================================================================

CDepthRenderShader::CDepthRenderShader(CBoxShader* pObjectsShader, LIGHT* pLights)
{
	m_pObjectsShader = pObjectsShader;

	m_pLights = pLights;
	m_pToLightSpaces = new TOLIGHTSPACES;

	XMFLOAT4X4 xmf4x4ToTexture = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f };
	m_xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
}

CDepthRenderShader::~CDepthRenderShader()
{
	if (m_pToLightSpaces) delete m_pToLightSpaces;
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDepthWriteShader", "ps_5_1", &m_pd3dPixelShaderBlob));
}

D3D12_DEPTH_STENCIL_DESC CDepthRenderShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //D3D12_COMPARISON_FUNC_LESS_EQUAL
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_RASTERIZER_DESC CDepthRenderShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
#ifdef _WITH_RASTERIZER_DEPTH_BIAS
	d3dRasterizerDesc.DepthBias = 250000;
#endif
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

void CDepthRenderShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = MAX_DEPTH_TEXTURES;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	m_pDepthTexture = new CTexture(MAX_DEPTH_TEXTURES, RESOURCE_TEXTURE2D_ARRAY, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R32_FLOAT, { 0.0f, 0.0f, 0.0f, 0.0f } };
	//D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 0.0f } };


	for (UINT i{}; i < MAX_DEPTH_TEXTURES; ++i)
		m_pDepthTexture->CreateTexture(pd3dDevice, pd3dCommandList, i, RESOURCE_TEXTURE2D, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, 1, 0, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue);
	//m_pDepthTexture->CreateTexture(pd3dDevice, pd3dCommandList, i, RESOURCE_TEXTURE2D, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, 1, 0, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	//d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i{}; i < MAX_DEPTH_TEXTURES; ++i)
	{
		ID3D12Resource* pd3dTextureResource = m_pDepthTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);

	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = _DEPTH_BUFFER_WIDTH;
	d3dResourceDesc.Height = _DEPTH_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	HRESULT h = pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_d3dDsvDescriptorCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pd3dDevice->CreateDepthStencilView(m_pd3dDepthBuffer, &d3dDepthStencilViewDesc, m_d3dDsvDescriptorCPUHandle);

	for (int i{}; i < MAX_DEPTH_TEXTURES; ++i)
	{
		m_ppDepthRenderCameras[i] = new CCamera();
		m_ppDepthRenderCameras[i]->SetViewport(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_ppDepthRenderCameras[i]->SetScissorRect(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT);
		m_ppDepthRenderCameras[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CDepthRenderShader::ReleaseObjects()
{
	for (int i{}; i < MAX_DEPTH_TEXTURES; ++i)
	{
		if (m_ppDepthRenderCameras[i])
		{
			m_ppDepthRenderCameras[i]->ReleaseShaderVariables();
			delete m_ppDepthRenderCameras[i];
		}
	}

	if (m_pDepthTexture)
		m_pDepthTexture->Release();
	if (m_pd3dDepthBuffer)
		m_pd3dDepthBuffer->Release();

	if (m_pd3dRtvDescriptorHeap)
		m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDsvDescriptorHeap)
		m_pd3dDsvDescriptorHeap->Release();
}

void CDepthRenderShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbDepthElementBytes;

	ncbDepthElementBytes = ((sizeof(TOLIGHTSPACES) + 255) & ~255); //256�� ���
	m_pd3dcbToLightSpaces = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbDepthElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbToLightSpaces->Map(0, NULL, (void**)&m_pcbMappedToLightSpaces);
}

void CDepthRenderShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	::memcpy(m_pcbMappedToLightSpaces, m_pToLightSpaces, sizeof(TOLIGHTSPACES));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbToLightGpuVirtualAddress = m_pd3dcbToLightSpaces->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(13, d3dcbToLightGpuVirtualAddress); //ToLight
}

void CDepthRenderShader::ReleaseShaderVariables()
{
	if (m_pd3dcbToLightSpaces)
	{
		m_pd3dcbToLightSpaces->Unmap(0, NULL);
		m_pd3dcbToLightSpaces->Release();
	}
}


//동적 그림자 처리를 위해 ShadowMap 구현
//라이트 위치에 가상의 카메라를 두고,
//해당 시점에서 장면을 Depth 텍스처에 렌더링
//이후 메인 렌더링 단계에서 현재 픽셀을 라이트 좌표계로 변환해
//ShadowMap의 Depth 값과 비교하여 그림자 여부를 판별함.
void CDepthRenderShader::PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList, LIGHT* m_pLights, vector<CMonster*> Monsters, vector<CPlayer*> Players)
{
	for (int j{}; j < MAX_SHADOW_LIGHTS; ++j)
	{
		if (m_pLights[j].m_bEnable)
		{
			XMFLOAT3 xmf3Position = m_pLights[j].m_xmf3Position;
			XMFLOAT3 xmf3Look = m_pLights[j].m_xmf3Direction;
			XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, +1.0f, 0.0f);

			XMMATRIX xmmtxView = DirectX::XMMatrixLookToLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Up));//assertion failed

			float fNearPlaneDistance = 10.0f, fFarPlaneDistance = m_pLights[j].m_fRange;


		/*	Directional Light는 태양처럼 광선이 평행하게 들어오기 때문에 원근 왜곡이 없으며, 
				따라서 Orthographic Projection을 사용함.
				반면 Spot Light는 특정 위치에서 원뿔 형태로 퍼지기 때문에 
				시야각을 반영해야 하므로 Perspective Projection을 사용함.*/

			XMMATRIX xmmtxProjection;
			if (m_pLights[j].m_nType == DIRECTIONAL_LIGHT)
			{
				float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
				xmmtxProjection = DirectX::XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);
				//float fLeft = -(_PLANE_WIDTH * 0.5f), fRight = +(_PLANE_WIDTH * 0.5f), fTop = +(_PLANE_HEIGHT * 0.5f), fBottom = -(_PLANE_HEIGHT * 0.5f);
				//xmmtxProjection = XMMatrixOrthographicOffCenterLH(fLeft * 6.0f, fRight * 6.0f, fBottom * 6.0f, fTop * 6.0f, fBack, fFront);
			}
			else if (m_pLights[j].m_nType == SPOT_LIGHT)
			{
				float fFovAngle = 60.0f; // m_pLights->m_pLights[j].m_fPhi = cos(60.0f);
				float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
				xmmtxProjection = DirectX::XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
			}
			else if (m_pLights[j].m_nType == POINT_LIGHT)
			{
				//ShadowMap[6]
			}

			m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);

			XMMATRIX xmmtxToTexture = DirectX::XMMatrixTranspose(xmmtxView * xmmtxProjection * m_xmProjectionToTexture);
			XMStoreFloat4x4(&m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4x4ToTexture, xmmtxToTexture);

			//m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);
			m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(-100, 512, 0, 1.0f);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

			FLOAT pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			pd3dCommandList->ClearRenderTargetView(m_pd3dRtvCPUDescriptorHandles[j], pfClearColor, 0, NULL);

			pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

			pd3dCommandList->OMSetRenderTargets(1 + j, &m_pd3dRtvCPUDescriptorHandles[j], TRUE, &m_d3dDsvDescriptorCPUHandle);

			Render(pd3dCommandList, m_ppDepthRenderCameras[j], Monsters, Players);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		}
		else
		{
			m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position.w = 0.0f;
		}
	}
}

void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, vector<CMonster*> Monsters, vector<CPlayer*> Players)
{
	CShader::Render(pd3dCommandList, pCamera);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (const auto& monster : Monsters)
	{
		if (monster->c_id > -1)
		{
			if (-70 < monster->GetPosition().y)//몬스터 2층
			{
				if (-70 <= Players[0]->GetPosition().y)//플레이어 2층
				{
					monster->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					monster->m_ppHat->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
			else //몬스터 1층
			{
				if (-299 < Players[0]->GetPosition().y
					&& -70 > Players[0]->GetPosition().y)
				{
					if (330 > Players[0]->GetPosition().x)
					{
						monster->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
						monster->m_ppHat->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
				}
				else if (-299 >= Players[0]->GetPosition().y)//플레이어 1층
				{
					monster->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					monster->m_ppHat->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
		}
	}

	for (auto& player : Players)
	{
		if (player->c_id > -1)
		{
			if (-70 <= Players[0]->GetPosition().y)//플레이어1 2층
			{
				if (-70 < player->GetPosition().y)//다른 플레이어 2층
				{
					player->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					player->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
			else if (-299 < Players[0]->GetPosition().y
				&& -70 > Players[0]->GetPosition().y)//플레이어1 계단
			{
				if (330 > Players[0]->GetPosition().x
					|| -272 > player->GetPosition().y)
				{
					player->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					player->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
			else//플레이어1 1층
			{
				if (-299 >= player->GetPosition().y)//다른 플레이어 1층
				{
					player->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					player->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
		}

		Players[0]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
		Players[0]->m_ppBullet->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
	}

	for (auto& o : m_pObjectsShader->obj)
	{
		if (strcmp(o->m_pstrName, "Dense_Floor_mesh")
			&& strcmp(o->m_pstrName, "Stair_step_01_mesh")
			&& !o->m_bGetItem)
		{

			if (-70 < o->GetPosition().y)
			{
				if (-70 <= Players[0]->GetPosition().y)
					o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
			}
			else
			{
				if (-70 > Players[0]->GetPosition().y)
				{
					if (0 == strncmp(o->m_pstrName, "Candle", 6))
					{
						if (-200 < o->GetPosition().y)
						{
							if (-299 < Players[0]->GetPosition().y)
								o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
						}
						else
							o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
					else if (0 == strcmp(o->m_pstrName, "Stair_side_01_mesh"))
					{
						if (-299 > Players[0]->GetPosition().y || 330 > Players[0]->GetPosition().x)
							o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
					else if (0 == strncmp(o->m_pstrName, "Paper", 5))
					{
						if (-299 > Players[0]->GetPosition().y)
							o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
					}
					else
						o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, NULL, pCamera);
				}
			}
		}
	}
}

//====================================================================================================================================

void CBoxShader::ReleaseObjects()
{
	for (const auto& o : obj)
	{
		delete o;
	}
}

void CBoxShader::AnimateObjects(float fTimeElapsed)
{
	for (const auto& o : obj)
	{
		o->Animate(fTimeElapsed, false);
	}
}

void CBoxShader::ReleaseUploadBuffers()
{
	for (const auto& o : obj)
		o->ReleaseUploadBuffers();
}

void CBoxShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CIlluminatedShader::Render(pd3dCommandList, pCamera);


	for (const auto& o : obj)
	{
		o->UpdateShaderVariables(pd3dCommandList);
		o->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
	}
}

void CBoxShader::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
//==============================================================================================================================

CTextureToViewportShader::CTextureToViewportShader()
{
}

CTextureToViewportShader::~CTextureToViewportShader()
{
}

D3D12_DEPTH_STENCIL_DESC CTextureToViewportShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextureToViewport", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextureToViewport", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CTextureToViewportShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CTextureToViewportShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
}

void CTextureToViewportShader::ReleaseObjects()
{
}

bool CTextureToViewportShader::IsInPlayerView(XMFLOAT3 playerPosition, XMFLOAT3 playerLookVector, XMFLOAT3 monsterPosition, float fieldOfViewAngle)
{
	XMVECTOR playerPosVec = XMLoadFloat3(&playerPosition);
	XMVECTOR playerLookVec = XMLoadFloat3(&playerLookVector);
	XMVECTOR monsterPosVec = XMLoadFloat3(&monsterPosition);

	XMVECTOR directionToMonster = XMVectorSubtract(monsterPosVec, playerPosVec);
	XMVECTOR normalizedDirectionToMonster = XMVector3Normalize(directionToMonster);

	float dotProduct = XMVectorGetX(XMVector3Dot(normalizedDirectionToMonster, playerLookVec));
	float angle = std::acosf(dotProduct);

	// 시야 각도보다 작은 경우에만 true를 반환
	return angle <= fieldOfViewAngle;
}

XMFLOAT2 CTextureToViewportShader::WorldToScreen(XMFLOAT3 worldPosition, CCamera* pCamera, int screenWidth, int screenHeight)
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&pCamera->GetViewMatrix());
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&pCamera->GetProjectionMatrix());

	XMMATRIX viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

	XMVECTOR worldPositionVec = XMLoadFloat3(&worldPosition);

	// 월드 좌표를 카메라 좌표로 변환
	XMVECTOR cameraPositionVec = XMVector3TransformCoord(worldPositionVec, viewMatrix);

	// 카메라 좌표를 클립 좌표로 변환
	XMVECTOR clipPositionVec = XMVector3TransformCoord(cameraPositionVec, projectionMatrix);

	// 클립 좌표를 NDC로 변환
	XMFLOAT4 ndcPosition;
	XMStoreFloat4(&ndcPosition, clipPositionVec);
	ndcPosition.x /= ndcPosition.w;
	ndcPosition.y /= ndcPosition.w;

	// NDC를 스크린 좌표로 변환
	XMFLOAT2 screenPosition;
	screenPosition.x = (ndcPosition.x + 1.0f) * 0.5f * screenWidth;
	screenPosition.y = (1.0f - ndcPosition.y) * 0.5f * screenHeight;

	return screenPosition;
}

void CTextureToViewportShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, float plHp, XMFLOAT2 pos)
{
	
	D3D12_RECT d3dScissorRect = { pos.x,pos.y, pos.x + plHp, pos.y + 15.f };


	pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);

	CShader::Render(pd3dCommandList, pCamera);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}



//=======================================================================================================
CMultiSpriteObjectsShader::CMultiSpriteObjectsShader()
{
}

CMultiSpriteObjectsShader::~CMultiSpriteObjectsShader()
{
}

D3D12_RASTERIZER_DESC CMultiSpriteObjectsShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CMultiSpriteObjectsShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	//d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	//d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}


D3D12_INPUT_LAYOUT_DESC CMultiSpriteObjectsShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}


void CMultiSpriteObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CTexturedRectMesh* pSpriteMesh = nullptr;

	m_nObjects = 13;

	obj = new CMultiSpriteObject * [m_nObjects];

	XMFLOAT3 xmf3Position = XMFLOAT3(1030.0f, 180.0f, 1410.0f);

	for (int j{}; j < m_nObjects; ++j)
	{
		pSpriteObject = new CMultiSpriteObject();

		if (0 == j || 8 == j || 9 == j)
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);//연기
		else if (1 == j)
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f);//로딩
		else if (2 == j)
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 180.0f, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f);//파티클
		else if (3 == j)//||5==j)//피
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		else if (10 == j)//팝업
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 60.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		else if (11 == j || 12 == j)//크로스헤어
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		else//화면
			pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 130.0f, 10.0f, 0.0f, 0.0f, 45.0f, 0.0f);

		pSpriteObject->SetMesh(0, pSpriteMesh);
		pSpriteObject->SetPosition(XMFLOAT3(xmf3Position.x, xmf3Position.y, xmf3Position.z));

		obj[j] = pSpriteObject;
		obj[j]->m_nMaterials = 1;
	}
}

void CMultiSpriteObjectsShader::ReleaseUploadBuffers()
{
	CShader::ReleaseUploadBuffers();
}

void CMultiSpriteObjectsShader::ReleaseObjects()
{
	CShader::ReleaseObjects();
}
void CMultiSpriteObjectsShader::AnimateObjects(float fTimeElapsed, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int j{}; j < m_nObjects; ++j) {
		bool active = false;
		for (int i = 0; i < 3; ++i) {
			if (obj[j]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[i] == true) {
				active = true;
				break;
			}
		}
		if (1 != obj[j]->texMat.z && 2 != obj[j]->texMat.z && active)
			obj[j]->Animate(fTimeElapsed, false, pd3dDevice, pd3dCommandList);
	}
}

void CMultiSpriteObjectsShader::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, vector<CMonster*> mon, short daMo, vector<CPlayer*> pl, int boss, bool game, void* pContext)
{
	for (int i{}; i < m_nObjects; ++i)
	{
		CPlayer* pPlayer = pCamera->GetPlayer();
		CPlayer* p1 = nullptr;
		CPlayer* p2 = nullptr;
		CPlayer* p3 = nullptr;

		for (int i{}; i < pl.size(); ++i)
		{
			if (0 == pl[i]->c_id % 3 && obj[0]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0])
				p1 = pl[i];
			else if (1 == pl[i]->c_id % 3 && obj[8]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[1])
				p2 = pl[i];
			else if (2 == pl[i]->c_id % 3 && obj[9]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[2])
				p3 = pl[i];
		}

		if (0 < i && 5 > i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0]
			|| 10 == i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0]

			|| 5 == i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] && 0 == pPlayer->c_id % 3
			|| 6 == i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[1] && 1 == pPlayer->c_id % 3
			|| 7 == i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[2] && 2 == pPlayer->c_id % 3

			|| 0 == i && p1
			|| 8 == i && p2
			|| 9 == i && p3
			|| 11 == i && obj[i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0])
		{
			XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
			XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
			XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
			XMFLOAT3 xmf3Position;

			XMFLOAT3 xmf3MonPos;
			XMFLOAT3 xmf3MonLook;
			XMFLOAT3 xmf3Pos = XMFLOAT3(0, 0, 0);
			CMonster* m = nullptr;

			if (0 == i)//|| 8==i || 9==i)//연기
			{
				xmf3PlayerPosition = p1->GetPosition();

				xmf3PlayerPosition.x += 25 * p1->GetRight().x;
				xmf3PlayerPosition.y += 25 * p1->GetRight().y;
				xmf3PlayerPosition.z += 25 * p1->GetRight().z;

				xmf3PlayerPosition.y += 8.0f;

				xmf3PlayerLook = p1->GetLookVector();
			}
			else if (8 == i)//|| 8==i || 9==i)//연기
			{
				xmf3PlayerPosition = p2->GetPosition();

				xmf3PlayerPosition.x += 25 * p2->GetRight().x;
				xmf3PlayerPosition.y += 25 * p2->GetRight().y;
				xmf3PlayerPosition.z += 25 * p2->GetRight().z;

				xmf3PlayerPosition.y += 8.0f;

				xmf3PlayerLook = p2->GetLookVector();
			}
			else if (9 == i)//|| 8==i || 9==i)//연기
			{
				xmf3PlayerPosition = p3->GetPosition();

				xmf3PlayerPosition.x += 25 * p3->GetRight().x;
				xmf3PlayerPosition.y += 25 * p3->GetRight().y;
				xmf3PlayerPosition.z += 25 * p3->GetRight().z;

				xmf3PlayerPosition.y += 8.0f;

				xmf3PlayerLook = p3->GetLookVector();
			}
			else if (1 == i)//로딩
			{
				xmf3PlayerPosition.y += 32.0f;
				xmf3PlayerPosition.x -= 30.0f;

				xmf3PlayerPosition.x = (xmf3PlayerPosition.x + 2 * xmf3CameraPosition.x) / 3;
				xmf3PlayerPosition.y = (xmf3PlayerPosition.y + 2 * xmf3CameraPosition.y) / 3;
				xmf3PlayerPosition.z = (xmf3PlayerPosition.z + 2 * xmf3CameraPosition.z) / 3;
			}
			else if (2 == i && !mon.empty() && -1 != boss)//파티클
			{
				try {
					xmf3MonPos = mon.at(boss)->GetPosition();
					xmf3MonLook = mon.at(boss)->GetLook();
					xmf3MonPos.y += 40.0f;
					xmf3Pos = Vector3::Add(xmf3MonPos, Vector3::ScalarProduct(xmf3MonLook, 50.0f, false));
				}
				catch (const exception& e) {

				}
			}
			else if (3 == i)//몬스터 피격
			{
				for (auto& t : mon)
				{
					if (daMo == t->c_id)
					{
						m = t;
						break;
					}
				}

				if (!m)
					break;

				xmf3MonPos = m->GetPosition();
				xmf3MonLook = m->GetLook();
				xmf3MonPos.y += 50.0f;

				xmf3MonPos.x = (xmf3MonPos.x + 1.f * xmf3PlayerPosition.x) / 2;
				xmf3MonPos.y = (xmf3MonPos.y + 1.f * xmf3PlayerPosition.y) / 2;
				xmf3MonPos.z = (xmf3MonPos.z + 1.f * xmf3PlayerPosition.z) / 2;

				xmf3Pos = Vector3::Add(xmf3MonPos, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));
			}
			else if (4 == i)//화면
			{
				if (game)
					xmf3PlayerPosition.y -= 79.0f;
				else
					xmf3PlayerPosition.y -= 85.0f;

				xmf3PlayerPosition.x = (xmf3PlayerPosition.x + 3.f * xmf3CameraPosition.x) / 4.f;
				xmf3PlayerPosition.y = (xmf3PlayerPosition.y + 3.f * xmf3CameraPosition.y) / 4.f;
				xmf3PlayerPosition.z = (xmf3PlayerPosition.z + 3.f * xmf3CameraPosition.z) / 4.f;
			}
			else if (10 == i)//팝업
			{
				xmf3PlayerPosition.y -= 60.0f;

				xmf3PlayerPosition.x = (xmf3PlayerPosition.x + 3.f * xmf3CameraPosition.x) / 4.f;
				xmf3PlayerPosition.y = (xmf3PlayerPosition.y + 3.f * xmf3CameraPosition.y) / 4.f;
				xmf3PlayerPosition.z = (xmf3PlayerPosition.z + 3.f * xmf3CameraPosition.z) / 4.f;
			}
			else//플레이어 피격
			{
				xmf3PlayerPosition.y -= 79.0f;

				xmf3PlayerPosition.x = (xmf3PlayerPosition.x + 3.f * xmf3CameraPosition.x) / 4.f;
				xmf3PlayerPosition.y = (xmf3PlayerPosition.y + 3.f * xmf3CameraPosition.y) / 4.f;
				xmf3PlayerPosition.z = (xmf3PlayerPosition.z + 3.f * xmf3CameraPosition.z) / 4.f;
			}

			xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));
			if (obj[i])
			{
				if (1 != i && 2 != i && 1 != obj[i]->texMat.z)
				{
					obj[i]->SetPosition(xmf3Position);
					obj[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
				}
				else if (1 == i)//로딩
				{
					obj[i]->SetPosition(xmf3PlayerPosition);
					obj[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
				}
				else if (2 == i)//파티클
				{
					obj[i]->SetPosition(xmf3MonPos);
					obj[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
				}
				else if (1 == obj[i]->texMat.z)//몬스터 피격 3
				{
					obj[i]->SetPosition(xmf3Pos);
					obj[i]->SetLookAt(xmf3PlayerPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
				}
				if (i == 11) {

					obj[i + pPlayer->gun_hit]->SetPosition(pPlayer->Aiming_Position);
					obj[i + pPlayer->gun_hit]->SetLookAt(pCamera->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
					//auto mesh = static_cast<CTexturedRectMesh*>(obj[i + pPlayer->gun_hit]->m_ppMeshes[0]);
					//mesh->Scale(pd3dDevice, pd3dCommandList,pPlayer->aimSize);
					CShader::Render(pd3dCommandList, pCamera);
					obj[i + pPlayer->gun_hit]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
					return;
				}
				CShader::Render(pd3dCommandList, pCamera);
				obj[i]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CComputeShader::CComputeShader()
{
}

CComputeShader::~CComputeShader()
{
}

D3D12_SHADER_BYTECODE CComputeShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

void CComputeShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob, nPipelineState);
	d3dPipelineStateDesc.NodeMask = 0;
	d3dPipelineStateDesc.CachedPSO = d3dCachedPipelineState;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();

	m_cxThreadGroups = cxThreadGroups;
	m_cyThreadGroups = cyThreadGroups;
	m_czThreadGroups = czThreadGroups;
}

void CComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	UpdateShaderVariables(pd3dCommandList, NULL);
	pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
}

void CComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState)
{
	UpdateShaderVariables(pd3dCommandList, NULL);
	pd3dCommandList->Dispatch(cxThreadGroups, cyThreadGroups, czThreadGroups);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGaussian2DBlurComputeShader::CGaussian2DBlurComputeShader()
{
}

CGaussian2DBlurComputeShader::~CGaussian2DBlurComputeShader()
{
}

D3D12_SHADER_BYTECODE CGaussian2DBlurComputeShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "CSGaussian2DBlur", "cs_5_1", ppd3dShaderBlob));
}

void CGaussian2DBlurComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* tex)
{
	if (!set[1])
	{
		m_pTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 3);
		m_pTexture->m_pnResourceTypes[0] = RESOURCE_TEXTURE2D;
		m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, 0, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL);
		m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, 1, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT , 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
		m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, 2, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT , 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	}

	pd3dCommandList->CopyResource(m_pTexture->GetResource(1), tex);
	//pd3dCommandList->CopyResource(m_pTexture->GetResource(2), tex);


	if (!set[1])
	{
		CreateComputeShaderResourceView(pd3dDevice, m_pTexture, 1, 0, 0, 1);
		//CreateComputeShaderResourceView(pd3dDevice, m_pTexture, 0, 1, 0, 1);
		CreateComputeUnorderedAccessView(pd3dDevice, m_pTexture, 0, 0, 0, 1);

		m_pTexture->SetComputeSrvRootParameter(0, 0, 0, 1);
		//m_pTexture->SetComputeSrvRootParameter(1, 2, 1, 1);
		m_pTexture->SetComputeUavRootParameter(0, 1, 0, 1);

		m_cxThreadGroups = ceil(FRAME_BUFFER_WIDTH / 32.0f);
		m_cyThreadGroups = ceil(FRAME_BUFFER_HEIGHT / 32.0f);

		set[1] = true;
	}
}



void CGaussian2DBlurComputeShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, ID3D12Resource* tex, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups, int nPipelineState)
{
	if (!set[0])
	{
		CComputeShader::CreateShader(pd3dDevice, pd3dRootSignature, cxThreadGroups, cyThreadGroups, czThreadGroups, 0);
		CreateCbvSrvUavDescriptorHeaps(pd3dDevice, 0, 2, 1);

		set[0] = true;
	}
	else
		CreateShaderVariables(pd3dDevice, pd3dCommandList, tex);
}

void CGaussian2DBlurComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 0);
	//if (m_pTexture) m_pTexture->UpdateComputeSrvShaderVariable(pd3dCommandList, 1);
	if (m_pTexture) m_pTexture->UpdateComputeUavShaderVariable(pd3dCommandList, 0);
}

void CGaussian2DBlurComputeShader::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->Release();
}

void CGaussian2DBlurComputeShader::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CGaussian2DBlurComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState)
{
	OnPrepare(pd3dCommandList);
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	UpdateShaderVariables(pd3dCommandList);

	//for (int i{}; i < 5; ++i)
	{
		pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
		ID3D12Resource* pd3dSource = m_pTexture->GetResource(0);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		ID3D12Resource* pd3dDestination = m_pTexture->GetResource(1);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

void CGaussian2DBlurComputeShader::Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource** rt, UINT index, int nPipelineState)
{
	OnPrepare(pd3dCommandList);
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	UpdateShaderVariables(pd3dCommandList);

	//for (int i{}; i < 5; ++i)
	{
		pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
		ID3D12Resource* pd3dSource = rt[index];
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
		ID3D12Resource* pd3dDestination = m_pTexture->GetResource(0);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	{
		pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
		ID3D12Resource* pd3dSource = m_pTexture->GetResource(0);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		ID3D12Resource* pd3dDestination = m_pTexture->GetResource(1);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	/*{
		pd3dCommandList->Dispatch(m_cxThreadGroups, m_cyThreadGroups, m_czThreadGroups);
		ID3D12Resource* pd3dSource = m_pTexture->GetResource(0);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		ID3D12Resource* pd3dDestination = m_pTexture->GetResource(2);
		pd3dCommandList->CopyResource(pd3dDestination, pd3dSource);
		::SynchronizeResourceTransition(pd3dCommandList, pd3dSource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}*/
}

void CGaussian2DBlurComputeShader::OnPrepare(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
}
void CGaussian2DBlurComputeShader::CreateComputeShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	for (UINT i{}; i < nDescriptors; ++i)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(nTextureIndex + i);
		if (pShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nTextureIndex + i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			pTexture->SetComputeSrvGpuDescriptorHandle(nHandleIndex + i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGraphicsShader::CGraphicsShader()
{
}

CGraphicsShader::~CGraphicsShader()
{
}

D3D12_SHADER_BYTECODE CGraphicsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CGraphicsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CGraphicsShader::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CGraphicsShader::CreateDomainShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CGraphicsShader::CreateHullShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CGraphicsShader::CreateInputLayout(int nPipelineState)
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CGraphicsShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CGraphicsShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CGraphicsShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CGraphicsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nPipelineState);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nPipelineState);
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(nPipelineState);
	d3dPipelineStateDesc.BlendState = CreateBlendState(nPipelineState);
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(nPipelineState);
	d3dPipelineStateDesc.InputLayout = CreateInputLayout(nPipelineState);
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	for (UINT i{}; i < nRenderTargets; ++i) d3dPipelineStateDesc.RTVFormats[i] = (pdxgiRtvFormats) ? pdxgiRtvFormats[i] : DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CGraphicsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext, int nPipelineState)
{
	/*OnPrepare(pd3dCommandList);
	if (m_ppd3dPipelineStates[nPipelineState]) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	UpdateShaderVariables(pd3dCommandList, pContext);*/
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTextureToFullScreenShader::CTextureToFullScreenShader(CTexture* pTexture)
{
	/*m_pTexture = pTexture;

	if (m_pTexture)
		m_pTexture->AddRef();*/
}

CTextureToFullScreenShader::~CTextureToFullScreenShader()
{
}



D3D12_DEPTH_STENCIL_DESC CTextureToFullScreenShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextureToViewport", "vs_5_1", ppd3dShaderBlob));//VSTextureToViewport
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextureToFullScreen", "ps_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CTextureToFullScreenShader2::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextureToFull", "ps_5_1", ppd3dShaderBlob));
}

void CTextureToFullScreenShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!set[1])
	{
		CreateGraphicsShaderResourceView(pd3dDevice, m_pTexture, 2, 0, 0, 1);
		//CreateGraphicsShaderResourceView(pd3dDevice, m_pTexture, 2, 1, 0, 1);

		m_pTexture->SetGraphicsSrvRootParameter(0, 16, 0, 1);
		//m_pTexture->SetGraphicsSrvRootParameter(1, 16, 1, 1);
		
		set[1] = true;
	}
}

void CTextureToFullScreenShader::ReleaseShaderVariables()
{
#ifndef _WITH_SHARED_TEXTURE
	if (m_pTexture) m_pTexture->Release();
#endif
}

void CTextureToFullScreenShader::ReleaseUploadBuffers()
{
#ifndef _WITH_SHARED_TEXTURE
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
#endif
}

void CTextureToFullScreenShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	if (m_pTexture) m_pTexture->UpdateGraphicsSrvShaderVariable(pd3dCommandList, 0);
	if (m_pTexture) m_pTexture->UpdateGraphicsSrvShaderVariable(pd3dCommandList, 1);
}

void CTextureToFullScreenShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	if (!set[0])
	{
		CGraphicsShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat, 0);
		CreateCbvSrvUavDescriptorHeaps(pd3dDevice, 0, 1, 0);
		set[0] = true;
	}
	else
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CTextureToFullScreenShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext,  int nPipelineState)
{
	OnPrepare(pd3dCommandList);

	if (m_pd3dPipelineState)
		pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	UpdateShaderVariables(pd3dCommandList, NULL);
	
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}
void CTextureToFullScreenShader2::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext, int nPipelineState)
{

	D3D12_RECT d3dScissorRect = { 220,120, 420, 320 };
	pd3dCommandList->RSSetScissorRects(1, &d3dScissorRect);


	OnPrepare(pd3dCommandList);

	if (m_pd3dPipelineState)
		pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	UpdateShaderVariables(pd3dCommandList, NULL);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CTextureToFullScreenShader::OnPrepare(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
}
void CTextureToFullScreenShader::CreateGraphicsShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nTextureIndex, UINT nHandleIndex, UINT nDescriptorHeapIndex, UINT nDescriptors)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	for (UINT i = 0; i < nDescriptors; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(nTextureIndex + i);
		if (pShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nTextureIndex + i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			pTexture->SetGraphicsSrvGpuDescriptorHandle(nHandleIndex + i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}

CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSNewTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSNewTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


CIlluminatedTexturedShader::CIlluminatedTexturedShader()
{
}

CIlluminatedTexturedShader::~CIlluminatedTexturedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedTexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CIlluminatedTexturedShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTexturedLighting", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CIlluminatedTexturedShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedLighting", "ps_5_1", &m_pd3dPixelShaderBlob));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//



CPostProcessingShader::CPostProcessingShader()
{
}

CPostProcessingShader::~CPostProcessingShader()
{
	if (m_pTexture) delete m_pTexture;

	if (m_pd3dRtvCPUDescriptorHandles) delete[] m_pd3dRtvCPUDescriptorHandles;
}

D3D12_INPUT_LAYOUT_DESC CPostProcessingShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CPostProcessingShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

ID3D12RootSignature* CPostProcessingShader::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 5;
	pd3dDescriptorRanges[0].BaseShaderRegister = 1; //Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;

	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

D3D12_SHADER_BYTECODE CPostProcessingShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSPostProcessing", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CPostProcessingShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSPostProcessing", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CPostProcessingShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
#ifdef _WITH_SCENE_ROOT_SIGNATURE
	m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	m_pd3dGraphicsRootSignature->AddRef();
#else
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
#endif

	CShader::CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat);
}

void CPostProcessingShader::CreateResourcesAndRtvsSrvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, UINT nShaderResources)
{
	m_pTexture = new CTexture(nRenderTargets, RESOURCE_TEXTURE2D, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f } };
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		d3dClearValue.Format = pdxgiFormats[i];

		m_pTexture->CreateTexture(pd3dDevice, pd3dCommandList, i, RESOURCE_TEXTURE2D, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 1, 0, pdxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue);
	}

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, nShaderResources);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CreateShaderResourceViews(pd3dDevice, m_pTexture, 0, 6);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	m_pd3dRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets];

	for (UINT i = 0; i < nRenderTargets; i++)
	{
		d3dRenderTargetViewDesc.Format = pdxgiFormats[i];
		ID3D12Resource* pd3dTextureResource = m_pTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CPostProcessingShader::OnPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle)
{
	int nResources = m_pTexture->GetTextures();
	D3D12_CPU_DESCRIPTOR_HANDLE* pd3dAllRtvCPUHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets + nResources];

	for (int i = 0; i < nRenderTargets; i++)
	{
		pd3dAllRtvCPUHandles[i] = pd3dRtvCPUHandles[i];
		pd3dCommandList->ClearRenderTargetView(pd3dRtvCPUHandles[i], Colors::White, 0, NULL);
	}

	for (int i = 0; i < nResources; i++)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, Colors::White, 0, NULL);
		pd3dAllRtvCPUHandles[nRenderTargets + i] = d3dRtvCPUDescriptorHandle;
	}
	pd3dCommandList->OMSetRenderTargets(nRenderTargets + nResources, pd3dAllRtvCPUHandles, FALSE, pd3dDsvCPUHandle);

	if (pd3dAllRtvCPUHandles) delete[] pd3dAllRtvCPUHandles;
}

void CPostProcessingShader::OnPostRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList)
{
	int nResources = m_pTexture->GetTextures();
	for (int i{}; i < nResources; ++i)
	{
		::SynchronizeResourceTransition(pd3dCommandList, GetTextureResource(i), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void CPostProcessingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
{
	CShader::Render(pd3dCommandList, pCamera, pContext);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

//CTextureSampling::CTextureSampling()
//{
//}
//
//CTextureSampling::~CTextureSampling()
//{
//	ReleaseShaderVariables();
//}
//}
//
//D3D12_SHADER_BYTECODE CTextureSampling::CreateVertexShader()
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSScreenRectSamplingTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
//}
//
//D3D12_SHADER_BYTECODE CTextureSampling::CreatePixelShader()
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSScreenRectSamplingTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
//}
//
//void CTextureSampling::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
//{
//	UINT ncbElementBytes = ((sizeof(PS_CB_DRAW_OPTIONS) + 255) & ~255); //256의 배수
//	m_pd3dcbDrawOptions = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
//	m_pd3dcbDrawOptions->Map(0, NULL, (void**)&m_pcbMappedDrawOptions);
//
//	CPostProcessingShader::CreateShaderVariables(pd3dDevice, pd3dCommandList);
//}
//
//void CTextureSampling::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
//{
//	m_pcbMappedDrawOptions->m_xmn4DrawOptions.x = *((int*)pContext);
//	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbDrawOptions->GetGPUVirtualAddress();
//	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dGpuVirtualAddress);
//
//	CPostProcessingShader::UpdateShaderVariables(pd3dCommandList, pContext);
//}
//
//void CTextureSampling::ReleaseShaderVariables()
//{
//	if (m_pd3dcbDrawOptions) m_pd3dcbDrawOptions->Release();
//}
//
//void CTextureSampling::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext)
//{
//	CPostProcessingShader::Render(pd3dCommandList, pCamera, pContext);
//}
