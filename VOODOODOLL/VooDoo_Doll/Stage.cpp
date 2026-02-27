// File: CStage.cpp
//-----------------------------------------------------------------------------

#include <DirectXMath.h>
#include "stdafx.h"
#include "Stage.h"
#include"GameFramework.h"
ID3D12DescriptorHeap* CStage::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dUavCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dUavGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dUavCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dUavGPUDescriptorNextHandle;

CStage::CStage()
{
}

CStage::~CStage()
{
}


float GetDegreeWithTwoVectors(XMFLOAT3& v1, XMFLOAT3& v2)
{
	float dot = Vector3::DotProduct(v1, v2);
	float v1Length = Vector3::Length(v1);
	float v2Length = Vector3::Length(v2);

	float angleRadian = acos(dot / (v1Length * v2Length));

	return XMConvertToDegrees(angleRadian);
}

XMFLOAT3 RotatePointBaseOnPoint(XMFLOAT3& p1, XMFLOAT3& p2, float angle)
{
	XMFLOAT3 translatedP1 = XMFLOAT3(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);

	XMFLOAT4X4 RotationMatrix = Matrix4x4::RotationYawPitchRoll(0, angle, 0);
	XMFLOAT3 rotatedP1 = Vector3::TransformCoord(translatedP1, RotationMatrix);
	XMFLOAT3 finalP1 = XMFLOAT3(rotatedP1.x + p2.x, rotatedP1.y + p2.y, rotatedP1.z + p2.z);

	return finalP1;
}


void CStage::BuildDefaultLightsAndMaterials()
{
	m_nLights = MAX_LIGHTS;

	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_fRange = 4307.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.73f, 0.73f, 0.73f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(562, 140, 2300);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(-1.0f, -1.0f, 0.0f);

	m_pLights[1].m_bEnable = false;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));


	m_pLights[2].m_bEnable = false;
	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 500.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.85f, 0.85f, 0.85f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 256.0f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(+1.0f, -1.0f, 0.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(0.5f, 0.01f, 0.0001f);
	m_pLights[2].m_fFalloff = 4.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[3].m_fRange = 1000.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.83f, 0.83f, 0.83f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 128.0f, 0.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(+1.0f, -1.0f, 0.0f);


	{

		m_pLights[4].m_bEnable = true;
		m_pLights[4].m_nType = SPOT_LIGHT;
		m_pLights[4].m_fRange = 500.0f;
		m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		m_pLights[4].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		m_pLights[4].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
		m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
		m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights[4].m_fFalloff = 8.0f;
		m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

		m_pLights[5].m_bEnable = false;
		m_pLights[5].m_nType = POINT_LIGHT;
		m_pLights[5].m_fRange = 1000.f;
		m_pLights[5].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLights[5].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 7.0f);
		m_pLights[5].m_xmf4Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
		m_pLights[5].m_xmf3Position = XMFLOAT3(561.939f, 18.61f, 620.49f);
		//m_pLights[5].m_xmf3Position = XMFLOAT3(mpObjVec[0].x, mpObjVec[0].y, mpObjVec[0].z);
		m_pLights[5].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
		m_pLights[5].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights[5].m_fFalloff = 100.0f;
		m_pLights[5].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights[5].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

		for (int i = 6; i < MAX_LIGHTS; ++i)
		{
			m_pLights[i].m_bEnable = false;
			m_pLights[i].m_nType = POINT_LIGHT;
			m_pLights[i].m_fRange = 50.0f;

			m_pLights[i].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			m_pLights[i].m_xmf4Diffuse = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);
			m_pLights[i].m_xmf4Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
			/*m_pLights[i].m_xmf3Position = XMFLOAT3(580.f, -192.9157f,1052.653f);*/
			m_pLights[i].m_xmf3Position = XMFLOAT3(mpObjVec[i - 6].x, mpObjVec[i - 6].y, mpObjVec[i - 6].z);
			m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
			m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
			m_pLights[i].m_fFalloff = 100.0f;
			m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
			m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
		}
	}
}


