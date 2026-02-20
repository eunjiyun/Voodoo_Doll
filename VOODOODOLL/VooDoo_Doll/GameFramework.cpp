//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"


ID3D12GraphicsCommandList* CGameFramework::m_pd3dCommandList = NULL;

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i{}; i < m_nSwapChainBuffers; ++i)
		m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i{}; i < m_nSwapChainBuffers; ++i) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pStage = NULL;
	m_pLogin = NULL;
	m_pPlayer = NULL;
	m_eCurrentScene = SCENE_STAGE;

	m_ePrevScene = SCENE_LOGIN;
	m_eCurrentScene = SCENE_STAGE;

	_tcscpy_s(m_pszFrameRate, _T("VoodooDoll ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	//full screen
	if (onFullScreen)
		ChangeSwapChainState();
	else
		CreateRenderTargetViews();

	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//??u??? ????? ????????? ????? ????u??(???????)?? ??? ???? ???????. 



	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	//full screen
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i{}; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); ++i)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i{}; i < m_nSwapChainBuffers; ++i) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	::gnCbvSrvUavDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}





void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i{}; i < m_nSwapChainBuffers; ++i)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateSwapChainRenderTargetViews()
{

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	m_d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i{}; i < m_nSwapChainBuffers; ++i)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], &d3dRenderTargetViewDesc, m_d3dRtvCPUDescriptorHandle);
		m_pd3dSwapChainBackBufferRTVCPUHandles[i] = m_d3dRtvCPUDescriptorHandle;
		m_d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);


	for (int i{}; i < m_nSwapChainBuffers; ++i)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,
		m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);


	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	
	CreateSwapChainRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	RECT rcWindow;
	GetWindowRect(Get_HWND(), &rcWindow);

	int windowX = rcWindow.left;
	int windowY = rcWindow.top;

	XMFLOAT2 sign[4];

	if (m_pStage) m_pStage->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

		move = true;
		
		if (false == onFullScreen)
		{
			sign[0].x = 253; sign[1].x = 365;
			sign[0].y = 381; sign[1].y = 422;

			sign[2].x = 421; sign[3].x = 531;
			sign[2].y = 379; sign[3].y = 425;
		}
		else
		{
			sign[0].x = 245; sign[1].x = 356;
			sign[0].y = 346; sign[1].y = 392;

			sign[2].x = 413; sign[3].x = 522;
			sign[2].y = 345; sign[3].y = 392;
		}

		if (1 != gameButton)
		{
			if (sign[0].x <= m_ptOldCursorPos.x - windowX && sign[1].x >= m_ptOldCursorPos.x - windowX
				&& sign[0].y <= m_ptOldCursorPos.y - windowY && sign[1].y >= m_ptOldCursorPos.y - windowY)
				signIn = 0;
			else if (sign[2].x <= m_ptOldCursorPos.x - windowX && sign[3].x >= m_ptOldCursorPos.x - windowX
				&& sign[2].y <= m_ptOldCursorPos.y - windowY && sign[3].y >= m_ptOldCursorPos.y - windowY)
			{
				signIn = 1;
				//loginSign[1] = true;//yej
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();

		move = false;
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pStage) m_pStage->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			m_pStage->exitGame = true;
			if (onFullScreen)
				ChangeSwapChainState();

			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			if (!idSet && !userId.empty())
				idSet = true;

			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::Scene_Change(SCENEID _eSceneid)
{
	switch (_eSceneid)
	{
		/*case SCENE_OPEN:
			m_pScene = new CLogo;
			break;*/

	case SCENE_LOGIN:
		m_pLogin = new CLogin;
		break;

	case SCENE_STAGE:
		m_pStage = new CStage;
		break;

		//case SCENE_END:
		//	m_pScene = new CMyEdit;
		//	break;
	}
}

void CGameFramework::Change_Scene(SCENEID _eSceneid)
{
	switch (_eSceneid)
	{
		/*case SCENE_OPEN:
			m_pScene = new CLogo;
			break;*/

	case SCENE_LOGIN:
		m_pLogin = new CLogin;
		break;

	case SCENE_STAGE:
		m_pStage = new CStage;
		break;

		//case SCENE_END:
		//	m_pScene = new CMyEdit;
		//	break;
	}
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i{}; i < m_nSwapChainBuffers; ++i)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();

	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER


void CGameFramework::BuildObjects()
{
	//커맨드 리스트 열림
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	Scene_Change(m_eCurrentScene);

	m_pStage->hpUi = new CGameObject * [2];
	m_pStage->hpUi[0] = new CGameObject(1);
	m_pStage->hpUi[1] = new CGameObject(1);

	m_pStage->userId = new CGameObject * [10];
	for (int i{}; i < 10; ++i)
		m_pStage->userId[i] = new CGameObject(1);
	m_pStage->userPw = new CGameObject * [10];
	for (int i{}; i < 10; ++i)
		m_pStage->userPw[i] = new CGameObject(1);


	if (m_pLogin) m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	if (m_pStage)
	{
		m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	}

	m_pStage->hpUi[0]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);
	m_pStage->hpUi[0]->SetScale(0.05f, 0.3f, 0.1f);

	m_pStage->hpUi[1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);
	m_pStage->hpUi[1]->SetScale(0.05f, 0.3f, 0.7f);

	for (int i{}; i < 10; ++i)
	{
		m_pStage->userId[i]->SetScale(0.033f, 0.3f, 0.05f);
		m_pStage->userId[i]->SetPosition(50, -141, 455 + 12 * i);
	}
	for (int i{}; i < 10; ++i)
	{
		m_pStage->userPw[i]->SetScale(0.033f, 0.3f, 0.05f);
		m_pStage->userPw[i]->SetPosition(50, -158, 455 + 12 * i);
	}


	// Initialize SoundPlayer
	sound[0].Initialize();
	sound[0].LoadWave(inGame, 0);


	sound[1].Initialize();
	sound[1].LoadWave(opening, 0);


	sound[2].Initialize();
	sound[2].LoadWave(closing, 0);


	sound[3].Initialize();
	sound[3].LoadWave(win, 1);

	hitSound.Initialize();

	m_pLights = m_pStage->m_pLights;

	DXGI_FORMAT RtvFormats[5] = { DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT };

	for (int i = 1; i <= 2; ++i) {
		for (int j{}; j < 3; ++j) {
			pMonsterModel[i].push(
				CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), binFileNames[i], NULL, i + 1));
		}
	}

	for (int j{}; j < 4; ++j)
		pMonsterModel[0].push(
			CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), binFileNames[0], NULL, 1));




	for (int i{}; i < 3; ++i) {
		MagiciansHat.push(CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Warlock_cap.bin", NULL, 7));
	}

	pMonsterModel[3].push(
		CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), binFileNames[3], NULL, 1));


	m_pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);


	m_pPlayer->AnimationControllers[0]->SetCallbackKeys(1, 1);
	//m_pPlayer->AnimationControllers[0]->SetCallbackKeys(2, 1);
	m_pPlayer->AnimationControllers[0]->SetCallbackKeys(3, 1);
	m_pPlayer->AnimationControllers[0]->SetCallbackKeys(4, 1);
	m_pPlayer->AnimationControllers[0]->SetCallbackKeys(5, 1);

	m_pPlayer->AnimationControllers[1]->SetCallbackKeys(1, 1);
	m_pPlayer->AnimationControllers[1]->SetCallbackKeys(2, 1);
	m_pPlayer->AnimationControllers[1]->SetCallbackKeys(3, 1);
	m_pPlayer->AnimationControllers[1]->SetCallbackKeys(4, 1);
	m_pPlayer->AnimationControllers[1]->SetCallbackKeys(5, 1);

	m_pPlayer->AnimationControllers[2]->SetCallbackKeys(1, 1);
	//m_pPlayer->AnimationControllers[2]->SetCallbackKeys(2, 1);
	m_pPlayer->AnimationControllers[2]->SetCallbackKeys(3, 1);
	m_pPlayer->AnimationControllers[2]->SetCallbackKeys(4, 1);
	m_pPlayer->AnimationControllers[2]->SetCallbackKeys(5, 1);

