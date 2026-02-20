//-----------------------------------------------------------------------------
// File: Stage.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include "Monster.h"


#define MAX_LIGHTS						32//5+26
#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

struct SHADOWMATERIAL
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_Scene_MATERIALS];
};

class CStage
{
public:
	CStage();
	~CStage();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	virtual ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs);

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, LIGHT*, ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap, vector<CMonster*> Monsters, vector<CPlayer*> Players);
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Device* pd3dDevice, bool, ID3D12Resource**,UINT, CCamera* pCamera = NULL);


	void ReleaseUploadBuffers();
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }

	void CheckObjectByObjectCollisions(float fTimeElapsed, CPlayer*& Pl);
	void CheckMoveObjectsCollisions(float TimeElapsed, CPlayer*& pl, vector<CMonster*>& monsters, vector<CPlayer*>& players);
	void CheckCameraCollisions(float fTimeElapsed, CPlayer*& pl, CCamera*& cm);
	XMFLOAT3 GetReflectVec(XMFLOAT3 ObjLook, XMFLOAT3 MovVec);

	XMFLOAT3 Calculate_Direction(BoundingBox& pBouningBoxA, BoundingBox& pBouningBoxB);

	//XMFLOAT3 Calculate_Direction(BoundingBox& pBouningBoxA, BoundingBox& pBouningBoxB);


	void Lighthing(CPlayer*& pl);

	float CalculateDistance(XMFLOAT3& pPlayer, XMFLOAT3& pLight);

	void CheckDoorCollisions(float fTimeElapsed, CPlayer*& pl);
	void Pushing_Button(CPlayer*& pl);




	CPlayer* m_pPlayer = NULL;
	float							m_fLightRotationAngle = 0.0f;
	static ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	bool blur = false;
	int softBlur = 0;
	bool move = false;
	
protected:




	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dUavCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorNextHandle;



public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	float							m_fElapsedTime = 0.0f;

	XMFLOAT3					m_xmf3RotatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	LIGHT* m_pLights = NULL;

	int								m_nLights = 0;
	MATERIALS* m_pMaterials = NULL;
	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;

	CShader** m_ppShaders = NULL;
	int								m_nShaders = 0;
	CObjectsShader* pObjectShader = nullptr;

	ID3D12PipelineState* m_pd3dPipelineState = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

	XMFLOAT4					m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;


public:
	vector<XMFLOAT3> mpObjVec;
	float							mpTime = 0.f;
	XMFLOAT4X4				m_xmf4x4World = Matrix4x4::Identity();

	CBoxShader* pBoxShader = NULL;
	CShadowMapShader* m_pShadowShader = NULL;
	CDepthRenderShader* m_pDepthRenderShader = NULL;
	CTextureToViewportShader* m_pShadowMapToViewport = NULL;
	CGameObject** hpUi = nullptr;
	CGameObject** userId = nullptr;
	CGameObject** userPw = nullptr;
	CMultiSpriteObjectsShader* pMultiSpriteObjectShader = nullptr;

	//��� ���̴�
	ID3D12RootSignature* m_pd3dComputeRootSignature = NULL;

	CGaussian2DBlurComputeShader** pComputeShader = nullptr;
	CTextureToFullScreenShader** pGraphicsShader = nullptr;
	CTextureToFullScreenShader2* pGraphicsShader2 = nullptr;

	
	int								m_nComputeShaders = 0;

	int								m_nGraphicsShaders = 0;
	
	DXGI_FORMAT compShaderFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };

	// �� ������ ������ ������ �޼����� ��� true�� ��ȯ
	bool										b1stDoorPass = false;
	bool										b2ndDoorPass = false;
	bool										b3rdDoorPass = false;
	bool										b4thDoorPass = false;
	bool										b5thDoorPass = false;// �� ���

	vector<int>							DeleteObject;
	int											iGetItem = 0;
	int											iGetCoin = 0;

	CTexture* pCandleTextures[3] = {};
	CTexture* pButtonTextures[2] = {};
	int											m_iHitNum = 0;
	int											m_iHit[5] = {};
	bool exitGame = false;

public:
	/*vector<CGameObject*>		p1stRoomPuzzle;
	vector<CGameObject*>		p2ndRoomPuzzle;*/
	array<vector<CGameObject*>, 7> pPuzzles;
	vector<CMonster*> Monsters;
};