void CStage::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 500, 0);//4 1

	m_pd3dComputeRootSignature = CreateComputeRootSignature(pd3dDevice);

	m_nComputeShaders = 2;
	pComputeShader = new CGaussian2DBlurComputeShader * [m_nComputeShaders];

	pComputeShader[0] = new CGaussian2DBlurComputeShader();
	pComputeShader[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature, nullptr);

	pComputeShader[1] = new CGaussian2DBlurComputeShader();
	pComputeShader[1]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature, nullptr);

	/*pComputeShader->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
	pComputeShader->m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	pComputeShader->m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	pComputeShader->m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	pComputeShader->m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	pComputeShader->m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	pComputeShader->m_d3dUavGPUDescriptorNextHandle=m_d3dUavGPUDescriptorStartHandle;*/


	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nGraphicsShaders = 1;
	pGraphicsShader = new CTextureToFullScreenShader * [m_nGraphicsShaders];

	pGraphicsShader[0] = new CTextureToFullScreenShader(pComputeShader[0]->m_pTexture);
	pGraphicsShader[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, compShaderFormats, DXGI_FORMAT_D32_FLOAT);

	pGraphicsShader2 = new CTextureToFullScreenShader2(pComputeShader[1]->m_pTexture);
	pGraphicsShader2->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, compShaderFormats, DXGI_FORMAT_D32_FLOAT);


	/*pGraphicsShader->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
	pGraphicsShader->m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	pGraphicsShader->m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	pGraphicsShader->m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	pGraphicsShader->m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	pGraphicsShader->m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	pGraphicsShader->m_d3dUavGPUDescriptorNextHandle = m_d3dUavGPUDescriptorStartHandle;*/


	DXGI_FORMAT pdxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);


	pBoxShader = new CBoxShader();

	m_nShaders = 1;
	m_ppShaders = new CShader * [m_nShaders];
	pObjectShader = new CObjectsShader();
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	mpObjVec = pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, "Models/Scene.bin", pBoxShader);
	//Find_LightPosition();
	m_ppShaders[0] = pObjectShader;


	m_pLights = new LIGHT[MAX_LIGHTS];
	BuildDefaultLightsAndMaterials();//조명


	int iMaterialCheck = 0;

	CTexture* ppTextures[39];


	ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wall_wood_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wood_ground_whole_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/popup1.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[3]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/wall_03_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[4]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/popup2.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[5] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[5]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_4_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[6]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_3_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[7] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[7]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/popup3.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[8] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[8]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wooden_Chair_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[9] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[9]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/paper_type_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[10] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[10]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/cart_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[11] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[11]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Poster_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[12] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[12]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/dressing_table_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[13] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[13]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Pillows_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[14] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[14]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Matress_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[15] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[15]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Cloth_hanging_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[16] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[16]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Cloth_hanging_01_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[17] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[17]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/woodalt_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[18] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[18]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Stairs_secondary_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[19] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[19]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/stairs_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[20] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[20]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_2_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[21] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[21]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Ceiling_wood_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[22] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[22]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Booknotext.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[23] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[23]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Large_Shelf_Large_Shelf_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[24] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[24]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Blackboard.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[25] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[25]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Info_Board_Info_Board_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[26] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[26]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Paper_Paper_BaseColor.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[27] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[27]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Coin01_Roughness.dds", RESOURCE_TEXTURE2D, 0);









	ppTextures[28] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);

	ppTextures[28]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/hp.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[29] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[29]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/slider.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[30] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[30]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/UI_ENGFONT.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[31] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[31]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Puppet_add_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);


	ppTextures[32] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[32]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Explosion_6x6.dds", RESOURCE_TEXTURE2D, 0);


	ppTextures[33] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[33]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/UI_LOADING.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[34] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[34]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/particle.dds", RESOURCE_TEXTURE2D, 0);//boss

	ppTextures[35] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[35]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/blood.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[36] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[36]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/plBlood.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[37] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[37]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/crosshair.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[38] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[38]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/hit_marker.dds", RESOURCE_TEXTURE2D, 0);


	// 버튼 퍼즐 오브젝트
	pButtonTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	pButtonTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wood_8_Albedo.dds", RESOURCE_TEXTURE2D, 0);

	pButtonTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	pButtonTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/popup4.dds", RESOURCE_TEXTURE2D, 0);


	// 캔들 오브젝트
	pCandleTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	pCandleTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Candle1_Off.dds", RESOURCE_TEXTURE2D, 0);

	pCandleTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	pCandleTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Candle2_Off.dds", RESOURCE_TEXTURE2D, 0);

	pCandleTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	pCandleTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Candle3_Off.dds", RESOURCE_TEXTURE2D, 0);





	m_ppShaders[0]->gameScreen[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/gameStart.dds", RESOURCE_TEXTURE2D, 0);

	m_ppShaders[0]->gameScreen[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/gameClear.dds", RESOURCE_TEXTURE2D, 0);

	m_ppShaders[0]->gameScreen[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/over.dds", RESOURCE_TEXTURE2D, 0);

	m_ppShaders[0]->gameScreen[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[3]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/login.dds", RESOURCE_TEXTURE2D, 0);

	m_ppShaders[0]->gameScreen[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[4]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/lobby1.dds", RESOURCE_TEXTURE2D, 0);


	m_ppShaders[0]->gameScreen[5] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	m_ppShaders[0]->gameScreen[5]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/lobby2.dds", RESOURCE_TEXTURE2D, 0);


	for (int a{}; a < 39; ++a)
		CreateShaderResourceViews(pd3dDevice, ppTextures[a], 0, 3);

	for (int a{}; a < 3; ++a)
		CreateShaderResourceViews(pd3dDevice, pCandleTextures[a], 0, 3);


	for (int a{}; a < 2; ++a)
		CreateShaderResourceViews(pd3dDevice, pButtonTextures[a], 0, 3);


	for (int u{}; u < 2; ++u)
	{
		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

		pMaterial->SetTexture(ppTextures[28 + u]);
		hpUi[u]->m_ppMaterials[0] = pMaterial;
	}
	for (int u{}; u < 10; ++u)
	{
		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

		pMaterial->SetTexture(ppTextures[30]);
		userId[u]->m_ppMaterials[0] = pMaterial;
	}
	for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		for (int k{}; k < m_ppShaders[0]->m_ppObjects[i]->m_nMaterials; k++)
		{
			CMaterial* pMaterial = new CMaterial(1);
			pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

			if (0 == strcmp("Dense_Floor_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[1]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}


			if (0 == strcmp("Bedroom_wall_d_02_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bedroom_wall_b_01_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bedroom_wall_b_06_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bedroom_wall_c_04_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bedroom_wall_b_05_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				if (0 == iMaterialCheck)
				{
					pMaterial->SetTexture(ppTextures[0]);
					m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
				}

				if (1 == iMaterialCheck)
				{
					pMaterial->SetTexture(ppTextures[3]);
					m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
				}
			}
			if (0 == strcmp("WoodBox10", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[5]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("WoodBox3", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("WoodBox4", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("WoodBox9", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[6]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Chair_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[8]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Door_01_Frame_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[9]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cart_static_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[10]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Poster_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_04_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[11]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Dressing_table_drawer_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Dressing_table_mirror_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[12]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Bed_blanked_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bed_pillows_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[13]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Matress_mat_BaseMap", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[14]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cloth_04_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Cloth_05_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Cloth_06_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Cloth_07_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[15]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strncmp("Puppet", m_ppShaders[0]->m_ppObjects[i]->m_pstrName, 6))
			{
				pMaterial->SetTexture(ppTextures[31]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cloth_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Cloth_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[16]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Beam_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Beam_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[17]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Stair_side_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[18]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Stair_step_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[19]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("WoodBox6", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[20]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Ceiling_concrete_base_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[21]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strcmp("Book_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) || 0 == strcmp("Book_01_alt_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Book_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) || 0 == strcmp("Book_02_alt_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strncmp("Book_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName, 12) || 0 == strcmp("Book_03_alt_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||//Book_03_mesh_(1)
				0 == strcmp("Book_04_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) || 0 == strcmp("Book_04_alt_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[22]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strcmp("Large_Shelf", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[23]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Blackboard", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[24]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Info_Board", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[25]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Paper", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[26]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("2ndRoomCoin", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[27]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Candle1", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(pCandleTextures[3]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Candle2", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(pCandleTextures[4]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Candle3", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(pCandleTextures[5]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("ButtonBottom", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Button", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(pButtonTextures[0]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			++iMaterialCheck;
		}
		iMaterialCheck = 0;
	}



	pMultiSpriteObjectShader = new CMultiSpriteObjectsShader();
	pMultiSpriteObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);
	pMultiSpriteObjectShader->BuildObjects(pd3dDevice, pd3dCommandList);

	for (int i{}; i < 4; ++i)//연기 로딩 파티클 피
	{
		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
		pMaterial->SetTexture(ppTextures[32 + i]);

		pMultiSpriteObjectShader->obj[i]->m_ppMaterials = new CMaterial * [1];
		pMultiSpriteObjectShader->obj[i]->m_ppMaterials[0] = pMaterial;

		if (0 == i)//연기
		{
			pMultiSpriteObjectShader->obj[i]->texMat.z = 6;
			pMultiSpriteObjectShader->obj[i]->m_fSpeed = 1.1f / (pMultiSpriteObjectShader->obj[i]->texMat.z * pMultiSpriteObjectShader->obj[i]->texMat.z);

			pMultiSpriteObjectShader->obj[8]->m_ppMaterials = new CMaterial * [1];
			pMultiSpriteObjectShader->obj[8]->m_ppMaterials[0] = pMaterial;
			pMultiSpriteObjectShader->obj[8]->texMat.z = 6;
			pMultiSpriteObjectShader->obj[8]->m_fSpeed = 1.1f / (pMultiSpriteObjectShader->obj[8]->texMat.z * pMultiSpriteObjectShader->obj[8]->texMat.z);

			pMultiSpriteObjectShader->obj[9]->m_ppMaterials = new CMaterial * [1];
			pMultiSpriteObjectShader->obj[9]->m_ppMaterials[0] = pMaterial;
			pMultiSpriteObjectShader->obj[9]->texMat.z = 6;
			pMultiSpriteObjectShader->obj[9]->m_fSpeed = 1.1f / (pMultiSpriteObjectShader->obj[9]->texMat.z * pMultiSpriteObjectShader->obj[9]->texMat.z);
		}
		else if (1 == i)//로딩
		{
			pMultiSpriteObjectShader->obj[i]->texMat.z = 4;
			pMultiSpriteObjectShader->obj[i]->m_fSpeed = 3.0f / (pMultiSpriteObjectShader->obj[i]->texMat.z * pMultiSpriteObjectShader->obj[i]->texMat.z * 1.5f);
		}
		else if (2 == i)//파티클
		{
			pMultiSpriteObjectShader->obj[i]->texMat.z = 4;
			pMultiSpriteObjectShader->obj[i]->m_fSpeed = 1.1f / (pMultiSpriteObjectShader->obj[i]->texMat.z * pMultiSpriteObjectShader->obj[i]->texMat.z);
		}
		else //피
			pMultiSpriteObjectShader->obj[i]->texMat.z = 1;
	}

	for (int h{}; h < 6; ++h)
	{
		CreateShaderResourceViews(pd3dDevice, m_ppShaders[0]->gameScreen[h], 0, 3);

		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

		pMaterial->SetTexture(m_ppShaders[0]->gameScreen[h]);
		m_ppShaders[0]->gameMat[h] = pMaterial;
	}

	pMultiSpriteObjectShader->obj[4]->m_ppMaterials = new CMaterial * [1];
	pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = m_ppShaders[0]->gameMat[3];
	pMultiSpriteObjectShader->obj[4]->texMat.z = 2;
	pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;

	for (int i{}; i < 3; ++i)
	{
		pMultiSpriteObjectShader->obj[5 + i]->m_ppMaterials = new CMaterial * [1];
		CMaterial* pMaterial = new CMaterial(1);
		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
		pMaterial->SetTexture(ppTextures[36]);
		pMultiSpriteObjectShader->obj[5 + i]->m_ppMaterials[0] = pMaterial;
		pMultiSpriteObjectShader->obj[5 + i]->texMat.z = 3;
	}

	for (int i{}; i < 4; ++i)
	{
		m_ppShaders[0]->popUpMat[i] = new CMaterial(1);
		m_ppShaders[0]->popUpMat[i]->SetMaterialType(MATERIAL_ALBEDO_MAP);

		if (2 > i)
			m_ppShaders[0]->popUpMat[i]->SetTexture(ppTextures[2 * i + 2]);
		else if (2 == i)
			m_ppShaders[0]->popUpMat[i]->SetTexture(ppTextures[7]);
		else if (3 == i)
			m_ppShaders[0]->popUpMat[i]->SetTexture(pButtonTextures[1]);
	}
	pMultiSpriteObjectShader->obj[10]->m_ppMaterials = new CMaterial * [1];
	pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0] = m_ppShaders[0]->popUpMat[1];
	pMultiSpriteObjectShader->obj[10]->texMat.z = 2;


	for (int i{}; i < 2; ++i)
	{
		pMultiSpriteObjectShader->obj[11 + i]->m_ppMaterials = new CMaterial * [1];
		CMaterial* p_Material = new CMaterial(1);
		p_Material->SetMaterialType(MATERIAL_ALBEDO_MAP);
		p_Material->SetTexture(ppTextures[37 + i]);
		pMultiSpriteObjectShader->obj[11 + i]->m_ppMaterials[0] = p_Material;
		pMultiSpriteObjectShader->obj[11 + i]->texMat.z = 1;
		pMultiSpriteObjectShader->obj[11 + i]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
	}


	for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		//cout << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << "	|	" << m_ppShaders[0]->m_ppObjects[i]->m_iObjID << endl;

		m_ppShaders[0]->m_ppObjects[i]->Boundingbox_Transform();

		if (18 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID)
		{
			pPuzzles[0].push_back(m_ppShaders[0]->m_ppObjects[i]);
			//cout << "p1stRoomPuzzle		: " << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
		}


		if (11 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID || 388 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID ||
			389 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID || 390 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID ||
			391 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID || 392 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID)
		{
			pPuzzles[1].push_back(m_ppShaders[0]->m_ppObjects[i]);
			//cout << "p2ndRoomPuzzle		: " << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
		}


		if (409 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID || 411 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID ||
			413 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID || 415 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID ||
			417 == m_ppShaders[0]->m_ppObjects[i]->m_iObjID)
		{
			pPuzzles[5].push_back(m_ppShaders[0]->m_ppObjects[i]);
			m_ppShaders[0]->m_ppObjects[i]->obBox =
				BoundingOrientedBox(m_ppShaders[0]->m_ppObjects[i]->GetPosition(), XMFLOAT3(3.f, 3.f, 2.5f), XMFLOAT4(0, 0, 0, 1));
		}


		//cout << "Name: " <<i<< m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
		/*cout << "Center: ";
		Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox.Center);
		cout << "Extents: ";
		Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox.Extents);*/
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CStage::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();

	if (m_pd3dCbvSrvDescriptorHeap)
		m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_pd3dComputeRootSignature)
		m_pd3dComputeRootSignature->Release();

	if (m_ppShaders)
	{
		for (int i{}; i < m_nShaders; ++i)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}
	if (pMultiSpriteObjectShader)
	{
		pMultiSpriteObjectShader->ReleaseShaderVariables();
		pMultiSpriteObjectShader->ReleaseObjects();
		pMultiSpriteObjectShader->Release();
	}
	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->ReleaseShaderVariables();
		m_pDepthRenderShader->ReleaseObjects();
		m_pDepthRenderShader->Release();
	}
	if (m_pShadowShader)
	{
		if (!exitGame)
		{
			m_pShadowShader->ReleaseShaderVariables();
			m_pShadowShader->ReleaseObjects();
			m_pShadowShader->Release();
		}
	}
	if (m_pShadowMapToViewport)
	{
		m_pShadowMapToViewport->ReleaseShaderVariables();
		m_pShadowMapToViewport->ReleaseObjects();
		m_pShadowMapToViewport->Release();
	}
	if (hpUi)
	{
		for (int i{}; i < 2; ++i)
		{
			hpUi[i]->ReleaseShaderVariables();
			hpUi[i]->Release();
		}
		delete[] hpUi;
	}
	if (pBoxShader)
	{
		pBoxShader->ReleaseShaderVariables();
		pBoxShader->Release();

		if (!exitGame)
		{
			pBoxShader->ReleaseObjects();
		}
	}


	/*if (pGraphicsShader)
	{
		for (int i{}; i < m_nGraphicsShaders; ++i)
		{
			pGraphicsShader[i]->ReleaseShaderVariables();
			pGraphicsShader[i]->ReleaseObjects();
			pGraphicsShader[i]->Release();
		}
		delete[] pGraphicsShader;
	}
	if (pGraphicsShader2)
	{
		pGraphicsShader2->ReleaseShaderVariables();
		pGraphicsShader2->ReleaseObjects();
		pGraphicsShader2->Release();

		delete pGraphicsShader2;
	}

	if (pComputeShader)
	{
		for (int i{}; i < m_nComputeShaders; ++i)
		{
			pComputeShader[i]->ReleaseShaderVariables();
			pComputeShader[i]->Release();
		}
		delete[] pComputeShader;
	}*/

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature* CStage::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtPrevFrame
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//9->6
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 1; //t1: Texture2D
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = MAX_DEPTH_TEXTURES;
	pd3dDescriptorRanges[7].BaseShaderRegister = 2; //Depth Buffer
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 0; //t0: Texture2D
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;




	D3D12_ROOT_PARAMETER pd3dRootParameters[17];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	//멤버를 잘못 건드림
	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//17->9
	pd3dRootParameters[9].Descriptor.ShaderRegister = 5; //DrawOptions

	//파라미터 타입이 CBV인데, Constants.RegisterSpace를 쓰고 있음.
	//pd3dRootParameters[9].Constants.RegisterSpace = 0;
	pd3dRootParameters[9].Descriptor.RegisterSpace = 0;

	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[7]; //Depth Buffer
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[13].Descriptor.ShaderRegister = 6; //ToLight
	pd3dRootParameters[13].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[14].Constants.Num32BitValues = 17;
	pd3dRootParameters[14].Constants.ShaderRegister = 3; //Material
	pd3dRootParameters[14].Constants.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[8]; //Texture2D
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6]; //Texture2D
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0.0f;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].MipLODBias = 0.0f;
	pd3dSamplerDescs[3].MaxAnisotropy = 1;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT h = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CStage::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255);

	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CStage::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CStage::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CStage::ReleaseUploadBuffers()
{
	for (int i{}; i < m_nShaders; ++i)
		m_ppShaders[i]->ReleaseUploadBuffers();

	pMultiSpriteObjectShader->ReleaseUploadBuffers();

	if (m_pShadowShader)
		m_pShadowShader->ReleaseUploadBuffers();

	if (pBoxShader)
		pBoxShader->ReleaseUploadBuffers();

	if (hpUi)
	{
		hpUi[0]->ReleaseUploadBuffers();
		hpUi[1]->ReleaseUploadBuffers();
	}

	for (int i{}; i < m_nGraphicsShaders; ++i)
		pGraphicsShader[i]->ReleaseUploadBuffers();
	pGraphicsShader2->ReleaseUploadBuffers();

	for (int i{}; i < m_nComputeShaders; ++i)
		pComputeShader[i]->ReleaseUploadBuffers();
}

void CStage::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
	m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	m_d3dUavGPUDescriptorNextHandle = m_d3dUavGPUDescriptorStartHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CStage::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j{}; j < nConstantBufferViews; ++j)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CStage::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i{}; i < nTextures; ++i)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j{}; j < nRootParameters; ++j)
		pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

bool CStage::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CStage::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool CStage::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CStage::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (int i{}; i < m_nShaders; ++i)
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[4].m_xmf3Position = m_pPlayer->obBox.Center;
		m_pLights[4].m_xmf3Direction = m_pPlayer->GetLookVector();

		m_pLights[1].m_bEnable = false;
	}


	static float fAngle = 0.0f;
	fAngle += 1.50f;
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Rotate(0.0f, -fAngle, 0.0f);
	XMFLOAT3 xmf3Position = Vector3::TransformCoord(XMFLOAT3(50.0f, 0.0f, 0.0f), xmf4x4Rotate);

}

void CStage::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, LIGHT* light, ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap, vector<CMonster*> Monsters, vector<CPlayer*> Players)
{
	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
		m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList, light, Monsters, Players);
	}
}
void CStage::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights
	}
}


void CStage::Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Device* pd3dDevice, bool login, ID3D12Resource** rt, UINT index, CCamera* pCamera)//ID3D12Resource*
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_pd3dCbvSrvDescriptorHeap) 
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);


	if (m_pDepthRenderShader)
		m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);//����

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights

	if (login)
	{
		for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
		{
			if (strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Dense_Floor_mesh")
				&& strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Stair_step_01_mesh")
				&& strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "ForDoorcollider"))
			{

				if (false == m_ppShaders[0]->m_ppObjects[i]->m_bGetItem)
				{
					m_ppShaders[0]->m_ppObjects[i]->m_ppMaterials[0]->m_pStandardShader->Render(pd3dCommandList, pCamera);
					m_ppShaders[0]->m_ppObjects[i]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
				}
			}
		}

		for (int j{}; j < m_ppShaders[0]->m_nDoor; ++j)
		{
			m_ppShaders[0]->door[j]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
		}
	}



	if (login && blur)
	{
		pComputeShader[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature, rt[index]);

		if (!pGraphicsShader[0]->set[2])
		{
			pGraphicsShader[0]->m_pTexture = pComputeShader[0]->m_pTexture;
			if (pGraphicsShader[0]->m_pTexture) pGraphicsShader[0]->m_pTexture->AddRef();

			pGraphicsShader[0]->set[2] = true;
		}
		pGraphicsShader[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, compShaderFormats, DXGI_FORMAT_D32_FLOAT);



		pComputeShader[1]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dComputeRootSignature, rt[UINT(abs(int(index - 1)))]);

		if (!pGraphicsShader2->set[2])
		{
			pGraphicsShader2->m_pTexture = pComputeShader[1]->m_pTexture;
			if (pGraphicsShader2->m_pTexture) pGraphicsShader2->m_pTexture->AddRef();

			pGraphicsShader2->set[2] = true;
		}
		pGraphicsShader2->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, compShaderFormats, DXGI_FORMAT_D32_FLOAT);


		if (m_pd3dComputeRootSignature)
			pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);

		pComputeShader[0]->Dispatch(pd3dCommandList, 0);

		//pComputeShader[1]->Dispatch(pd3dCommandList,rt, abs(int(index - 1)), 0);
		pComputeShader[1]->Dispatch(pd3dCommandList, 0);
		++softBlur;

		if (1 == softBlur % 4 || !pComputeShader[0]->blur)
		{

			::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader[0]->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
			pd3dCommandList->CopyResource(pGraphicsShader[0]->m_pTexture->GetResource(2), pGraphicsShader[0]->m_pTexture->GetResource(1));
			::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader[0]->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

			if (move || 1 == softBlur)
			{
				::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader2->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
				pd3dCommandList->CopyResource(pGraphicsShader2->m_pTexture->GetResource(2), pGraphicsShader2->m_pTexture->GetResource(1));
				::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader2->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

				move = false;
			}
			else
			{
				::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader[0]->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
				pd3dCommandList->CopyResource(pGraphicsShader2->m_pTexture->GetResource(2), pGraphicsShader[0]->m_pTexture->GetResource(1));
				::SynchronizeResourceTransition(pd3dCommandList, pGraphicsShader[0]->m_pTexture->GetResource(1), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			}
		}


		if (m_pd3dGraphicsRootSignature)
			pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);


		pGraphicsShader[0]->Render(pd3dCommandList, pCamera, NULL);

		if (pComputeShader[0]->blur)
			pGraphicsShader2->Render(pd3dCommandList, pCamera, NULL);
	}
}



