#include "WinWindow.hpp"

#include <WindowsX.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

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

	//////////////////// Root Signature ////////////////////

	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers). The root signature defines the resources the shader
	// programs expect. If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// The following code below creates a root signature that has two root parameter
	// that is a descriptor table large enough to store two CBV (constant buffer view)

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
    cbvTable0.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,  // Number of descriptors in table
		0,  // base shader register arguments are bound to for this root parameter
		0,  // register space
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND   // offset from start of table
	);

    CD3DX12_DESCRIPTOR_RANGE cbvTable1;
    cbvTable1.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,  // Number of descriptors in table
		0,  // base shader register arguments are bound to for this root parameter
		0,  // register space
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND   // offset from start of table
	);

	// Root signature is defined by an array of root parameters that describe the resources the shaders expect for a draw call
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Create root CBVs.
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
    slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, 
    	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a two slots. Each slot points to a descriptor range consisting of a constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	
	if (errorBlob != nullptr) {
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_D3dDevice->CreateRootSignature(
        0,                                      // For single GPU operation, set this to zero
        serializedRootSig->GetBufferPointer(),  // A pointer to the source data for the serialized signature.
        serializedRootSig->GetBufferSize(),     // The size, in bytes, of the block of memory that pBlobWithRootSignature points to.
        IID_PPV_ARGS(&m_RootSignature)          // The globally unique identifier (GUID) for the root signature interface. 
    ));
	
	//////////////////// Shaders And Input Layout ////////////////////

	HRESULT hr = S_OK;
    
	m_Shaders["standardVS"] = DxUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
	m_Shaders["opaquePS"] = DxUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");

    m_InputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

	//////////////////// Frame Resources ////////////////////
	for (int i = 0; i < c_NumFrameResources; i++) {
        m_FrameResources.push_back(std::make_unique<FrameResource>(m_D3dDevice.Get(), 1, (UINT)m_RenderItems.size()));
    }

	//////////////////// Build Render Items ////////////////////

	//////////////////// CONSTANT BUFFER VIEW DESCRIPTOR HEAP ////////////////////

	// heap for the combination of constant-buffer, shader-resource, and unordered-access views.
	UINT objCount = (UINT)m_RenderItems.size();

    // Need a CBV descriptor for each object for each frame resource,
    // +1 for the perPass CBV for each frame resource.
    UINT numDescriptors = (objCount + 1) * c_NumFrameResources;

    // Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
    m_PassCbvOffset = objCount * c_NumFrameResources;

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDescriptors;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_CbvHeap)));

	//////////////////// RENDER ITEMS AND PASS CONSTANT BUFFERS ////////////////////

	UINT objCBByteSize = DxUtils::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT objCount = (UINT)m_RenderItems.size();

    // Need a CBV descriptor for each object for each frame resource.
    for (int frameIndex = 0; frameIndex < c_NumFrameResources; frameIndex++) {
        auto objectCB = m_FrameResources[frameIndex]->objectCB->Resource();
        for (UINT i = 0; i < objCount; i++) {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

            // Offset to the ith object constant buffer in the buffer.
            cbAddress += i * objCBByteSize;

            // Offset to the object cbv in the descriptor heap.
            int heapIndex = frameIndex * objCount + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress;
            cbvDesc.SizeInBytes = objCBByteSize;

            m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }
    }

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    // Last three descriptors are the pass CBVs for each frame resource.
    for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++) {
        auto passCB = mFrameResources[frameIndex]->passCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

        // Offset to the pass cbv in the descriptor heap.
        int heapIndex = m_PassCbvOffset + frameIndex;
        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress;
        cbvDesc.SizeInBytes = passCBByteSize;
        
        m_D3dDevice->CreateConstantBufferView(&cbvDesc, handle);
    }

	//////////////////// Pipeline State Objects ////////////////////

	//
	// PSO for opaque objects.
	//

	CD3DX12_RASTERIZER_DESC rasterDesc(D3D12_DEFAULT);
	rasterDesc.FillMode = D3D12_FILL_SOLID;
	rasterDesc.CullMode = D3D12_CULL_BACK;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.ScissorEnable = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	zeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS = { 
		reinterpret_cast<BYTE*>(m_Shaders["standardVS"]->GetBufferPointer()), 
		m_Shaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = { 
		reinterpret_cast<BYTE*>(m_Shaders["opaquePS"]->GetBufferPointer()),
		m_Shaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = rasterDesc
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
    ThrowIfFailed(m_D3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

	//
    // PSO for opaque wireframe objects.
    //

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(m_D3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque_wireframe"])));

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

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width    = static_cast<float>(m_Props.width);
	m_ScreenViewport.Height   = static_cast<float>(m_Props.height);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = { 0, 0, m_Props.width, m_Props.height };
}


void draw() {
	auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    if (m_IsWireframe) {
        ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque_wireframe"].Get()));
    } else {
        ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));
    }

    m_CommandList->RSSetViewports(1, &m_ScreenViewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    // Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    m_CommandList->ClearRenderTargetView(currentBackBuffer(), Colors::LightSteelBlue, 0, nullptr);
    m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    m_CommandList->OMSetRenderTargets(1, &currentBackBuffer(), true, &depthStencilView());

    ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
    m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

    int passCbvIndex = m_PassCbvOffset + m_CurrFrameResourceIndex;
    auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
    m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    drawRenderItems(m_CommandList.Get(), m_RenderItems);

    // Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(m_CommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    m_CurrFrameResource->Fence = ++m_CurrentFence;
    
    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    m_CommandQueue->Signal(mFence.Get(), m_CurrentFence);
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

void WinWindow::registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes) {
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geometry;
	
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	for (size_t i = 0; i < subGeometries.size(); i++) {
		auto& id = subGeometries[i];
		auto& mesh = subMeshes[i];

		SubmeshGeometry subGeo;
		subGeo.indexCount = (UINT)mesh.indices.size();
		subGeo.startIndexLocation = indices.size();
		subGeo.baseVertexLocation = vertices.size();

		indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

		geo->drawArgs[id] = subGeo;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->vertexBufferCPU));
	CopyMemory(geo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->vertexBufferGPU = DxUtils::CreateDefaultBuffer(m_D3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->vertexBufferUploader);

	geo->indexBufferGPU = DxUtils::CreateDefaultBuffer(m_D3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->indexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	m_Geometries[geo->Name] = std::move(geo);
}

void WinWindow::addRenderItem(const std::string& geometry, const std::string& subGeometry, const glm::mat4& world) {
	auto item = std::make_unique<RenderItem>();
	item->world = world;
	item->objCBIndex = m_RenderItems.size();
	item->geo = m_Geometries[geometry].get();
	item->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	item->indexCount = item->geo->drawArgs[subGeometry].indexCount;
	item->startIndexLocation = item->geo->drawArgs[subGeometry].startIndexLocation;
	item->baseVertexLocation = item->geo->drawArgs[subGeometry].baseVertexLocation;
	m_RenderItems.push_back(std::move(item));
}

void WinWindow::drawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems) {
    // For each render item...
    for (size_t i = 0; i < ritems.size(); i++) {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->geo->vertexBufferView());
        cmdList->IASetIndexBuffer(&ri->geo->indexBufferView());
        cmdList->IASetPrimitiveTopology(ri->primitiveType);

        // Offset to the CBV in the descriptor heap for this object and for this frame resource.
        UINT cbvIndex = m_CurrFrameResourceIndex * (UINT) m_RenderItems.size() + ri->objCBIndex;
        auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
        cbvHandle.Offset(cbvIndex, m_CbvSrvUavDescriptorSize);

        cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

        cmdList->DrawIndexedInstanced(ri->indexCount, 1, ri->startIndexLocation, ri->baseVertexLocation, 0);
    }
}

void WinWindow::update(const int delta) {
    // OnKeyboardInput(gt);
	// UpdateCamera(gt);

    // Cycle through the circular frame resource array.
    m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % c_NumFrameResources;
    m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (m_CurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < m_CurrFrameResource->Fence) {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

	auto currObjectCB = m_CurrFrameResource->objectCB.get();
	for (auto& item : m_RenderItems) {
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0) {
			ObjectConstants objConstants;
			objContanst.world = glm::transpose(item->world);

			currObjectCB->CopyData(item->objCBIndex, objContanst);

			// Next FrameResource need to be updated too.
			item->numFramesDirty--;
		}
	}

	glm::mat4 viewProj = m_Proj * m_View;

	m_MainPassCB.view = glm::transpose(m_View);
	m_MainPassCB.invView = glm::transpose(glm::inverse(m_View));
	m_MainPassCB.proj = glm::transpose(m_View);
	m_MainPassCB.invProj = glm::transpose(glm::inverse(m_View));
	m_MainPassCB.viewProj, glm::transpose(viewProj);
	m_MainPassCB.invViewProj = glm::transpose(glm::inverse(viewProj));
	m_MainPassCB.eyePosW = m_EyePos;
	m_MainPassCB.renderTargetSize = glm::vec2(props.width, props.height);
	m_MainPassCB.invRenderTargetSize = glm::vec2(1.0f / props.width, 1.0f / props.height);
	m_MainPassCB.nearZ = 1.0f;
	m_MainPassCB.farZ = 1000.0f;
	m_MainPassCB.totalTime = 0;
	m_MainPassCB.deltaTime = 0;

	auto currPassCB = m_CurrFrameResource->passCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

ID3D12Resource* WinWindow::currentBackBuffer() const {
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE WinWindow::currentBackBufferView() const {
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE WinWindow::depthStencilView() const {
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

} // namespace Engine