#ifdef _WITH_SOUND_RESOURCE
	m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else

	m_pPlayer->AnimationControllers[0]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	//m_pPlayer->AnimationControllers[0]->SetCallbackKey(2, 0, 0.2f, _T("Sound/swordAttack.wav"));
	m_pPlayer->AnimationControllers[0]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	m_pPlayer->AnimationControllers[0]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	m_pPlayer->AnimationControllers[0]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));


	m_pPlayer->AnimationControllers[1]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	m_pPlayer->AnimationControllers[1]->SetCallbackKey(2, 0, 0.2f, _T("Sound/gunAttack.wav"));
	m_pPlayer->AnimationControllers[1]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	m_pPlayer->AnimationControllers[1]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	m_pPlayer->AnimationControllers[1]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));


	m_pPlayer->AnimationControllers[2]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	//m_pPlayer->AnimationControllers[2]->SetCallbackKey(2, 0, 0.2f, _T("Sound/attack.wav"));
	m_pPlayer->AnimationControllers[2]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	m_pPlayer->AnimationControllers[2]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	m_pPlayer->AnimationControllers[2]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));

#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();

	m_pPlayer->AnimationControllers[0]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	//AnimationControllers[0]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[0]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[0]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[0]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);

	m_pPlayer->AnimationControllers[1]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[1]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[1]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[1]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[1]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);

	m_pPlayer->AnimationControllers[2]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	//m_pPlayer->AnimationControllers[2]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[2]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[2]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	m_pPlayer->AnimationControllers[2]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);





	m_pStage->m_pPlayer = m_pPlayer;
	m_pCamera = m_pPlayer->GetCamera();


	Players.push_back(m_pPlayer);

	for (int i{}; i < 2; ++i) {
		CTerrainPlayer* pAirplanePlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);
		Players.push_back(pAirplanePlayer);
	}
	
	m_pStage->m_pDepthRenderShader = new CDepthRenderShader(m_pStage->pBoxShader, m_pLights);
	m_pStage->m_pDepthRenderShader->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 5, RtvFormats, DXGI_FORMAT_D32_FLOAT);
	m_pStage->m_pDepthRenderShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, NULL);


	m_pStage->m_pShadowShader = new CShadowMapShader(m_pStage->pBoxShader);
	m_pStage->m_pShadowShader->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 5, NULL, DXGI_FORMAT_D32_FLOAT);//pipelinestate null
	m_pStage->m_pShadowShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pStage->m_pDepthRenderShader->GetDepthTexture());

	m_pStage->m_pShadowMapToViewport = new CTextureToViewportShader();
	m_pStage->m_pShadowMapToViewport->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);


	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pStage->userId)
		for (int i{}; i < userId.size(); ++i)
			m_pStage->userId[i]->ReleaseUploadBuffers();

	if (m_pStage->userPw)
		for (int i{}; i < userPw.size(); ++i)
			m_pStage->userPw[i]->ReleaseUploadBuffers();


	if (m_pStage) m_pStage->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();



	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (!m_pStage->exitGame)
	{
		if (m_pPlayer) m_pPlayer->Release();


		for (auto& player : Players)
			delete player;
	}

	for (auto& monster : m_pStage->Monsters)
		delete monster;

	if (sound)
	{
		for (int i{}; i < 4; ++i)
			sound[i].~SoundPlayer();
		//delete sound;
	}
	playerSound.~SoundPlayer();
	hitSound.~SoundPlayer();
	doorSound.~SoundPlayer();

	if (m_pStage->userId)
	{
		for (int i{}; i < userId.size(); ++i)
		{
			m_pStage->userId[i]->ReleaseShaderVariables();
			m_pStage->userId[i]->Release();
		}
		delete[] m_pStage->userId;
	}
	if (m_pStage->userPw)
	{
		for (int i{}; i < userPw.size(); ++i)
		{
			m_pStage->userPw[i]->ReleaseShaderVariables();
			m_pStage->userPw[i]->Release();
		}
		delete[] m_pStage->userPw;
	}


	if (m_pStage) m_pStage->ReleaseObjects();
	if (m_pStage) delete m_pStage;

	if (m_pLogin) m_pLogin->ReleaseObjects();
	if (m_pLogin) delete m_pLogin;
}