void CStage::CheckObjectByObjectCollisions(float fTimeElapsed, CPlayer*& pl)
{
	XMFLOAT3 Vel = pl->GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);

	pl->onFloor = false;
	for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		BoundingOrientedBox oBox = m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox;

		if (0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Bedroom_wall_b_06_mesh") ||
			0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "stone") ||
			0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "2ndRoomCoin") ||
			0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Key_mesh"))
		{
			continue;
		}

		if (true == m_ppShaders[0]->m_ppObjects[i]->m_bGetItem)
		{
			continue;
		}

		if (pl->obBox.Intersects(oBox))
		{

			if (pl->obBox.Center.y > oBox.Center.y + oBox.Extents.y && Vel.y <= 0) {
				XMFLOAT3 Pos = pl->GetPosition();
				Pos.y = oBox.Center.y + oBox.Extents.y;
				pl->SetPosition(Pos);
				pl->SetVelocity(XMFLOAT3(Vel.x, 0.0f, Vel.z));
				pl->onFloor = true;
				continue;
			}

			//cout << "Name - " << i << " " << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
			//cout << "Center - ";
			//Vector3::Print(oBox.Center);
			//cout << "Extents - ";
			//Vector3::Print(oBox.Extents);
			//cout << "Look - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetLook());
			//cout << "Right - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetRight());
			//cout << "Up - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetUp());

			float angle = GetDegreeWithTwoVectors(m_ppShaders[0]->m_ppObjects[i]->GetLook(), XMFLOAT3(0, 0, 1));
			XMFLOAT3 ObjLook = { 0,0,0 };


			if ((int)angle % 90 == 0)
			{
				XMVECTOR xmVector = XMLoadFloat3(&oBox.Extents);
				XMVECTOR xmQuaternion = XMLoadFloat4(&oBox.Orientation);

				// Rotate the vector using the quaternion
				XMVECTOR rotatedVector = XMVector3Rotate(xmVector, xmQuaternion);

				// Convert the rotated vector back to an XMFLOAT3
				XMFLOAT3 realExtents;
				XMStoreFloat3(&realExtents, rotatedVector);

				realExtents.x = sqrtf(realExtents.x * realExtents.x);
				realExtents.y = sqrtf(realExtents.y * realExtents.y);
				realExtents.z = sqrtf(realExtents.z * realExtents.z);

				if (oBox.Center.x - realExtents.x < pl->obBox.Center.x && oBox.Center.x + realExtents.x > pl->obBox.Center.x) {
					if (oBox.Center.z < pl->obBox.Center.z) ObjLook = { 0,0,1 };
					else ObjLook = { 0, 0, -1 };
				}
				else if (oBox.Center.x < pl->obBox.Center.x) ObjLook = { 1,0,0 };
				else ObjLook = { -1, 0, 0 };

			}
			else
			{

				XMFLOAT3 RotatedPos = RotatePointBaseOnPoint(pl->obBox.Center, oBox.Center, angle);

				if (oBox.Center.x - oBox.Extents.x < RotatedPos.x && oBox.Center.x + oBox.Extents.x > RotatedPos.x) {
					if (oBox.Center.z < RotatedPos.z) ObjLook = m_ppShaders[0]->m_ppObjects[i]->GetLook();
					else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->m_ppObjects[i]->GetLook(), -1);
				}
				else if (oBox.Center.x < RotatedPos.x) ObjLook = m_ppShaders[0]->m_ppObjects[i]->GetRight();
				else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->m_ppObjects[i]->GetRight(), -1);
			}
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}
}

