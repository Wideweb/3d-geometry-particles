#pragma once

#include "Window.hpp"
#include "DxRenderObject.hpp"
#include "FrameResource.h"

#define NOMINMAX
#include <windows.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

namespace Engine {

class WinWindow : public Window {
  private:
    WindowProps m_Props;

    HINSTANCE m_AppInstance;
    HWND m_Window;

    D3D12_VIEWPORT m_ScreenViewport; 
    D3D12_RECT m_ScissorRect;

    Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_CurrentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

    static const int c_SwapChainBufferCount = 2;
    uint32_t m_CurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[c_SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

    uint32_t m_RtvDescriptorSize = 0;
    uint32_t m_DsvDescriptorSize = 0;
    uint32_t m_CbvSrvUavDescriptorSize = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeap;

    D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool m_4xMsaaState = false;    // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;  // quality level of 4X MSAA

    const int c_NumFrameResources = 3;
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* m_CurrFrameResource = nullptr;
    int m_CurrFrameResourceIndex = 0;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;

    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_Shaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

    std::vector<std::unique_ptr<RenderItem>> m_RenderItems;

    PassConstants m_MainPassCB;

    UINT m_PassCbvOffset = 0;

    bool m_IsWireframe = false;

    glm::vec3 m_EyePos;
    glm::mat4 m_View;
    glm::mat4 m_Proj;

    EventCallbackFn<MouseEvent &> m_mouseEventCallback;
    EventCallbackFn<WindowEvent &> m_windowEventCallback;
    EventCallbackFn<void *> m_nativeEventCallback;
    MouseEvent m_MouseEvent{};

  public:
    WinWindow(const WindowProps &props);
    virtual ~WinWindow();

    virtual int getWidth() const override;
    virtual int getHeight() const override;
    virtual glm::vec2 getSize() const override;

    virtual void getDrawableSize(int &width, int &height) const override;

    virtual void setMouseEventCallback(const EventCallbackFn<MouseEvent &> &callback) override;

    virtual void setWindowEventCallback(const EventCallbackFn<WindowEvent &> &callback) override;

    virtual void setNativeEventCallback(const EventCallbackFn<void *> &callback) override;

    virtual void readInput() override;
    virtual void swapBuffers() override;
    virtual void shutDown() override;
    virtual void *getNaviteWindow() const override;
    virtual void *getContext() const override;
    virtual MouseEvent &getMouseEvent() override;

    LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    std::shader_ptr<RenderObject> WinWindow::createRenderObject(Mesh mesh)

  private:
    void onResize();
    void flushCommandQueue();
};

} // namespace Engine
