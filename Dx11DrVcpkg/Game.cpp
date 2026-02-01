//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    // 추가된 내용
    m_ship->Update(elapsedTime);    // 애니메이션 프레임을 넘긴다.
    m_stars->Update(elapsedTime * 500); // 배경일 이동.

    elapsedTime;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    // 추가된 내용
    m_spriteBatch->Begin();                             // 1. 스프라이트 배치 시작 (상태 설정)
    
    m_stars->Draw(m_spriteBatch.get());                 // 2. 배경 그리기
    m_ship->Draw(m_spriteBatch.get(), m_shipPos);       // 3. 우주선 그리기

    m_spriteBatch->End();                               // 4. 그리기 제출 (Flush)
                                                        // Painter's Algorithm (화가 알고리즘): 먼저 그린 것이 뒤에 깔린다. 따라서 배경(m_stars)을 먼저 그리고, 그 위에 우주선(m_ship)을 그리는 순서가 매우 중요.

    

    context;

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
    m_graphicsMemory->Commit();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);
    
    // TODO: Initialize device dependent objects here (independent of window size).
    // 추가된 내용
    auto context = m_deviceResources->GetD3DDeviceContext();
    m_spriteBatch = std::make_unique<SpriteBatch>(context); // 생성: Context를 받아 그리기 명령 준비.

    DX::ThrowIfFailed(CreateWICTextureFromFile(device, L"shipanimated.png", // 파일을 로드해 SRV를 만든다.
        nullptr, m_texture.ReleaseAndGetAddressOf()));

    m_ship = std::make_unique<AnimatedTexture>();
    m_ship->Load(m_texture.Get(), 4, 20);       // 초기화: 로드된 텍스쳐(SRV)를 각 객체에 연결.

    DX::ThrowIfFailed(CreateWICTextureFromFile(device, L"starfield.png",
        nullptr, m_backgroundTex.ReleaseAndGetAddressOf()));

    m_stars = std::make_unique<ScrollingBackground>();
    m_stars->Load(m_backgroundTex.Get());       // 초기화: 로드된 텍스쳐(SRV)를 각 객체에 연결.
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 추가된 내용
    auto size = m_deviceResources->GetOutputSize();

    // 우주선의 초기 위치를 중앙 하단으로.
    m_shipPos.x = float(size.right / 2);
    m_shipPos.y = float((size.bottom / 2) + (size.bottom / 4));

    // 배경 객체에 현재 화면 크기를 알려주어 스크롤링 범위 갱신
    m_stars->SetWindow(size.right, size.bottom);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // 추가된 내용
    m_ship.reset();
    m_spriteBatch.reset();
    m_texture.Reset();
    m_stars.reset();
    m_backgroundTex.Reset();
    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