void CStage::CheckMoveObjectsCollisions(float fTimeElapsed, CPlayer*& pl, vector<CMonster*>& monsters, vector<CPlayer*>& players) {

	XMFLOAT3 Vel = pl->GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);

	for (const auto& monster : monsters) {
		if (pl->obBox.Intersects(monster->m_xmOOBB)) {
			XMFLOAT3 ObjLook = { 0,0,0 };


			if (monster->m_xmOOBB.Center.x - monster->m_xmOOBB.Extents.x < pl->obBox.Center.x && monster->m_xmOOBB.Center.x + monster->m_xmOOBB.Extents.x > pl->obBox.Center.x) {
				if (monster->m_xmOOBB.Center.z < pl->obBox.Center.z) ObjLook = { 0,0,1 };
				else ObjLook = { 0, 0, -1 };
			}
			else if (monster->m_xmOOBB.Center.x < pl->obBox.Center.x) ObjLook = { 1,0,0 };
			else ObjLook = { -1, 0, 0 };
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}

	for (auto& player : players) {
		if (player->c_id == pl->c_id || player->alive == false) continue;
		if (pl->obBox.Intersects(player->obBox)) {
			XMFLOAT3 ObjLook = { 0,0,0 };


			if (player->obBox.Center.x - player->obBox.Extents.x < pl->obBox.Center.x && player->obBox.Center.x + player->obBox.Extents.x > pl->obBox.Center.x) {
				if (player->obBox.Center.z < pl->obBox.Center.z) ObjLook = { 0,0,1 };
				else ObjLook = { 0, 0, -1 };
			}
			else if (player->obBox.Center.x < pl->obBox.Center.x) ObjLook = { 1,0,0 };
			else ObjLook = { -1, 0, 0 };
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}
}
void CStage::CheckCameraCollisions(float fTimeElapsed, CPlayer*& pl, CCamera*& cm)
{

	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();

	XMFLOAT3 xmf3Right = pl->GetRightVector();
	XMFLOAT3 xmf3Up = pl->GetUpVector();
	XMFLOAT3 xmf3Look = pl->GetLookVector();
	xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
	xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
	xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

	XMFLOAT3 LookAtPos = Vector3::Add(pl->obBox.Center, Vector3::ScalarProduct(cm->GetLookVector(), 40, false));

	if (cm->GetMode() == THIRD_PERSON_CAMERA) {
		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(cm->GetOffset(), xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(pl->obBox.Center, xmf3Offset);
		XMFLOAT3 ray_castPos = pl->obBox.Center;
		XMFLOAT3 dir = Vector3::Normalize(Vector3::Subtract(xmf3Position, ray_castPos));

		bool collide = false;
		while (Vector3::Length(Vector3::Subtract(xmf3Position, ray_castPos)) > 5.f)
		{
			for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
			{
				if (0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Bedroom_wall_b_06_mesh")) continue;

				BoundingOrientedBox oBox = m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox;
				if (oBox.Contains(XMLoadFloat3(&ray_castPos)))
				{
					collide = true;
					break;
				}
			}

			for (int i{}; i < 7; ++i)
			{
				if (m_ppShaders[0]->door[i]->obBox.Contains(XMLoadFloat3(&xmf3Position)))
				{
					collide = true;
					break;
				}
			}

			if (collide) {
				xmf3Position = ray_castPos;
				break;
			}
			ray_castPos = Vector3::Add(ray_castPos, dir);
		}
		//cm->Update(xmf3Position, pl->obBox.Center, fTimeElapsed);
		cm->Update(xmf3Position, LookAtPos, fTimeElapsed);
	}
	//cm->SetLookAt(pl->obBox.Center);
	cm->SetLookAt(LookAtPos);

	cm->RegenerateViewMatrix();

	{
		XMFLOAT3 xmf3Position = Vector3::Add(XMFLOAT3(0, 20, 0), pl->obBox.Center);
		pl->Aiming_Position = Vector3::Add(XMFLOAT3(0, 80, 0), Vector3::Add(cm->GetPosition(), Vector3::ScalarProduct(cm->GetLookVector(), 300, false)));
		XMFLOAT3 dir = Vector3::Normalize(Vector3::Subtract(pl->Aiming_Position, xmf3Position));
		//pl->aimSize = 0.7f;
		while (Vector3::Length(Vector3::Subtract(pl->Aiming_Position, xmf3Position)) > 10.f)
		{
			XMVECTOR _Pos = XMLoadFloat3(&xmf3Position);
			for (const auto& monster : Monsters) {
				if (monster->m_xmOOBB.Contains(_Pos))
				{
					xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(dir, -10, false));
					pl->Aiming_Position = xmf3Position;
					return;
				}
			}

			for (int i{}; i < m_ppShaders[0]->m_nObjects; ++i)
			{
				if (0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Bedroom_wall_b_06_mesh")) continue;
				BoundingOrientedBox oBox = m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox;
				if (oBox.Contains(_Pos))
				{
					xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(dir, -10, false));
					pl->Aiming_Position = xmf3Position;
					return;
				}
			}

			for (int i{}; i < 7; ++i)
			{
				if (m_ppShaders[0]->door[i]->obBox.Contains(_Pos))
				{
					xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(dir, -10, false));
					pl->Aiming_Position = xmf3Position;
					return;
				}
			}

			//pl->aimSize += 0.003f;
			xmf3Position = Vector3::Add(xmf3Position, dir);
		}
	}

}

XMFLOAT3 CStage::GetReflectVec(XMFLOAT3 ObjLook, XMFLOAT3 MovVec)
{
	float Dot = Vector3::DotProduct(MovVec, ObjLook);
	XMFLOAT3 Nor = Vector3::ScalarProduct(ObjLook, Dot, false);
	XMFLOAT3 SlidingVec = Vector3::Subtract(MovVec, Nor);
	return SlidingVec;
}

// 409, 411, 413, 415, 417

void CStage::Pushing_Button(CPlayer*& pl)
{
	int iAnswer[5] = { 417, 409, 415, 411, 413 }; // 5. 1. 4. 2. 3
	int iResult = 0; // 틀릴 경우 -1

	for (int i{}; i < 5; ++i)
	{
		BoundingOrientedBox oBox = pPuzzles[5][i]->obBox;

		if (false == pPuzzles[5][i]->m_bGetItem)
		{
			if (pl->obBox.Intersects(oBox))
			{
				pPuzzles[5][i]->m_bGetItem = true;
				m_iHit[m_iHitNum] = pPuzzles[5][i]->m_iObjID;
				cout << m_iHitNum << "	|" << m_iHit[m_iHitNum] << endl;
				m_iHitNum++;
			}
		}

		if (5 == m_iHitNum)
		{
			for (int k{}; k < 5; ++k)
			{
				if (iAnswer[k] == m_iHit[k])
				{
					iResult++;
				}
				else
				{
					iResult = -1;
				}
			}
		}

		if (5 == iResult)
		{
			cout << "Right!" << endl;
			b5thDoorPass = true;
			break;
		}

		else if (-1 == iResult)
		{
			cout << "Lose" << endl;


			for (int i{}; i < 5; ++i)
			{
				pPuzzles[5][i]->m_bGetItem = false;
			}
			m_iHitNum = 0;
		}
	}

}





void CStage::CheckDoorCollisions(float fTimeElapsed, CPlayer*& pl)
{
	XMFLOAT3 Vel = pl->GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);

	pl->onFloor = false;
	for (int i{}; i < 6; ++i)
	{
		BoundingOrientedBox oBox = m_ppShaders[0]->door[i]->obBox;
		if (pl->obBox.Intersects(oBox))
		{
			if (pl->obBox.Center.y > oBox.Center.y + oBox.Extents.y && Vel.y <= 0) {
				XMFLOAT3 Pos = pl->GetPosition();
				Pos.y = oBox.Center.y + oBox.Extents.y;
				pl->SetPosition(Pos);
				pl->SetVelocity(XMFLOAT3(Vel.x, 0.0f, Vel.z));
				pl->onFloor = true;
				continue;
			}

			float angle = GetDegreeWithTwoVectors(m_ppShaders[0]->door[i]->GetLook(), XMFLOAT3(0, 0, 1));
			XMFLOAT3 ObjLook = { 0,0,0 };


			if ((int)angle % 90 == 0)
			{
				XMVECTOR xmVector = XMLoadFloat3(&oBox.Extents);
				XMVECTOR xmQuaternion = XMLoadFloat4(&oBox.Orientation);

				// Rotate the vector using the quaternion
				XMVECTOR rotatedVector = XMVector3Rotate(xmVector, xmQuaternion);

				// Convert the rotated vector back to an XMFLOAT3
				XMFLOAT3 realExtents;
				XMStoreFloat3(&realExtents, rotatedVector);

				realExtents.x = sqrtf(realExtents.x * realExtents.x);
				realExtents.y = sqrtf(realExtents.y * realExtents.y);
				realExtents.z = sqrtf(realExtents.z * realExtents.z);

				if (oBox.Center.x - realExtents.x < pl->obBox.Center.x && oBox.Center.x + realExtents.x > pl->obBox.Center.x) {
					if (oBox.Center.z < pl->obBox.Center.z) ObjLook = { 0,0,1 };
					else ObjLook = { 0, 0, -1 };
				}
				else if (oBox.Center.x < pl->obBox.Center.x) ObjLook = { 1,0,0 };
				else ObjLook = { -1, 0, 0 };

			}
			else
			{

				XMFLOAT3 RotatedPos = RotatePointBaseOnPoint(pl->obBox.Center, oBox.Center, angle);

				if (oBox.Center.x - oBox.Extents.x < RotatedPos.x && oBox.Center.x + oBox.Extents.x > RotatedPos.x) {
					if (oBox.Center.z < RotatedPos.z) ObjLook = m_ppShaders[0]->door[i]->GetLook();
					else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->door[i]->GetLook(), -1);
				}
				else if (oBox.Center.x < RotatedPos.x) ObjLook = m_ppShaders[0]->door[i]->GetRight();
				else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->door[i]->GetRight(), -1);
			}
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}
}


