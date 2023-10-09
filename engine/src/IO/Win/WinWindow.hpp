#pragma once

#include <windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>

namespace Engine {

class WinWindow : public Window {
  private:
    WindowProps m_Props;

    HINSTANCE m_AppInstance;
    HWND *m_Window;
    SDL_GLContext m_Context;

    Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_CurrentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

    static const int c_SwapChainBufferCount = 2;
    uint32_t m_CurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[c_SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

    uint32_t m_RtvDescriptorSize = 0;
    uint32_t m_DsvDescriptorSize = 0;
    uint32_t m_CbvSrvUavDescriptorSize = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

    D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool m_4xMsaaState = false;    // 4X MSAA enabled
    uint32_t m_4xMsaaQuality = 0;  // quality level of 4X MSAA

    EventCallbackFn<MouseEvent &> m_mouseEventCallback;
    EventCallbackFn<WindowEvent &> m_windowEventCallback;
    EventCallbackFn<void *> m_nativeEventCallback;
    MouseEvent m_MouseEvent;

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

  private:
    void onResize();
    void flushCommandQueue();
};

} // namespace Engine
