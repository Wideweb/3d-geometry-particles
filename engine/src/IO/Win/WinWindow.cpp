#include "WinWindow.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

#include <WindowsX.h>

#include "DxUtils.hpp"

namespace Engine {

static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WinWindow* window;

    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        window = static_cast<WinWindow*>(lpcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<WinWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window != nullptr) {
        return window->msgProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

WinWindow::WinWindow(const WindowProps &props) {
    using namespace std;
    using namespace std::string_view_literals;
    using Microsoft::WRL::ComPtr;

    m_AppInstance = static_cast<HINSTANCE>(props.appInstance);
    m_Props = props;

    WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = windowProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_AppInstance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "MainWnd";

    if (!RegisterClass(&wc)) {
        cerr << "RegisterClass Failed." << endl;
		return;
	}

    // Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, props.width, props.height };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

    m_Window = CreateWindow(
        "MainWnd",           // Window class
        "Engine",            // Window text
        WS_OVERLAPPEDWINDOW, // Window Style
        CW_USEDEFAULT,       // Window initial horizontal position
        CW_USEDEFAULT,       // Window initial vertical position
        width,               // Window width in device units
        height,              // Window height in device units
        0,                   // Parent window handle
        0,                   // Menu handle
        m_AppInstance,       // Application handle
        this                 // Data
    ); 
	
    if (!m_Window) {
		cerr << "CreateWindow Failed." << endl;
		return;
	}

	ShowWindow(m_Window, SW_SHOW);
    UpdateWindow(m_Window);

    // Enable the D3D12 debug layer.
    #if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
    #endif

    // Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,                  // NULL to use the default adapter
        D3D_FEATURE_LEVEL_11_0,   // The minimum D3D_FEATURE_LEVEL
        IID_PPV_ARGS(&m_D3dDevice) // 
    );

	// Fallback to WARP device.
	if (hardwareResult < 0) {
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_D3dDevice))
        );
	}

    ThrowIfFailed(m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

    // The descriptor heap for the render-target view.
    m_RtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // The descriptor heap for the depth-stencil view.
	m_DsvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // The descriptor heap for the combination of constant-buffer, shader-resource, and unordered-access views.
	m_CbvSrvUavDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render 
    // target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_D3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

    //////////////////////// COMANDS ///////////////////////

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    // Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // Default command queue.
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())
    ));

	ThrowIfFailed(m_D3dDevice->CreateCommandList(
		0,                                // For single-GPU operation, set this to zero. 
		D3D12_COMMAND_LIST_TYPE_DIRECT,   // An optional pointer to the pipeline state object that contains the initial pipeline state for the command list.
		m_DirectCmdListAlloc.Get(),       // Associated command allocator
		nullptr,                          // Initial PipelineStateObject
		IID_PPV_ARGS(m_CommandList.GetAddressOf())
    ));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	m_CommandList->Close();

    //////////////////////// SWAP CHAIN ///////////////////////
    
    // Release the previous swapchain we will be recreating.
    m_SwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = props.width;
    sd.BufferDesc.Height = props.height;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = c_SwapChainBufferCount;
    sd.OutputWindow = m_Window;
    sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;     // Discard the contents of the back buffer after.
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // When switching from windowed to full-screen mode, the display mode (or monitor resolution) will be changed to match the dimensions of the application window.

	// Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(m_DxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd, 
		m_SwapChain.GetAddressOf()
    ));

    //////////////////////// DESCRIPTOR HEAPS ///////////////////////

    // render-target view heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = c_SwapChainBufferCount; // The number of descriptors in the heap.
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;                             // For single-adapter operation, set this to zero.
    ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

    // depth-stencil view heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;                       // The number of descriptors in the heap.
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;                             // For single-adapter operation, set this to zero.
    ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));

    onResize();
}

void WinWindow::onResize() {
    assert(m_D3dDevice);
	assert(m_SwapChain);
    assert(m_DirectCmdListAlloc);

	// Flush before changing any resources.
	flushCommandQueue();

    ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < c_SwapChainBufferCount; i++) {
		m_SwapChainBuffer[i].Reset();
    }
    m_DepthStencilBuffer.Reset();

    // Resize the swap chain.
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
		c_SwapChainBufferCount, 
		m_Props.width, m_Props.height, 
		m_BackBufferFormat, 
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    ));

	m_CurrBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint32_t i = 0; i < c_SwapChainBufferCount; i++) {
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_D3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	}

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_Props.width;
    depthStencilDesc.Height = m_Props.height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_D3dDevice->CreateCommittedResource(&heapProps,
		D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())
    ));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
        m_D3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc,
                                            m_DsvHeap->GetCPUDescriptorHandleForHeapStart());

    // Transition the resource from its initial state to be used as a depth buffer.
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_CommandList->ResourceBarrier(1, &barrier);
	
    // Execute the resize commands.
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	flushCommandQueue();
}