void CStage::Lighthing(CPlayer*& pl)
{

	for (int iNum{ 6 }; iNum < MAX_LIGHTS; ++iNum)
	{
		float fDisatnce = CalculateDistance(pl->obBox.Center, m_pLights[iNum].m_xmf3Position);

		if (300.f > fDisatnce)
		{
			m_pLights[iNum].m_bEnable = true;
		}

		else
		{
			m_pLights[iNum].m_bEnable = false;
		}
	}

}

float CStage::CalculateDistance(XMFLOAT3& pPlayer, XMFLOAT3& pLight)
{

	DirectX::XMVECTOR vec1 = DirectX::XMLoadFloat3(&pPlayer);
	DirectX::XMVECTOR vec2 = DirectX::XMLoadFloat3(&pLight);
	DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(vec1, vec2);

	float fDisatnce;
	XMStoreFloat(&fDisatnce, XMVector3Length(diff));
	return fDisatnce;
}



ID3D12RootSignature* CStage::CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs)
{
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = nRootParameters;
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = nStaticSamplerDescs;
	d3dRootSignatureDesc.pStaticSamplers = pd3dStaticSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3D12RootSignature* pd3dRootSignature = NULL;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dRootSignature);
}

ID3D12RootSignature* CStage::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: Texture2D
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //u0: RWTexture2D
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 11; //t11: Texture2D
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture2D
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //RWTexture2D
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //Texture2D
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	ID3D12RootSignature* pd3dComputeRootSignature = CreateRootSignature(pd3dDevice, d3dRootSignatureFlags, _countof(pd3dRootParameters), pd3dRootParameters, 0, NULL);

	return(pd3dComputeRootSignature);
}








