//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "AnimatedTexture.h" // 게임 로직을 담당하는 객체
#include "ScrollingBackground.h" // 게임 로직을 담당하는 객체

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
    void GetDefaultSize( int& width, int& height ) const noexcept;

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

    // 추가된 내용
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch; // 복잡한 정점 버퍼(Vertex Buffer) 설정 없이 간편하게 2D이미지를 그리기 위한 DirectXTK 클래스
    std::unique_ptr<AnimatedTexture> m_ship;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture; // 텍스쳐 파일(.png)을 GPU가 읽을 수 있는 데이터(SRV)로 관리하는 변수.
    DirectX::SimpleMath::Vector2 m_shipPos;

    std::unique_ptr<ScrollingBackground> m_stars;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTex;
};
