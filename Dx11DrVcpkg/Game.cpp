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

    // 3. 이미지 그리기 시작
    // SpriteBatch는 내부적으로 VS, PS, IA 단계를 자동으로 설정해준다.
    m_spriteBatch->Begin();

    // 중심점을 기준으로 그리기
    m_spriteBatch->Draw(m_texture.Get(),
        m_screenPos, // 화면상 위치
        nullptr,     // 소스 사각형 (전체 그리기)
        Colors::White,
        0.f,        // 회전 각도 m_rotation 등으로 지정
        m_origin,   // 회전의 중심축
        0.5f        // 스케일
    );

    m_spriteBatch->End();


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
    auto context = m_deviceResources->GetD3DDeviceContext(); // SpriteBatch 생성시 필요
    

    // 1. SpriteBatch 초기화
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device); // GPU 업로드 버퍼 관리
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

    // 2. 텍스처 로드 (PNG 파일 -> GPU 리소스)
    // 파일명이 정확해야 하며, 실행 파일 경로에 있어야 함
    ComPtr<ID3D11Resource> resource;
    //DX::ThrowIfFailed( // 헬퍼 함수 사용. 예외 기반.
    //    CreateWICTextureFromFile(device, L"scarecrow.png",
    //    resource.GetAddressOf(), // 실제 리소스(Texture2D)를 받음
    //    m_texture.ReleaseAndGetAddressOf()) // SRV(Shader Resource View)를 받음
    //); // ID3D11Resource를 먼저 얻고 GetDesc()를 호출

    /*HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        L"scarecrow.png",
        nullptr,
        m_texture.ReleaseAndGetAddressOf()
    );*/ // ID3D11ShaderResourceView만 얻음. 내부 리소스에 직접 접근 X

    HRESULT hr = CreateWICTextureFromFile(
        device,
        L"scarecrow.png",
        resource.GetAddressOf(),
        m_texture.ReleaseAndGetAddressOf()
    ); // FALID(hr) 매크로와 if문 사용. 반환값(Return Code) 기반.

    if (FAILED(hr))
    { // 에러 핸들링 : 로그 출력 후 예외 던지기
        OutputDebugStringA("Error: Failed to load texture 'scarecrow.png'\n");
        throw std::exception("Texture Load Failed");
    }

    // 3. 텍스처 정보(Width, Height) 추출
    // ID3D11Resource는 범용 인터페이스므로 Texture2D로 형변환(QueryInterface/As)이 필요
    ComPtr<ID3D11Texture2D> sprite;
    hr = resource.As(&sprite);

    if (FAILED(hr))
    {
        throw std::exception("Resource is Not a Texture2D");
    }

    CD3D11_TEXTURE2D_DESC spriteDesc;
    sprite->GetDesc(&spriteDesc);

    // 3. 중심점(Origin) 계산 - 회전이나 스케일링의 기준점이 됨
    m_origin.x = float(spriteDesc.Width / 2);
    m_origin.y = float(spriteDesc.Height / 2);

    // 디버깅용 정보 출력
    char debugMSG[64];
    sprintf_s(debugMSG, "Texture Loaded: %d%d\n", spriteDesc.Width, spriteDesc.Height);
    OutputDebugStringA(debugMSG);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.

    auto size = m_deviceResources->GetOutputSize();
    m_screenPos.x = float(size.right) / 2.f;
    m_screenPos.y = float(size.bottom) / 2.f;
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    // 리소스 해제
    m_texture.Reset();
    m_spriteBatch.reset();

    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
