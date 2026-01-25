//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


#include <memory>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Xbox One XDK 를 사용하는 경우, 빡센 메모리 관리를 위해 필수.
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

    DirectX::SimpleMath::Vector2 m_screenPos;

    DirectX::SimpleMath::Vector2 m_origin;

    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch; // 스프라이트 배출기 (2D 그리기 도구)

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture; // 텍스쳐 리소스 뷰 (이미지 데이터)
};