void CGameFramework::CreateOtherPlayer(int p_id, XMFLOAT3 Pos)
{
	for (auto& player : Players)
		if (player->c_id < 0) {
			player->c_id = p_id;
			player->SetPosition(Pos);
			player->recv_time = high_resolution_clock::now();
			player->alive = true;
			break;
		}
}

void CGameFramework::SummonMonster(int npc_id, int type, XMFLOAT3 Pos)
{
	// ?? ??????? ????? ???? ?????? ?????
	if (pMonsterModel[type].empty()) {

		cout << "????????\n";

		return;
	}

	CMonster* Mon = nullptr;
	CLoadedModelInfo* Hat = nullptr;
	CLoadedModelInfo* Model = pMonsterModel[type].front();


	// Client's monster speed = Server's monster speed * 3 / 10	-> ???? 30ms, ?????? 100ms ???? ?????????? ????
	pMonsterModel[type].pop();
	switch (type)
	{

	case 0: {
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4); //�տ� Į	

		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);

		Mon->m_xmOOBB = BoundingBox(Pos, XMFLOAT3{ 15,30,15 });
		Mon->speed = 12.f;
		Mon->HP = 250;
		Mon->SetScale(1.0f, 1.0f, 1.0f);

		break;

	}
	case 1: {
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4);//���ٱ� �ٸ�

		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);

		Mon->m_xmOOBB = BoundingBox(Pos, XMFLOAT3{ 7,30,7 });
		Mon->speed = 12.f;
		Mon->HP = 150;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;

	}
	case 2: {
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4); // ������

		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);


		Mon->m_xmOOBB = BoundingBox(Pos, XMFLOAT3{ 7,30,7 });
		Hat = MagiciansHat.front();
		MagiciansHat.pop();
		Mon->Hat_Model = Hat;
		Mon->m_ppHat = new CBulletObject(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Mon->Hat_Model, 1, 2);
		Mon->m_ppHat->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_ppHat->SetScale(0.8f, 0.8f, 0.8f);
		Mon->speed = 12.f;
		Mon->HP = 50;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	}
	case 3: {
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 7);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);


		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, true);//���̵�
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, false);//�ٱ�
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);//����
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);//���
		Mon->m_pSkinnedAnimationController->SetTrackEnable(4, false);//����
		Mon->m_pSkinnedAnimationController->SetTrackEnable(5, false);//�ǰ�
		Mon->m_pSkinnedAnimationController->SetTrackEnable(6, false);//�ȱ�

		Mon->m_xmOOBB = BoundingBox(Pos, XMFLOAT3{ 30,60,30 });
		Mon->speed = 12.f;
		Mon->HP = 2000;

		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	}
	default:
		break;
	}
	if (Mon != nullptr)
	{
		Mon->c_id = npc_id;
		Mon->npc_type = type;
		Mon->SetPosition(Pos);
		m_pStage->Monsters.push_back(Mon);

		Mon->Sound.Initialize();
		Mon->Sound.LoadWave(monster[0], 1);
		Mon->Sound.Play();

		//if (0 == Mon->c_id % 10)
		//{
		//	monsterSound.Initialize();
		//	monsterSound.LoadWave(monster[0]);
		//	monsterSound.Play();
		//}
	}
}