void WinWindow::flushCommandQueue() {
    // Advance the fence value to mark commands up to this fence point.
    m_CurrentFence++;

    // Add an instruction to the command queue to set a new fence point. Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
    if (m_Fence->GetCompletedValue() < m_CurrentFence) {
        HANDLE eventHandle = CreateEvent(nullptr, false, false, "hello");

        // Fire event when GPU hits current fence.  
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

        // Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
	}
}

WinWindow::~WinWindow() {}

int WinWindow::getWidth() const { return m_Props.width; }

int WinWindow::getHeight() const { return m_Props.height; }

glm::vec2 WinWindow::getSize() const { return glm::vec2(m_Props.width, m_Props.height); };

void WinWindow::setMouseEventCallback(const EventCallbackFn<MouseEvent &> &callback) {
    m_mouseEventCallback = callback;
}

void WinWindow::setWindowEventCallback(const EventCallbackFn<WindowEvent &> &callback) {
    m_windowEventCallback = callback;
}

void WinWindow::setNativeEventCallback(const EventCallbackFn<void *> &callback) { m_nativeEventCallback = callback; }

LRESULT WinWindow::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			// mAppPaused = true;
			// mTimer.Stop();
		} else {
			// mAppPaused = false;
			// mTimer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_Props.width  = LOWORD(lParam);
		m_Props.height = HIWORD(lParam);
		if (m_D3dDevice) {
			if (wParam == SIZE_MINIMIZED) {
				// mAppPaused = true;
				// mMinimized = true;
				// mMaximized = false;
			} else if (wParam == SIZE_MAXIMIZED) {
				// mAppPaused = false;
				// mMinimized = false;
				// mMaximized = true;
				onResize();
			} else if(wParam == SIZE_RESTORED) {
				// Restoring from minimized state?
				//if (mMinimized) {
				//	mAppPaused = false;
				//	mMinimized = false;
				//	OnResize();
				//}

				//// Restoring from maximized state?
				//else if (mMaximized) {
				//	mAppPaused = false;
				//	mMaximized = false;
				//	OnResize();
				//} else if (mResizing) {
				//	// If user is dragging the resize bars, we do not resize 
				//	// the buffers here because as the user continuously 
				//	// drags the resize bars, a stream of WM_SIZE messages are
				//	// sent to the window, and it would be pointless (and slow)
				//	// to resize for each WM_SIZE message received from dragging
				//	// the resize bars.  So instead, we reset after the user is 
				//	// done resizing the window and releases the resize bars, which 
				//	// sends a WM_EXITSIZEMOVE message.
				//} else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				//{
				//	OnResize();
				//}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	// case WM_ENTERSIZEMOVE:
	// 	mAppPaused = true;
	// 	mResizing  = true;
	// 	mTimer.Stop();
	// 	return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	// case WM_EXITSIZEMOVE:
	// 	mAppPaused = false;
	// 	mResizing  = false;
	// 	mTimer.Start();
	// 	OnResize();
	// 	return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
	{
        WindowEvent event(EventType::WindowClosed);
        m_windowEventCallback(event);
        PostQuitMessage(0);
        return 0;
	}

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
	{
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
        return 0;
	}

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN: 
	{
		m_MouseEvent = MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseDown);
		m_mouseEventCallback(m_MouseEvent);
		return 0;
    }
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
        m_MouseEvent = MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseUp);
        m_mouseEventCallback(m_MouseEvent);
        return 0;
    }
	case WM_MOUSEMOVE:
	{
        m_MouseEvent = MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseMoved);
        m_mouseEventCallback(m_MouseEvent);
        return 0;
	}
    case WM_KEYUP:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }

        return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WinWindow::readInput() {
    {
        WindowEvent event(EventType::TextInput, "");
        m_windowEventCallback(event);
    }
    
    MSG msg = {0};
    while(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WinWindow::swapBuffers() { }

void *WinWindow::getNaviteWindow() const { return m_Window; }

void *WinWindow::getContext() const { return nullptr; }

MouseEvent &WinWindow::getMouseEvent() { return m_MouseEvent; }

void WinWindow::getDrawableSize(int &width, int &height) const {
    width = m_Props.width;
    height = m_Props.height;
}

void WinWindow::shutDown() {
    if (m_D3dDevice != nullptr) {
		flushCommandQueue();
    }
}

} // namespace Engine