void CGameFramework::AnimateObjects(float fTimeElapsed)
{
	if (m_pStage) m_pStage->AnimateObjects(fTimeElapsed);

	for (auto& player : Players)
	{
		if (player->c_id > -1) {
			player->boundingAnimate(fTimeElapsed);
			player->Animate(fTimeElapsed, true);
		}
	}
	for (auto& monster : m_pStage->Monsters)
	{
		if (monster->c_id > -1)
		{
			monster->Animate(fTimeElapsed, false);
		}
	}

	m_pStage->pMultiSpriteObjectShader->AnimateObjects(fTimeElapsed, m_pd3dDevice, m_pd3dCommandList);

	for (int i{}; i < 7; ++i)
	{
		if (openDoor[i])
		{
			if (3 != i)
			{
				if (curStage == i - 1)
					curStage = i;
			}
			else
			{
				if (curStage == i - 2)
					curStage = i;
			}

			if (2 != i)
			{
				if (m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fPosition
					== m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationSets->
					m_pAnimationSets[m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_nAnimationSet]->m_fLength
					&& i == curStage)
				{
					if (false == checkDoorSound)
					{
						doorSound.Stop();
						doorSound.Terminate();
						checkDoorSound = true;
					}
				}
				else
				{
					if (false == checkDoor[i])
					{
						doorSound.Initialize();
						doorSound.LoadWave(door, 1);
						doorSound.Play();
						checkDoor[i] = true;
						checkDoorSound = false;
					}
				}

				m_pStage->m_ppShaders[0]->door[i]->Animate(fTimeElapsed, false);
			}
		}
	}


	if (m_pPlayer->gun_hit)
		bloodTime += fTimeElapsed;
	if (bloodTime > 0.1f)
	{
		m_pStage->pMultiSpriteObjectShader->obj[3]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;//??
		damagedMon = -1;
		m_pPlayer->gun_hit = 0;
		bloodTime = 0.f;
	}



	if (m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0]) {
		popUpTime += fTimeElapsed;
		if (popUpTime > 3.f)
		{
			m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;//???
			popUpTime = 0.f;
		}
	}
	if (m_pStage->pMultiSpriteObjectShader->obj[5 + m_pPlayer->c_id % 3]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[m_pPlayer->c_id % 3]
		|| m_pStage->pComputeShader[0]->blur)
	{
		m_pStage->blur = true;
	}
	else if (!m_pStage->pMultiSpriteObjectShader->obj[5 + m_pPlayer->c_id % 3]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[m_pPlayer->c_id % 3]
		&& !m_pStage->pComputeShader[0]->blur)
		m_pStage->blur = false;
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(30.0f);//30??????

	float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	// Play sound
	sound[1].Play();//??????

	for (auto& player : Players) {
		if (player->c_id > -1) {
			player->Update(fTimeElapsed);

			m_pStage->CheckMoveObjectsCollisions(fTimeElapsed, player, m_pStage->Monsters, Players);
			// ������ �浹ó��
			m_pStage->CheckDoorCollisions(fTimeElapsed, player);
			m_pStage->CheckObjectByObjectCollisions(fTimeElapsed, player);
			m_pStage->Lighthing(player);
			m_pStage->Pushing_Button(player);
			if (player->onAct == false)
				player->processAnimation();
			player->Deceleration(fTimeElapsed);

		}
	}

	if (loginSign[1] && !gameEnd && !lobby[0])
	{
		m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[0];
		m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
	}

	RECT rcWindow;
	GetWindowRect(Get_HWND(), &rcWindow);

	int windowX = rcWindow.left;
	int windowY = rcWindow.top;

	if (loginSign[1] && !lobby[0])
	{
		if (false == onFullScreen)
		{
			if (530 <= m_ptOldCursorPos.x - windowX && 594 >= m_ptOldCursorPos.x - windowX
				&& 290 <= m_ptOldCursorPos.y - windowY && 325 >= m_ptOldCursorPos.y - windowY)//play
			{
				if (!lobby[0])
				{
					m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[4];
					m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
					m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
				}

				lobby[0] = true;

				for (int i{}; i < 10; ++i)
					m_pStage->userId[i]->SetPosition(50, -50, 559 + 12 * i);


			}
			else if (532 <= m_ptOldCursorPos.x - windowX && 595 >= m_ptOldCursorPos.x - windowX
				&& 348 <= m_ptOldCursorPos.y - windowY && 385 >= m_ptOldCursorPos.y - windowY)//exit
			{
				gameButton = 2;
				m_pStage->exitGame = true;
			}
			else if (504 <= m_ptOldCursorPos.x - windowX && 625 >= m_ptOldCursorPos.x - windowX
				&& 410 <= m_ptOldCursorPos.y - windowY && 446 >= m_ptOldCursorPos.y - windowY)//settings
				gameButton = 3;
		}
		else
		{
			if (523 <= m_ptOldCursorPos.x - windowX && 587 >= m_ptOldCursorPos.x - windowX
				&& 257 <= m_ptOldCursorPos.y - windowY && 293 >= m_ptOldCursorPos.y - windowY)
			{
				if (!lobby[0])
				{
					m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[4];
					m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
					m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
				}

				lobby[0] = true;

				for (int i{}; i < 10; ++i)
					m_pStage->userId[i]->SetPosition(50, -52, 554 + 12 * i);

			}

			else if (525 <= m_ptOldCursorPos.x - windowX && 588 >= m_ptOldCursorPos.x - windowX
				&& 318 <= m_ptOldCursorPos.y - windowY && 353 >= m_ptOldCursorPos.y - windowY)
			{
				gameButton = 2;
				m_pStage->exitGame = true;
				ChangeSwapChainState();
			}

			else if (495 <= m_ptOldCursorPos.x - windowX && 618 >= m_ptOldCursorPos.x - windowX
				&& 377 <= m_ptOldCursorPos.y - windowY && 415 >= m_ptOldCursorPos.y - windowY)
				gameButton = 3;
		}
	}

	m_pStage->CheckCameraCollisions(fTimeElapsed, m_pPlayer, m_pCamera);

	switch (gameButton)
	{
	case 1://play
		sound[1].Stop();//??????
		sound[0].Play();//?????

		ShowCursor(false);//????
		break;
	case 2://exit
		PostQuitMessage(0);
		break;
	case 3://settings
		break;
	}

	for (auto& monster : m_pStage->Monsters) {
		monster->Update(fTimeElapsed);
	}
	AnimateObjects(fTimeElapsed);
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pStage->OnPrepareRender(m_pd3dCommandList);

	//D3D12 ERROR: ID3D12CommandList::DrawIndexedInstanced: The render target format in slot 0 does not match that specified by the current pipeline state. (pipeline state = R8G8B8A8_UNORM, render target format = R32_FLOAT, RTV ID3D12Resource* = 0x000001B914EA31A0:'Unnamed ID3D12Resource Object') [ EXECUTION ERROR #613: RENDER_TARGET_FORMAT_MISMATCH_PIPELINE_STATE]
	m_pStage->OnPreRender(m_pd3dCommandList, m_pLights, m_pStage->m_pd3dCbvSrvDescriptorHeap, m_pStage->Monsters, Players);

	//리소스 상태 전환 (Present => Rendertarget)
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	//클리어
	float pfClearColor[4] = { 0.31f, 0.74f, 0.88f, 0.0f };// ??? ????
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);


	//RT 설정
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);



	if (4 == m_pPlayer->m_pSkinnedAnimationController->Cur_Animation_Track)//??????? 
	{
		if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_fPosition ==
			m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_nAnimationSet]->m_fLength)
		{
			m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[2];
			m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
			lobby[2] = false;
			gameEnd = true;

			sound[0].Stop();//�ΰ���
			sound[2].Play();//Ŭ��¡

		}
	}

	if (1 == gameButton && true == m_pPlayer->alive)
	{

		if (-64 > m_pPlayer->GetPosition().y)
		{
			m_pStage->hpUi[0]->SetPosition(50, -225, 178);
			m_pStage->hpUi[1]->SetPosition(50, -225, 346);
		}
		else
		{
			m_pStage->hpUi[0]->SetPosition(50, -55, 178);
			m_pStage->hpUi[1]->SetPosition(50, -55, 346);
		}


		if (m_pStage->m_pd3dCbvSrvDescriptorHeap)
			m_pd3dCommandList->SetDescriptorHeaps(1, &m_pStage->m_pd3dCbvSrvDescriptorHeap);

		if (m_pStage->m_ppShaders[0]->m_pd3dPipelineState)
			m_pd3dCommandList->SetPipelineState(m_pStage->m_ppShaders[0]->m_pd3dPipelineState);

		m_pStage->hpUi[0]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		m_pStage->hpUi[1]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
	}

	if (!loginSign[1])
	{
		if (!userId.empty() && !idSet && delUser)
			userId.pop_back();

		if (!userPw.empty() && idSet && delUser)
			userPw.pop_back();

		if (!userId.empty())
		{
			for (int u{}; u < 26; ++u)
			{
				if ('A' == userId[userId.size() - 1] - u)
					m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[113 + u]->m_ppMeshes[0]);
				else if ('a' == userId[userId.size() - 1] - u)
					m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[139 + u]->m_ppMeshes[0]);
				else if ('0' == userId[userId.size() - 1] - u)
				{
					if (9 > u)
						m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[167 + u]->m_ppMeshes[0]);
				}
				else if ('9' == userId[userId.size() - 1])
				{
					if (0 == u)
						m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[49]->m_ppMeshes[0]);
				}
				else if ('*' == userId[userId.size() - 1])
				{
					if (0 == u)
						m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[165]->m_ppMeshes[0]);
				}
				else if ('_' == userId[userId.size() - 1])
				{
					if (0 == u)
						m_pStage->userId[userId.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[166]->m_ppMeshes[0]);
				}

			}

			if (!gameEnd)
				for (int i{}; i < userId.size(); ++i)
					m_pStage->userId[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);



		}
		if (!userPw.empty())
		{
			m_pStage->userPw[userPw.size() - 1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[165]->m_ppMeshes[0]);

			for (int i{}; i < userPw.size(); ++i)
				m_pStage->userPw[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}


	if (lobby[0] && !lobby[1] && !lobby[2] && !gameEnd)
	{
		for (int i{}; i < userId.size(); ++i)
			m_pStage->userId[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
	}
	else if (lobby[0] && lobby[1] && false == lobby[2] && !gameEnd)
	{
		for (int i{}; i < userId.size(); ++i)
			m_pStage->userId[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
	}
	else if (lobby[0] && lobby[1] && lobby[2])
		gameButton = 1;

	m_pCamera->SetViewportsAndScissorRects(m_pd3dCommandList);
	m_pCamera->UpdateShaderVariables(m_pd3dCommandList);

	if (m_pStage->m_pShadowShader && lobby[2])
	{
		if (m_pStage->m_pShadowShader->m_pd3dPipelineState)
			m_pd3dCommandList->SetPipelineState(m_pStage->m_pShadowShader->m_pd3dPipelineState);

		m_pStage->m_pShadowShader->Render(m_pd3dCommandList, m_pCamera, m_pStage->Monsters, Players, m_pLights, true);
	}

	m_pStage->move = move;
	if (m_pPlayer->onAct)
		m_pStage->move = true;

	if (m_pStage)
		m_pStage->Render(m_pd3dCommandList, m_pd3dDevice, lobby[2], m_ppd3dSwapChainBackBuffers,m_nSwapChainBufferIndex, m_pCamera);

	WaitForGpuComplete();

	if (m_pStage->m_pd3dCbvSrvDescriptorHeap)
		m_pd3dCommandList->SetDescriptorHeaps(1, &m_pStage->m_pd3dCbvSrvDescriptorHeap);

	if (m_pStage->m_pShadowShader && lobby[2])
		m_pStage->m_pShadowShader->Render(m_pd3dCommandList, m_pCamera, m_pStage->Monsters, Players, m_pLights, false);
	
	int m = -1;
	if (!m_pStage->Monsters.empty())

	{
		if (-1 != damagedMon)
			m_pStage->pMultiSpriteObjectShader->obj[3]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;

		if (5 == m_pStage->Monsters[0]->c_id / 10)//particle
		{
			for (m=0; m < m_pStage->Monsters.size(); ++m)
			{
				if (59 == m_pStage->Monsters[m]->c_id)
				{
					if (2 == m_pStage->Monsters[m]->m_pSkinnedAnimationController->Cur_Animation_Track)
					{
						if (m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_fPosition >
							m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_nAnimationSet]->m_fLength * 0.15f
							&& m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_fPosition <
							m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_nAnimationSet]->m_fLength * 0.65f)
						{
							m_pStage->pMultiSpriteObjectShader->obj[2]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
						}
						else if (m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_fPosition >=
							m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pStage->Monsters[m]->m_pSkinnedAnimationController->m_pAnimationTracks[2].m_nAnimationSet]->m_fLength * 0.65f)

						{
							m_pStage->pMultiSpriteObjectShader->obj[2]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
							m_pStage->pMultiSpriteObjectShader->obj[2]->m_ppMaterials[0]->m_ppTextures[0]->m_nCol = m_pStage->pMultiSpriteObjectShader->obj[2]->m_ppMaterials[0]->m_ppTextures[0]->m_nRow = 0;
						}
					}


					break;
				}
			}
		}
	}



	m_pStage->pMultiSpriteObjectShader->Render(m_pd3dDevice, m_pd3dCommandList, m_pCamera, m_pStage->Monsters, damagedMon, Players, m, gameEnd);


	if (m_pStage->m_pShadowMapToViewport && 1 == gameButton && true == m_pPlayer->alive)
	{
		m_pStage->m_pShadowMapToViewport->Render(m_pd3dCommandList, m_pCamera, m_pPlayer->HP / 25.f, XMFLOAT2(38, 27));

		for (int i{}; i < m_pStage->Monsters.size(); ++i)
		{
			auto& Mon = m_pStage->Monsters.at(i);
			if (Mon->c_id == 59) continue;
			try {
				if (Mon->damaged) {

					XMFLOAT2 pos = m_pStage->m_pShadowMapToViewport->WorldToScreen(Vector3::Add(Mon->GetPosition(), XMFLOAT3(0, 50, 0)), m_pCamera, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

					pos.x -= Mon->HP / 2.f;
					m_pStage->m_pShadowMapToViewport->Render(m_pd3dCommandList, m_pCamera, Mon->HP, pos);
				}
			}
			catch (const exception& e) { cout << "MONSTER HP BAR LOAD ERROR\n"; }
		}
	}

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif


	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	unsigned long frame = m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);

	//int nKeyFrame = pMonsterModel[3].front()->m_pAnimationSets->m_pAnimationSets[6]->m_nKeyFrames;//????3?? ?????????
	//float clipMoStartTime = pMonsterModel[3].front()->m_pAnimationSets->m_pAnimationSets[6]->m_pfKeyFrameTimes[0];//???????0??
	//float clipMoEndTime = pMonsterModel[3].front()->m_pAnimationSets->m_pAnimationSets[6]->m_pfKeyFrameTimes[nKeyFrame-1];//?????? ???????

	//float cycleTime = (clipMoEndTime - clipMoStartTime) / frame * 60;// frame : frameSpeed, 60 : nFramePerSecond
	//
	//cout << "name : " << pMonsterModel[3].front()->m_pAnimationSets->m_pAnimationSets[6]->m_pstrAnimationSetName << endl;
	//cout << "cycleTime : " << cycleTime << endl;

	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}



