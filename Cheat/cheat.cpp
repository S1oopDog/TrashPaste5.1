#include "cheat.h"
#include <filesystem>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_win32.h>
#include <HookLib/HookLib.h>
#include "font.h"
#include "iconcpp.h"
#include "icons.h"
#include <iostream>
#include <chrono>
#include <thread>



uintptr_t milliseconds_now() { // Delay func def
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount64();
    }
}
int randomnumber = 0;
// Number Holder

void fn()
{
    int min = -900; // Back of the Target
    int max = +1900; // Front of the Target
    int randomNum = rand() % max + (min); // Random Number Gen
    randomnumber = randomNum; // Changes the value of randomnumber to the value of randomNum
    return; // disables the function because you should call it inside of a loop
}
void fna()
{
    int min = -1500; // Back of the Target
    int max = +2900; // Front of the Target
    int randomNum = rand() % max + (min); // Random Number Gen
    randomnumber = randomNum; // Changes the value of randomnumber to the value of randomNum
    return; // disables the function because you should call it inside of a loop
}
void fnb()
{
    int min = -1900; // Back of the Target
    int max = +3800; // Front of the Target
    int randomNum = rand() % max + (min); // Random Number Gen
    randomnumber = randomNum; // Changes th // Changes the value of randomnumber to the value of randomNum
    return; // disables the function because you should call it inside of a loop
}
float CachedFramerates[1000];

float MaxArr(float arr[], int n)
{
    int i;
    float max = arr[0];
    for (i = 1; i < n; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
    }
    return max;
}

float MinArr(float arr[], int n)
{
    int i;
    float min = arr[0];
    for (i = 1; i < n; i++)
    {
        if (arr[i] < min)
        {
            min = arr[i];
        }
    }
    return min;
}

#define STEAM
namespace fs = std::filesystem;

void Cheat::Hacks::OnWeaponFiredHook(UINT64 arg1, UINT64 arg2)
{
    Logger::Log("arg1: %p, arg2: %p\n", arg1, arg2);
    auto& cameraCache = cache.localCamera->CameraCache.POV;
    auto prev = cameraCache.Rotation;
    cameraCache.Rotation = { -cameraCache.Rotation.Pitch, -cameraCache.Rotation.Yaw, 0.f };
    return OnWeaponFiredOriginal(arg1, arg2);
}

void Cheat::Hacks::ProcessEventHook(void* obj, UFunction* function, void* parms)
{
    Logger::Log("ProcessEvent: %s with id %d\n", function->GetFullName().c_str(), function->Name.ComparisonIndex);
    ProcessEventOriginal(obj, function, parms);
}

void Cheat::Hacks::Init()
{

}

inline void Cheat::Hacks::Remove()
{

}

void Cheat::Renderer::Drawing::RenderText(const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined = false, const bool centered = true)
{
    if (!text) return;
    auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
    if (centered)
    {
        auto size = ImGui::CalcTextSize(text);
        ImScreen.x -= size.x * 0.5f;
        ImScreen.y -= size.y;
    }
    auto window = ImGui::GetCurrentWindow();
    if (cfg.visuals.bEnable && cfg.visuals.textoutlines)
    {
        window->DrawList->AddText(nullptr, 0.f, ImVec2(ImScreen.x - 1.f, ImScreen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
    }
    window->DrawList->AddText(nullptr, 0.f, ImScreen, ImGui::GetColorU32(color), text);

}

void Cheat::Renderer::Drawing::Render2DBox(const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color)
{
    ImGui::GetCurrentWindow()->DrawList->AddRect({ top.X - width * 0.5f, top.Y }, { top.X + width * 0.5f, bottom.Y }, ImGui::GetColorU32(color), 0.f, 15, 1.5f);
}

bool Cheat::Renderer::Drawing::Render3DBox(AController* const controller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color)
{
    FVector vertex[2][4];
    vertex[0][0] = { -extent.X, -extent.Y,  -extent.Z };
    vertex[0][1] = { extent.X, -extent.Y,  -extent.Z };
    vertex[0][2] = { extent.X, extent.Y,  -extent.Z };
    vertex[0][3] = { -extent.X, extent.Y, -extent.Z };
    vertex[1][0] = { -extent.X, -extent.Y, extent.Z };
    vertex[1][1] = { extent.X, -extent.Y, extent.Z };
    vertex[1][2] = { extent.X, extent.Y, extent.Z };
    vertex[1][3] = { -extent.X, extent.Y, extent.Z };
    FVector2D screen[2][4];
    FTransform const Transform(rotation);
    for (auto k = 0; k < 2; k++)
    {
        for (auto i = 0; i < 4; i++)
        {
            auto& vec = vertex[k][i];
            vec = Transform.TransformPosition(vec) + origin;
            if (!controller->ProjectWorldLocationToScreen(vec, screen[k][i])) return false;
        }
    }
    auto ImScreen = reinterpret_cast<ImVec2(&)[2][4]>(screen);
    auto window = ImGui::GetCurrentWindow();
    for (auto i = 0; i < 4; i++)
    {
        window->DrawList->AddLine(ImScreen[0][i], ImScreen[0][(i + 1) % 4], ImGui::GetColorU32(color));
        window->DrawList->AddLine(ImScreen[1][i], ImScreen[1][(i + 1) % 4], ImGui::GetColorU32(color));
        window->DrawList->AddLine(ImScreen[0][i], ImScreen[1][i], ImGui::GetColorU32(color));
    }
    return true;
}

bool Cheat::Renderer::Drawing::RenderSkeleton(AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color)
{
    for (auto s = 0; s < size; s++)
    {
        auto& bone = skeleton[s];
        FVector2D previousBone;
        for (auto i = 0; i < skeleton[s].second; i++)
        {
            FVector loc;
            int res = mesh->GetBone(bone.first[i], comp2world, loc);
            if (res == 0)
            return false;
            FVector2D screen;
            if (!controller->ProjectWorldLocationToScreen(loc, screen)) return false;
            if (previousBone.Size() != 0) {
            auto ImScreen1 = *reinterpret_cast<ImVec2*>(&previousBone);
            auto ImScreen2 = *reinterpret_cast<ImVec2*>(&screen);
            ImGui::GetCurrentWindow()->DrawList->AddLine(ImScreen1, ImScreen2, ImGui::GetColorU32(color), 4.f);
            }
            previousBone = screen;
        }
    }
    return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI Cheat::Renderer::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) && bIsOpen) return true;
    if (bIsOpen)
    {
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        SetCursorOriginal(LoadCursorA(nullptr, win32_cursor));
    }
    if (!bIsOpen || uMsg == WM_KEYUP) return CallWindowProcA(WndProcOriginal, hwnd, uMsg, wParam, lParam);
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

HCURSOR WINAPI Cheat::Renderer::SetCursorHook(HCURSOR hCursor)
{
    if (bIsOpen) return 0;
    return SetCursorOriginal(hCursor);
}

BOOL WINAPI Cheat::Renderer::SetCursorPosHook(int X, int Y)
{
    if (bIsOpen) return FALSE;
    return SetCursorPosOriginal(X, Y);
}

void Cheat::Renderer::HookInput()
{
    RemoveInput();
    WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
    Logger::Log("WndProcOriginal = %p\n", WndProcOriginal);
}

void Cheat::Renderer::RemoveInput()
{
    if (WndProcOriginal)
    {
        SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcOriginal));
        WndProcOriginal = nullptr;
    }
}


bool raytrace(UWorld* world, const struct FVector& start, const struct FVector& end, struct FHitResult* hit)
{
    if (world == nullptr || world->PersistentLevel == nullptr)
    return false;
    return UKismetMathLibrary::LineTraceSingle_NEW((UObject*)world, start, end, ETraceTypeQuery::TraceTypeQuery4, true, TArray<AActor*>(), EDrawDebugTrace::EDrawDebugTrace__None, true, hit);
}

bool GetProjectilePath(std::vector<FVector>& v, FVector& Vel, FVector& Pos, float Gravity, int count, UWorld* world)
{
    float interval = 0.033f;
    for (unsigned int i = 0; i < count; ++i)
    {
        v.push_back(Pos);
        FVector move;
        move.X = (Vel.X) * interval;
        move.Y = (Vel.Y) * interval;
        float newZ = Vel.Z - (Gravity * interval);
        move.Z = ((Vel.Z + newZ) * 0.5f) * interval;
        Vel.Z = newZ;
        FVector nextPos = Pos + move;
        bool res = true;
        {
        FHitResult hit_result;
        res = !raytrace(world, Pos, nextPos, &hit_result);
        }
        Pos = nextPos;
        if (!res) return true;
    }
    return false;
}

#include <complex>
void SolveQuartic(const std::complex<float> coefficients[5], std::complex<float> roots[4]) {
    const std::complex<float> a = coefficients[4];
    const std::complex<float> b = coefficients[3] / a;
    const std::complex<float> c = coefficients[2] / a;
    const std::complex<float> d = coefficients[1] / a;
    const std::complex<float> e = coefficients[0] / a;
    const std::complex<float> Q1 = c * c - 3.f * b * d + 12.f * e;
    const std::complex<float> Q2 = 2.f * c * c * c - 9.f * b * c * d + 27.f * d * d + 27.f * b * b * e - 72.f * c * e;
    const std::complex<float> Q3 = 8.f * b * c - 16.f * d - 2.f * b * b * b;
    const std::complex<float> Q4 = 3.f * b * b - 8.f * c;
    const std::complex<float> Q5 = std::pow(Q2 / 2.f + std::sqrt(Q2 * Q2 / 4.f - Q1 * Q1 * Q1), 1.f / 3.f);
    const std::complex<float> Q6 = (Q1 / Q5 + Q5) / 3.f;
    const std::complex<float> Q7 = 2.f * std::sqrt(Q4 / 12.f + Q6);
    roots[0] = (-b - Q7 - std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 - Q3 / Q7)) / 4.f;
    roots[1] = (-b - Q7 + std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 - Q3 / Q7)) / 4.f;
    roots[2] = (-b + Q7 - std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 + Q3 / Q7)) / 4.f;
    roots[3] = (-b + Q7 + std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 + Q3 / Q7)) / 4.f;
}

#define _USE_MATH_DEFINES
#include <math.h>

FRotator ToFRotator(FVector vec)
{
    FRotator rot;
    float RADPI = (float)(180 / M_PI);
    rot.Yaw = (float)(atan2f(vec.Y, vec.X) * RADPI);
    rot.Pitch = (float)atan2f(vec.Z, sqrt((vec.X * vec.X) + (vec.Y * vec.Y))) * RADPI;
    rot.Roll = 0;
    return rot;
}

int AimAtStaticTarget(const FVector& oTargetPos, float fProjectileSpeed, float fProjectileGravityScalar, const FVector& oSourcePos, FRotator& oOutLow, FRotator& oOutHigh) {
    const float gravity = 981.f * fProjectileGravityScalar;
    const FVector diff(oTargetPos - oSourcePos);
    const FVector oDiffXY(diff.X, diff.Y, 0.0f);
    const float fGroundDist = oDiffXY.Size();
    const float s2 = fProjectileSpeed * fProjectileSpeed;
    const float s4 = s2 * s2;
    const float y = diff.Z;
    const float x = fGroundDist;
    const float gx = gravity * x;
    float root = s4 - (gravity * ((gx * x) + (2 * y * s2)));
    if (root < 0)
    return 0;
    root = std::sqrtf(root);
    const float fLowAngle = std::atan2f((s2 - root), gx);
    const float fHighAngle = std::atan2f((s2 + root), gx);
    int nSolutions = fLowAngle != fHighAngle ? 2 : 1;
    const FVector oGroundDir(oDiffXY.unit());
    oOutLow = ToFRotator(oGroundDir * std::cosf(fLowAngle) * fProjectileSpeed + FVector(0.f, 0.f, 1.f) * std::sinf(fLowAngle) * fProjectileSpeed);
    if (nSolutions == 2)
    oOutHigh = ToFRotator(oGroundDir * std::cosf(fHighAngle) * fProjectileSpeed + FVector(0.f, 0.f, 1.f) * std::sinf(fHighAngle) * fProjectileSpeed);
    return nSolutions;
}

#include <limits>
int AimAtMovingTarget(const FVector& oTargetPos, const FVector& oTargetVelocity, float fProjectileSpeed, float fProjectileGravityScalar, const FVector& oSourcePos, const FVector& oSourceVelocity, FRotator& oOutLow, FRotator& oOutHigh) {
    const FVector v(oTargetVelocity - oSourceVelocity);
    const FVector g(0.f, 0.f, -981.f * fProjectileGravityScalar);
    const FVector p(oTargetPos - oSourcePos);
    const float c4 = g | g * 0.25f;
    const float c3 = v | g;
    const float c2 = (p | g) + (v | v) - (fProjectileSpeed * fProjectileSpeed);
    const float c1 = 2.f * (p | v);
    const float c0 = p | p;
    std::complex<float> pOutRoots[4];
    const std::complex<float> pInCoeffs[5] = { c0, c1, c2, c3, c4 };
    SolveQuartic(pInCoeffs, pOutRoots);
    float fBestRoot = FLT_MAX;
    for (int i = 0; i < 4; i++) {
    if (pOutRoots[i].real() > 0.f && std::abs(pOutRoots[i].imag()) < 0.0001f && pOutRoots[i].real() < fBestRoot) {
    fBestRoot = pOutRoots[i].real();
    }
    }
    if (fBestRoot == FLT_MAX)
    return 0;
    const FVector oAimAt = oTargetPos + (v * fBestRoot);
    return AimAtStaticTarget(oAimAt, fProjectileSpeed, fProjectileGravityScalar, oSourcePos, oOutLow, oOutHigh);
}

FVector2D rotate_radar(FVector target_location, FVector source_location, FRotator source_rotation)
{
    const FVector diff_loc = target_location - source_location;
    float yaw = source_rotation.Yaw;
    yaw *= M_PI / 180.f;
    float x_rot = diff_loc.Y * std::cosf(yaw) - diff_loc.X * std::sinf(yaw);
    float y_rot = diff_loc.X * std::cosf(yaw) + diff_loc.Y * std::sinf(yaw);
    return { x_rot, y_rot };
}

HRESULT Cheat::Renderer::PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
    if (!device)
    {
        ID3D11Texture2D* surface = nullptr;
        goto init;
    cleanup:
        Cheat::Remove();
        if (surface) surface->Release();
        return fnPresent(swapChain, syncInterval, flags);
    init:
        if (FAILED(swapChain->GetBuffer(0, __uuidof(surface), reinterpret_cast<PVOID*>(&surface)))) { goto cleanup; };
        Logger::Log("ID3D11Texture2D* surface = %p\n", surface);
        if (FAILED(swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device)))) goto cleanup;
        Logger::Log("ID3D11Device* device = %p\n", device);
        if (FAILED(device->CreateRenderTargetView(surface, nullptr, &renderTargetView))) goto cleanup;
        Logger::Log("ID3D11RenderTargetView* renderTargetView = %p\n", renderTargetView);
        surface->Release();
        surface = nullptr;
        device->GetImmediateContext(&context);
        Logger::Log("ID3D11DeviceContext* context = %p\n", context);
        ImGui::CreateContext();
        {
            ImGuiIO& io = ImGui::GetIO();
            static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
            ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 17);
            ImFontConfig config;
            config.MergeMode = true;
            io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.0f, &config, icons_ranges);
            io.Fonts->Build();
        }
#ifdef STEAM
        DXGI_SWAP_CHAIN_DESC desc;
        swapChain->GetDesc(&desc);
        auto& window = desc.OutputWindow;
        gameWindow = window;
#else
        auto window = FindWindowA(NULL, "Sea of Thieves");
        gameWindow = window;
#endif
        Logger::Log("gameWindow = %p\n", window);
        if (!ImGui_ImplWin32_Init(window)) goto cleanup;
        if (!ImGui_ImplDX11_Init(device, context)) goto cleanup;
        if (!ImGui_ImplDX11_CreateDeviceObjects()) goto cleanup;
        Logger::Log("ImGui initialized successfully!\n");
        HookInput();
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("#1", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    auto& io = ImGui::GetIO();
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
    auto drawList = ImGui::GetCurrentWindow()->DrawList;
    try
    {
        do
        {      
            memset(&cache, 0, sizeof(Cache));
            auto const world = *UWorld::GWorld;
            if (!world) break;
            auto const game = world->GameInstance;
            if (!game) break;
            auto const gameState = world->GameState;
            if (!gameState) break;
            cache.gameState = gameState;
            if (!game->LocalPlayers.Data) break;
            auto const localPlayer = game->LocalPlayers[0];
            if (!localPlayer) break;
            auto const localController = localPlayer->PlayerController;
            if (!localController) break;
            cache.localController = localController;
            auto const camera = localController->PlayerCameraManager;
            if (!camera) break;
            cache.localCamera = camera;
            const auto cameraLoc = camera->GetCameraLocation();
            const auto cameraRot = camera->GetCameraRotation();
            auto const localCharacter = localController->Character;
            if (!localCharacter) break;
            auto const localPlayerCharacter = (AAthenaPlayerCharacter*)localController->K2_GetPawn();
            if (!localPlayerCharacter) break;
            auto const AACharacter = (AAthenaPlayerCharacter*)localCharacter;
            if (!localPlayerCharacter) break;
            const auto levels = world->Levels;
            if (!levels.Data) break;
            const auto localLoc = localCharacter->K2_GetActorLocation();
            bool isWieldedWeapon = false;
            auto item = localCharacter->GetWieldedItem();                
            if (item)
            isWieldedWeapon = item->isWeapon();
            auto const localWeapon = *reinterpret_cast<AProjectileWeapon**>(&item);
            ACharacter* attachObject = localCharacter->GetAttachParentActor();
            cache.good = true;
            static struct {
                ACharacter* target = nullptr;
                FVector location;
                FRotator delta;
                float best = FLT_MAX;
                float smoothness = 1.f;
            } aimBest;
            aimBest.target = nullptr;
            aimBest.best = FLT_MAX;
            if (cfg.aim.cannon.bEnable)
            {
                if (ImGui::IsKeyPressed(VK_F1))
                {
                    if (cfg.aim.cannon.b_chain_shots == true)
                    {
                        cfg.aim.cannon.b_chain_shots = false;
                    }
                    else
                    {
                        if (cfg.aim.cannon.b_chain_shots == false)
                        {
                            cfg.aim.cannon.b_chain_shots = true;
                            cfg.aim.cannon.playeraimbot = false;
                            cfg.aim.cannon.deckshots = false;
                            cfg.aim.cannon.randomshots = false;
                        }
                    }
                }
                if (ImGui::IsKeyPressed(VK_F2))
                {
                    if (cfg.aim.cannon.randomshots == true)
                    {
                        cfg.aim.cannon.randomshots = false;
                    }
                    else
                    {
                        if (cfg.aim.cannon.randomshots == false)
                        {
                            cfg.aim.cannon.randomshots = true;
                            cfg.aim.cannon.b_chain_shots = false;
                            cfg.aim.cannon.playeraimbot = false;
                            cfg.aim.cannon.deckshots = false;
                        }
                    }
                }
                if (ImGui::IsKeyPressed(VK_F4))
                {
                    if (cfg.aim.cannon.deckshots == true)
                    {
                        cfg.aim.cannon.deckshots = false;
                    }
                    else
                    {
                        if (cfg.aim.cannon.deckshots == false)
                        {
                            cfg.aim.cannon.deckshots = true;
                            cfg.aim.cannon.b_chain_shots = false;
                            cfg.aim.cannon.playeraimbot = false;
                            cfg.aim.cannon.randomshots = false;
                        }
                    }
                }
                if (ImGui::IsKeyPressed(VK_F3))
                {
                    if (cfg.aim.cannon.playeraimbot == true)
                    {
                        cfg.aim.cannon.playeraimbot = false;
                    }
                    else
                    {
                        if (cfg.aim.cannon.playeraimbot == false)
                        {
                            cfg.aim.cannon.playeraimbot = true;
                            cfg.aim.cannon.randomshots = false;
                            cfg.aim.cannon.deckshots = false;
                            cfg.aim.cannon.b_chain_shots = false;
                        }
                    }
                }
                if (ImGui::IsKeyPressed(VK_F5))
                {
                    if (cfg.misc.macro.b_bunnyhop == true)
                    {
                        cfg.misc.macro.b_bunnyhop = false;
                    }
                    else
                    {
                        if (cfg.misc.macro.b_bunnyhop == false)
                        {
                            cfg.misc.macro.b_bunnyhop = true;
                        }
                    }
                }
            }
            if (attachObject && attachObject->isCannon())
            {
                auto cannon = reinterpret_cast<ACannon*>(attachObject);
                if (cannon->LoadedItemInfo)
                {
                    std::wstring loaded_name = cannon->LoadedItemInfo->Desc->Title->wide();
                    if (L"Chainshot" == loaded_name)
                        cfg.aim.cannon.b_chain_shots = true;
                    else
                        cfg.aim.cannon.b_chain_shots = false;
                    if (L"Player" == loaded_name)
                        cfg.aim.cannon.deckshots = true;
                    else
                        cfg.aim.cannon.deckshots = false;
                }
                float gravity_scale = cannon->ProjectileGravityScale;
                if (cfg.aim.cannon.b_chain_shots)
                {
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1686.f, 345.f), ImColor(0, 0, 0, 255), "Chain Aimbot On", 0, 0.0f, 0);
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1685.f, 343.f), ImColor(255, 0, 0, 255), "Chain Aimbot On", 0, 0.0f, 0);
                    gravity_scale = 1.f;
                }
                if (cfg.aim.cannon.deckshots)
                {
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1694.f, 365.f), ImColor(0, 0, 0, 255), "Deck Shots On", 0, 0.0f, 0);
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1692.f, 363.f), ImColor(255, 255, 0, 255), "Deck Shots On", 0, 0.0f, 0);
                    gravity_scale = cannon->ProjectileGravityScale;
                }
                if (cfg.aim.cannon.playeraimbot)
                {
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1656.f, 325.f), ImColor(0, 0, 0, 255), "Player Cannon Aimbot On", 0, 0.0f, 0);
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1655.f, 323.f), ImColor(255, 0, 255, 255), "Player Cannon Aimbot On", 0, 0.0f, 0);
                }
                if (cfg.aim.cannon.randomshots)
                {
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1626.f, 305.f), ImColor(0, 0, 0, 255), "Randomshots Aimbot On", 0, 0.0f, 0);
                    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1625.f, 303.f), ImColor(255, 255, 0, 255), "Randomshots Aimbot On", 0, 0.0f, 0);
                }
                
            }
            if (cfg.visuals.bEnable && !localCharacter->IsLoading())
            {                
                if (cfg.visuals.client.bCompass)
                {
                    const char* directions[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
                    int yaw = ((int)cameraRot.Yaw + 450) % 360;
                    int index = int(yaw + 22.5f) % 360 * 0.0222222f;
                    FVector2D pos = { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.02f };
                    auto col = ImVec4(1.f, 1.f, 1.f, 1.f);
                    Drawing::RenderText(const_cast<char*>(directions[index]), pos, col);
                    char buf[0x30];
                    int len = sprintf(buf, "%d", yaw);
                    pos.Y += 15.f;
                    Drawing::RenderText(buf, pos, col);
                }
                if (cfg.visuals.client.bOxygen && localCharacter->IsInWater())
                {
                    auto drownComp = localCharacter->DrowningComponent;
                    if (!drownComp) break;
                    auto level = drownComp->GetOxygenLevel();
                    auto posX = io.DisplaySize.x * 0.5f;
                    auto posY = io.DisplaySize.y * 0.85f;
                    auto barWidth2 = io.DisplaySize.x * 0.05f;
                    auto barHeight2 = io.DisplaySize.y * 0.0030f;
                    drawList->AddRectFilled({ posX - barWidth2, posY - barHeight2 }, { posX + barWidth2, posY + barHeight2 }, ImGui::GetColorU32(IM_COL32(0, 0, 0, 255)));
                    drawList->AddRectFilled({ posX - barWidth2, posY - barHeight2 }, { posX - barWidth2 + barWidth2 * level * 2.f, posY + barHeight2 }, ImGui::GetColorU32(IM_COL32(0, 200, 255, 255)));
                }
                if (cfg.visuals.client.bCrosshair)
                {
                    drawList->AddLine({ io.DisplaySize.x * 0.5f - cfg.visuals.client.fCrosshair, io.DisplaySize.y * 0.5f }, { io.DisplaySize.x * 0.5f + cfg.visuals.client.fCrosshair, io.DisplaySize.y * 0.5f }, ImGui::GetColorU32(cfg.visuals.client.crosshairColor));
                    drawList->AddLine({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f - cfg.visuals.client.fCrosshair }, { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f + cfg.visuals.client.fCrosshair }, ImGui::GetColorU32(cfg.visuals.client.crosshairColor));
                }
                if (cfg.visuals.islands.bEnable)
                {
                    if (cfg.visuals.islands.bName)
                    {
                        do
                        {
                            auto const islandService = gameState->IslandService;
                            if (!islandService) break;
                            auto const islandDataAsset = islandService->IslandDataAsset;
                            if (!islandDataAsset) break;
                            auto const islandDataEntries = islandDataAsset->IslandDataEntries;
                            if (!islandDataEntries.Data)break;
                            for (auto i = 0u; i < islandDataEntries.Count; i++)
                            {
                                auto const island = islandDataEntries[i];
                                auto const WorldMapData = island->WorldMapData;
                                if (!WorldMapData) continue;
                                const FVector islandLoc = WorldMapData->WorldSpaceCameraPosition;
                                const int dist = localLoc.DistTo(islandLoc) * 0.01f;
                                if (dist > cfg.visuals.islands.intMaxDist) continue;
                                FVector2D screen;
                                if (localController->ProjectWorldLocationToScreen(islandLoc, screen))
                                {
                                    char buf[0x64];
                                    auto len = island->LocalisedName->multi(buf, 0x50);
                                    sprintf_s(buf + len, sizeof(buf) - len, " [%dm]", dist);
                                    Drawing::RenderText(buf, screen, cfg.visuals.islands.textCol);
                                }
                            }
                        } while (false);
                    }
                }
            }
            if (cfg.misc.bEnable && !localCharacter->IsLoading())
            {
                if (cfg.misc.client.bEnable)
                {
                    if (cfg.misc.client.bShipInfo)
                    {
                        auto ship = localCharacter->GetCurrentShip();
                        if (ship)
                        {
                            FVector velocity = ship->GetVelocity() / 100.f;
                            char buf[0xFF];
                            FVector2D pos{ 1.f, 45.f };
                            ImVec4 col{ 1.f,1.f,1.f,1.f };
                            auto speed = velocity.Size();
                            sprintf(buf, "Speed: %.0fm/s", speed);
                            pos.Y += 5.f;
                            Drawing::RenderText(buf, pos, col, true, false);
                            int holes = ship->GetHullDamage()->ActiveHullDamageZones.Count;
                            sprintf(buf, "Holes: %d", holes);
                            pos.Y += 20.f;
                            Drawing::RenderText(buf, pos, col, true, false);
                            int amount = 0;
                            auto water = ship->GetInternalWater();
                            if (water) amount = water->GetNormalizedWaterAmount() * 100.f;
                            sprintf(buf, "Water: %d%%", amount);
                            pos.Y += 20.f;
                            Drawing::RenderText(buf, pos, col, true, false);
                            pos.Y += 22.f;
                            float internal_water_percent = ship->GetInternalWater()->GetNormalizedWaterAmount();
                            drawList->AddLine({ pos.X - 1, pos.Y }, { pos.X + 100 + 1, pos.Y }, 0xFF000000, 6);
                            drawList->AddLine({ pos.X, pos.Y }, { pos.X + 100, pos.Y }, 0xFF00FF00, 4);
                            drawList->AddLine({ pos.X, pos.Y }, { pos.X + (100.f * internal_water_percent), pos.Y }, 0xFF0000FF, 4);
                        }
                    }
                    static std::uintptr_t desiredTime = 0;
                    if (milliseconds_now() >= desiredTime)
                    {
                        int return_value = (int)localCharacter->GetTargetFOV(AACharacter);;
                        localController->FOV(cfg.misc.client.fov);
                        if (return_value == 17.f)
                        {
                            localController->FOV(cfg.misc.client.fov * 0.2f);
                        }
                        desiredTime = milliseconds_now() + 100;
                    }   
                }
                if (cfg.misc.macro.bEnable)
                {
                    if (cfg.misc.macro.b_bunnyhop)
                    {
                        static std::uintptr_t desiredTime = 0;
                        if (milliseconds_now() >= desiredTime)
                        {
                            if (ImGui::IsKeyPressed(VK_SPACE))
                            {
                                if (localCharacter->CanJump())
                                {
                                    localCharacter->Jump();
                                }
                            }
                            desiredTime = milliseconds_now() + 10;
                        }                    
                    }
                }
                if (cfg.misc.game.bEnable)
                {
                    if (cfg.misc.game.bShowPlayers)
                    {
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar(2);
                        ImGui::SetNextWindowSize(ImVec2(335.f, 700.f), ImGuiCond_Once);
                        ImGui::SetNextWindowPos(ImVec2(10.f, 180.f), ImGuiCond_Once);
                        ImGui::Begin("PlayersList", 0, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysAutoResize | ImGuiColumnsFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
                        auto shipsService = gameState->ShipService;
                        if (shipsService)
                        {
                            ImGui::BeginChild("Info", { 0.f, 18.f });
                            ImGui::Text("Server Player List | Total Ships (Including AI): %d", shipsService->GetNumShips());
                            ImGui::EndChild();
                        }
                        auto crewService = gameState->CrewService;
                        auto crews = crewService->Crews;
                        if (crews.Data)
                        {
                            ImGui::Columns(2, "CrewPlayers", ImGuiColumnsFlags_NoResize);
                            ImGui::Separator();
                            ImGui::Text("Name"); ImGui::NextColumn();
                            ImGui::SetColumnOffset(1, 185.0f);
                            ImGui::Text("Activity"); ImGui::NextColumn();
                            ImGui::Separator();
                            for (uint32_t i = 0; i < crews.Count; i++)
                            {
                                auto& crew = crews[i];
                                auto players = crew.Players;
                                if (players.Data)
                                {
                                    for (uint32_t k = 0; k < players.Count; k++)
                                    {
                                        auto& player = players[k];
                                        char buf[0x64];
                                        player->PlayerName.multi(buf, 0x50);
                                        ImGui::Text(buf);
                                        ImGui::NextColumn();
                                        const char* actions[] = { "None", "Bailing", "Cannon", "Cannon_END", "Anchor", "Anchor_END", "Carrying Item", "Carrying Item_END", "Dead", "Dead_END", "Digging", "Extinguishing Fire", "Emptying Bucket", "Harpoon", "Harpoon_END", "Losing Health", "Repairing", "Sails", "Sails_END", "Undoing Repair", "Wheel", "Wheel_END" };
                                        auto activity = (uint8_t)player->GetPlayerActivity();
                                        if (activity < 21) { ImGui::Text(actions[activity]); }
                                        ImGui::NextColumn();
                                    }
                                    ImGui::Separator();
                                }
                            }
                            ImGui::Columns();
                        }
                        ImGui::End();
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                    }
                }
            }
            for (auto l = 0u; l < levels.Count; l++)
            {
                auto const level = levels[l];
                if (!level) continue;
                const auto actors = level->AActors;
                if (!actors.Data) continue;
                for (auto a = 0u; a < actors.Count; a++)
                {
                    auto const actor = actors[a];
                    if (!actor) continue;
                    {
                    }
                    if (cfg.visuals.bEnable && !localCharacter->IsLoading())
                    {                        
                        if (cfg.visuals.ships.bEnable)
                        {
                            FVector location = actor->K2_GetActorLocation();
                            location.Z = 3000;
                            const int dist = localLoc.DistTo(location) * 0.01f;
                            if (cfg.visuals.ships.bName && actor->isShip() && dist < 1726)
                            {
                                int amount = 0;
                                FVector2D screen;
                                FVector velocity = actor->GetVelocity() / 100.f;
                                auto speed = velocity.Size();
                                if (localController->ProjectWorldLocationToScreen(location, screen))
                                {
                                    auto water = actor->GetInternalWater();
                                    if (water) amount = water->GetNormalizedWaterAmount() * 100.f;
                                    auto type = actor->GetName();
                                    char buf[0x64];
                                    if (type.find("BP_Small") != std::string::npos)
                                        sprintf(buf, "Sloop (%d%% Water) [%dm] [%.0fm/s]", amount, dist, speed);
                                    else if (type.find("BP_Medium") != std::string::npos)
                                        sprintf(buf, "Brig (%d%% Water) [%dm] [%.0fm/s]", amount, dist, speed);
                                    else if (type.find("BP_Large") != std::string::npos)
                                        sprintf(buf, "Galleon (%d%% Water) [%dm] [%.0fm/s]", amount, dist, speed);
                                    else if (type.find("AISmall") != std::string::npos)
                                        sprintf(buf, "Skeleton Sloop (%d%% Water) [%dm] [%.0fm/s]", amount, dist, speed);
                                    else if (type.find("AILarge") != std::string::npos)
                                        sprintf(buf, "Skeleton Galleon (%d%% Water) [%dm] [%.0fm/s]", amount, dist, speed);
                                    Drawing::RenderText(buf, screen, cfg.visuals.ships.textCol);
                                }
                            }
                            if (cfg.visuals.ships.bName && actor->isFarShip() && dist > 1725)
                            {
                                int amount = 0;
                                FVector2D screen;
                                FVector velocity = actor->GetVelocity() / 100.f;
                                auto speed = velocity.Size();
                                if (localController->ProjectWorldLocationToScreen(location, screen))
                                {
                                    auto type = actor->GetName();
                                    char buf[0x64];
                                    if (type.find("BP_Small") != std::string::npos)
                                        sprintf(buf, "Sloop [%dm]", dist);
                                    else if (type.find("BP_Medium") != std::string::npos)
                                        sprintf(buf, "Brig [%dm]", dist);
                                    else if (type.find("BP_Large") != std::string::npos)
                                        sprintf(buf, "Galleon [%dm]", dist);
                                    else if (type.find("BP_AISmall") != std::string::npos)
                                        sprintf(buf, "Skeleton Sloop [%dm]", dist);
                                    else if (type.find("BP_AILarge") != std::string::npos)
                                        sprintf(buf, "Skeleton Galleon [%dm]", dist);
                                    Drawing::RenderText(buf, screen, cfg.visuals.ships.textCol);
                                }
                            }
                            if (actor->isShip())
                            {
                                const FVector location = actor->K2_GetActorLocation();
                                const int dist = localLoc.DistTo(location) * 0.01f;
                                FVector2D screen;
                                if (cfg.visuals.ships.bDamage && dist <= 320)
                                {
                                    auto const damage = actor->GetHullDamage();
                                    if (!damage) continue;
                                    const auto holes = damage->ActiveHullDamageZones;
                                    for (auto h = 0u; h < holes.Count; h++)
                                    {
                                        auto const hole = holes[h];
                                        const FVector location = hole->K2_GetActorLocation();
                                        if (localController->ProjectWorldLocationToScreen(location, screen))
                                        {
                                            auto color = cfg.visuals.ships.damageColor;
                                            drawList->AddLine({ screen.X - 6.f, screen.Y + 6.f }, { screen.X + 6.f, screen.Y - 6.f }, ImGui::GetColorU32(color));
                                            drawList->AddLine({ screen.X - 6.f, screen.Y - 6.f }, { screen.X + 6.f, screen.Y + 6.f }, ImGui::GetColorU32(color));
                                        }
                                    }
                                }
                            }                        
                        }
                        if (cfg.visuals.rowboats.bEnable)
                        {
                            if (cfg.visuals.rowboats.bName)
                            {
                                if (actor->isRowboat())
                                {
                                    const FVector location = actor->K2_GetActorLocation();
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    if (dist > cfg.visuals.rowboats.drawdistance) continue;
                                    FVector2D screen;
                                    if (localController->ProjectWorldLocationToScreen(location, screen))
                                    {
                                        auto type = actor->GetName();
                                        char buf[0x64];
                                        if (type.find("Cannon") != std::string::npos)
                                            sprintf(buf, "Cannon Rowboat [%dm]", dist);
                                        else if (type.find("Harpoon") != std::string::npos)
                                            sprintf(buf, "Harpoon Rowboat [%dm]", dist);
                                        else if (type.find("Rowboat") != std::string::npos)
                                            sprintf(buf, "Rowboat [%dm]", dist);
                                        Drawing::RenderText(buf, screen, cfg.visuals.rowboats.textCol);
                                    }
                                }
                            }
                        }
                        if (cfg.visuals.world.bEnable)
                        {
                            if (actor->isEvent())
                            {
                                const FVector location = actor->K2_GetActorLocation();
                                FVector2D screen;
                                if (localController->ProjectWorldLocationToScreen(location, screen))
                                {
                                    auto type = actor->GetName();
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    if (dist > cfg.visuals.world.drawdistance) continue;
                                    char buf[0x64];
                                    if (type.find("ShipCloud") != std::string::npos)
                                        sprintf(buf, "Fleet [%dm]", dist);
                                    else if (type.find("AshenLord") != std::string::npos)
                                        sprintf(buf, "Ashen Lord [%dm]", dist);
                                    else if (type.find("Flameheart") != std::string::npos)
                                        sprintf(buf, "Flame Heart [%dm]", dist);
                                    else if (type.find("LegendSkellyFort") != std::string::npos)
                                        sprintf(buf, "Fort of Fortune [%dm]", dist);
                                    else if (type.find("SkellyFortOfTheDamned") != std::string::npos)
                                        sprintf(buf, "FOTD [%dm]", dist);
                                    else if (type.find("BP_SkellyFort") != std::string::npos)
                                        sprintf(buf, "Skull Fort [%dm]", dist);
                                    else if (type.find("BP_PhantomTornado_C") != std::string::npos)
                                        sprintf(buf, "Sharknado [%dm]", dist);
                                    Drawing::RenderText(buf, screen, cfg.visuals.world.textCol);
                                }
                            }
                        }
                        if (cfg.visuals.items.bEnable)
                        {
                            if (cfg.visuals.items.bName)
                            {
                                auto location = actor->K2_GetActorLocation();
                                FVector2D screen;
                                if (actor->isItem())
                                {
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    if (dist > cfg.visuals.items.itemdrawdistance) continue;
                                    if (localController->ProjectWorldLocationToScreen(location, screen))
                                    {
                                        auto const desc = actor->GetItemInfo()->Desc;
                                        if (!desc) continue;
                                        char buf[0x64];
                                        const int len = desc->Title->multi(buf, 0x50);
                                        snprintf(buf + len, sizeof(buf) - len, " [%dm]", dist);
                                        Drawing::RenderText(buf, screen, cfg.visuals.items.textCol);
                                    }
                                }
                            }
                            if (cfg.visuals.items.barrelitems)
                            {
                                const FVector location = actor->K2_GetActorLocation();
                                FVector2D screen;
                                if (actor->isBarrel())
                                {
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    if (dist > cfg.visuals.items.barreldrawdistance) continue;
                                    if (localController->ProjectWorldLocationToScreen(location, screen))
                                    {
                                        char buf[0x64];
                                        sprintf(buf, "B");
                                        Drawing::RenderText(buf, screen, cfg.visuals.items.barreltextCol);
                                    }
                                }
                            }
                            if (cfg.visuals.items.ammochests)
                            {
                                const FVector location = actor->K2_GetActorLocation();
                                FVector2D screen;
                                if (actor->isAmmoChest())
                                {
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    if (dist > cfg.visuals.items.ammochestsdrawdistance) continue;
                                    if (localController->ProjectWorldLocationToScreen(location, screen))
                                    {
                                        char buf[0x64];
                                        sprintf(buf, "Ammo Chest [%dm]", dist);
                                        Drawing::RenderText(buf, screen, cfg.visuals.items.ammotextCol);
                                    }
                                }
                            }                                              
                        }
                        if (cfg.visuals.skeletons.bEnable && actor->isSkeleton() && !actor->IsDead())
                        {
                            const FVector location = actor->K2_GetActorLocation();
                            const int dist = localLoc.DistTo(location) * 0.01f;
                            if (dist > cfg.visuals.skeletons.drawdistance) continue;
                            FVector origin, extent;
                            actor->GetActorBounds(true, origin, extent);
                            FVector2D headPos;
                            if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos)) continue;
                            FVector2D footPos;
                            if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos)) continue;
                            const float height = abs(footPos.Y - headPos.Y);
                            const float width = height * 0.4f;
                            const bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                            const ImVec4 col = bVisible ? cfg.visuals.skeletons.colorVis : cfg.visuals.skeletons.colorInv;
                            switch (cfg.visuals.skeletons.boxType)
                            {
                            case Config::EBox::E2DBoxes:
                            {
                                Drawing::Render2DBox(headPos, footPos, height, width, col);
                                break;
                            }
                            case Config::EBox::E3DBoxes:
                            {
                                FRotator rotation = actor->K2_GetActorRotation();
                                if (!Drawing::Render3DBox(localController, origin, extent, rotation, col)) continue;
                                break;
                            }
                            }
                            if (cfg.visuals.skeletons.bName)
                            {
                                char buf[0x20];
                                sprintf(buf, "Skeleton [%dm]", dist);
                                Drawing::RenderText(buf, headPos, cfg.visuals.skeletons.textCol);
                            }
                            if (cfg.visuals.skeletons.barType != Config::EBar::ENone)
                            {
                                auto const healthComp = actor->HealthComponent;
                                if (!healthComp) continue;
                                const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
                                const float width2 = width * 0.5f;
                                const float adjust = height * 0.025f;
                                switch (cfg.visuals.skeletons.barType)
                                {
                                case Config::EBar::ELeft:
                                {
                                    const float len = height * hp;
                                    drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    break;
                                }
                                case Config::EBar::ERight:
                                {
                                    const float len = height * hp;
                                    drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    break;
                                }
                                case Config::EBar::EBottom:
                                {
                                    const float len = width * hp;
                                    drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    break;
                                }
                                case Config::EBar::ETop:
                                {
                                    const float len = width * hp;
                                    drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    break;
                                }
                                }
                            }
                        }
                        if (cfg.visuals.shipwrecks.bEnable && actor->isShipwreck())
                        {
                            auto location = actor->K2_GetActorLocation();
                            FVector2D screen;
                            if (localController->ProjectWorldLocationToScreen(location, screen))
                            {
                                const int dist = localLoc.DistTo(location) * 0.01f;
                                if (dist > cfg.visuals.shipwrecks.drawdistance) continue;
                                char buf[0x64];
                                sprintf(buf, "Shipwreck [%dm]", dist);
                                Drawing::RenderText(buf, screen, cfg.visuals.shipwrecks.textCol);
                            }
                        }
                        else if (cfg.visuals.players.bEnable && actor->isPlayer() && actor != localCharacter && !actor->IsDead())
                        {
                            const bool teammate = UCrewFunctions::AreCharactersInSameCrew(actor, localCharacter);
                            if (teammate && !cfg.visuals.players.bDrawTeam) continue;
                            FVector origin, extent;
                            actor->GetActorBounds(true, origin, extent);
                            const FVector location = actor->K2_GetActorLocation();
                            FVector2D headPos;
                            if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos)) continue;
                            FVector2D footPos;
                            if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos)) continue;
                            const float height = abs(footPos.Y - headPos.Y);
                            const float width = height * 0.4f;
                            const bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                            ImVec4 col;
                            if (teammate) col = bVisible ? cfg.visuals.players.teamColorVis : cfg.visuals.players.teamColorInv;
                            else  col = bVisible ? cfg.visuals.players.enemyColorVis : cfg.visuals.players.enemyColorInv;
                            switch (cfg.visuals.players.boxType)
                            {
                            case Config::EBox::E2DBoxes:
                            {
                                Drawing::Render2DBox(headPos, footPos, height, width, col);
                                break;
                            }
                            case Config::EBox::E3DBoxes:
                            {
                                FRotator rotation = actor->K2_GetActorRotation();
                                FVector ext = { 35.f, 35.f, extent.Z };
                                if (!Drawing::Render3DBox(localController, location, ext, rotation, col)) continue;
                                break;
                            }
                            }
                            if (cfg.visuals.players.bName)
                            {
                                auto const playerState = actor->PlayerState;
                                if (!playerState) continue;
                                const auto playerName = playerState->PlayerName;
                                if (!playerName.Data) continue;
                                char name[0x30];
                                const int len = playerName.multi(name, 0x20);
                                const int dist = localLoc.DistTo(origin) * 0.01f;
                                snprintf(name + len, sizeof(name) - len, " [%d]", dist);
                                const float adjust = height * 0.05f;
                                FVector2D pos = { headPos.X, headPos.Y - adjust };
                                Drawing::RenderText(name, pos, cfg.visuals.players.textCol);
                            }
                            if (cfg.visuals.players.barType != Config::EBar::ENone)
                            {
                                auto const healthComp = actor->HealthComponent;
                                if (!healthComp) continue;
                                const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
                                const float width2 = width * 0.5f;
                                const float adjust = height * 0.025f;
                                switch (cfg.visuals.players.barType)
                                {
                                case Config::EBar::ELeft:
                                {
                                    const float len = height * hp;
                                    drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    break;
                                }
                                case Config::EBar::ERight:
                                {
                                    const float len = height * hp;
                                    drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    break;
                                }
                                case Config::EBar::EBottom:
                                {
                                    const float len = width * hp;
                                    drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    break;
                                }
                                case Config::EBar::ETop:
                                {
                                    const float len = width * hp;
                                    drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                    drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                    break;
                                }
                                }
                            }
                        }   
                    }
                    if (cfg.misc.client.b_map_pins)
                    {
                        if (actor->isMapTable())
                        {
                            if (localCharacter->GetCurrentShip() == actor->GetParentActor())
                            {
                                auto maptable = reinterpret_cast<AMapTable*>(actor);
                                auto map_pins = maptable->MapPins;
                                for (int i = 0; i < map_pins.Count; i++)
                                {
                                    FVector2D current_map_pin = map_pins[i];
                                    current_map_pin *= 100.f;
                                    FVector current_map_pin_world;
                                    current_map_pin_world.X = current_map_pin.X;
                                    current_map_pin_world.Y = current_map_pin.Y;
                                    current_map_pin_world.Z = 0.f;
                                    FVector2D screen;
                                    if (localController->ProjectWorldLocationToScreen(current_map_pin_world, screen))
                                    {
                                        const int dist = localLoc.DistTo(current_map_pin_world) * 0.01f;
                                        char buf[0x64];
                                        snprintf(buf, sizeof(buf), "Map Pin [%dm]", dist);
                                        Drawing::RenderText(buf, { screen.X, screen.Y - 8 }, { 1.f,1.f,1.f,1.f }, true, true);
                                    }
                                }
                            }
                        }
                    }
                    if (cfg.aim.bEnable)
                    {
                        if (attachObject && attachObject->isCannon())
                        {
                            if (cfg.aim.cannon.deckshots && actor->isShip()) {
                                do
                                {
                                    FVector location = actor->K2_GetActorLocation();
                                    if (location.DistTo(cameraLoc) > 127500)
                                    {
                                        break;
                                    }
                                    auto const hulldamage = actor->GetHullDamage();
                                    if (!hulldamage) continue;
                                    const auto sinking = hulldamage->IsShipSinking();
                                    if (sinking == true) { break; }
                                    auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                    float gravity_scale = cannon->ProjectileGravityScale;
                                    const FVector forward = actor->GetActorForwardVector();
                                    const FVector up = actor->GetActorUpVector();
                                    const FVector loc = actor->K2_GetActorLocation();
                                    FVector loc_mast = loc;
                                    loc_mast += forward * 177.f;
                                    loc_mast += up * 342.f;
                                    location = loc_mast;
                                    gravity_scale = 1.30f;
                                    FRotator low, high;
                                    int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                    if (i_solutions < 1)
                                        break;
                                    low.Clamp();
                                    low -= attachObject->K2_GetActorRotation();
                                    low.Clamp();
                                    float absPitch = abs(low.Pitch);
                                    float absYaw = abs(low.Yaw);
                                    if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                    float sum = absYaw + absPitch;
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = location;
                                        aimBest.delta = low;
                                        aimBest.best = sum;
                                    }
                                } while (false);
                            }
                            if (cfg.aim.cannon.b_chain_shots == true && cfg.aim.cannon.deckshots == false && actor->isShip())
                            {
                                do
                                {
                                    if (actor == localCharacter->GetCurrentShip())
                                    {
                                        break;
                                    }
                                    FVector location = actor->K2_GetActorLocation();
                                    if (location.DistTo(cameraLoc) > 127500)
                                    {
                                        break;
                                    }
                                    int amount = 0;
                                    auto water = actor->GetInternalWater();
                                    amount = water->GetNormalizedWaterAmount() * 100.f;
                                    if (amount == 100) {
                                        break;
                                    }
                                    auto const hulldamage = actor->GetHullDamage();
                                    if (!hulldamage) continue;
                                    const auto sinking = hulldamage->IsShipSinking();
                                    if (sinking == true) { break; }
                                    auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                    float gravity_scale = cannon->ProjectileGravityScale;
                                    auto type = actor->GetName();
                                    char buf[0x64];
                                    if (type.find("BP_Small") != std::string::npos)
                                    {
                                        FVector location = actor->K2_GetActorLocation();
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward * 80.f;
                                        loc_mast += up * 1669.f;
                                        location = loc_mast;
                                        FRotator low, high;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                    else if (type.find("BP_Medium") != std::string::npos)
                                    {
                                        FVector location = actor->K2_GetActorLocation();
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward * -285.f;
                                        loc_mast += up * 1690.f;
                                        location = loc_mast;
                                        FRotator low, high;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                    else if (type.find("BP_Large") != std::string::npos)
                                    {
                                        FVector location = actor->K2_GetActorLocation();
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward * -112.f;
                                        loc_mast += up * 1600.f;
                                        location = loc_mast;
                                        FRotator low, high;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                } while (false);
                            }
                            if (cfg.aim.cannon.playeraimbot == true && cfg.aim.cannon.b_chain_shots == false && cfg.aim.cannon.deckshots == false && actor->isPlayer() && actor != localCharacter && !actor->IsDead())
                            {
                                do
                                {
                                    if (!cfg.aim.players.bTeam) if (UCrewFunctions::AreCharactersInSameCrew(actor, localCharacter)) break;
                                    FVector location = actor->K2_GetActorLocation();
                                    if (location.DistTo(cameraLoc) > 127500)
                                    {
                                    break;
                                    }
                                    auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                    float gravity_scale = cannon->ProjectileGravityScale;
                                    FRotator low, high;
                                    int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetForwardVelocity(), low, high);
                                    if (i_solutions < 1)
                                    break;
                                    low.Clamp();
                                    low -= attachObject->K2_GetActorRotation();
                                    low.Clamp();
                                    float absPitch = abs(low.Pitch);
                                    float absYaw = abs(low.Yaw);
                                    if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                    float sum = absYaw + absPitch;
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = location;
                                        aimBest.delta = low;
                                        aimBest.best = sum;                                       
                                    }
                                } while (false);
                            }
                            if (cfg.aim.cannon.skeletonaimbot && cfg.aim.cannon.b_chain_shots == false && cfg.aim.cannon.deckshots == false && actor->isSkeleton() && actor != localCharacter && !actor->IsDead() && !localController->LineOfSightTo(actor, cameraLoc, false))
                            {
                                do
                                {
                                    FVector location = actor->K2_GetActorLocation();
                                    if (location.DistTo(cameraLoc) > 127500)
                                    {
                                    break;
                                    }
                                    auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                    float gravity_scale = cannon->ProjectileGravityScale;
                                    FRotator low, high;
                                    int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetForwardVelocity(), low, high);
                                    if (i_solutions < 1)
                                    break;
                                    low.Clamp();
                                    low -= attachObject->K2_GetActorRotation();
                                    low.Clamp();
                                    float absPitch = abs(low.Pitch);
                                    float absYaw = abs(low.Yaw);
                                    if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                    float sum = absYaw + absPitch;  
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = location;
                                        aimBest.delta = low;
                                        aimBest.best = sum;                                       
                                    }
                                } while (false);
                            }
                            if (cfg.aim.cannon.b_chain_shots == false && cfg.aim.cannon.deckshots == false && actor->isShip())
                            {
                                do
                                {
                                    if (actor == localCharacter->GetCurrentShip())
                                    {
                                        break;
                                    }
                                    FVector location = actor->K2_GetActorLocation();
                                    if (location.DistTo(cameraLoc) > 127500)
                                    {
                                        break;
                                    }
                                    int amount = 0;
                                    auto water = actor->GetInternalWater();
                                    amount = water->GetNormalizedWaterAmount() * 100.f;
                                    if (amount == 100) {
                                        break;
                                    }
                                    auto const hulldamage = actor->GetHullDamage();
                                    if (!hulldamage) continue;
                                    const auto sinking = hulldamage->IsShipSinking();
                                    if (sinking == true) { break; }
                                    const FVector forward = actor->GetActorForwardVector();
                                    const FVector up = actor->GetActorUpVector();
                                    const FVector loc = actor->K2_GetActorLocation();
                                    FVector loc_mast = loc;
                                    loc_mast += forward * 30;
                                    loc_mast += up * 40.f;
                                    location = loc_mast;
                                    auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                    float gravity_scale = cannon->ProjectileGravityScale;
                                    auto type = actor->GetName();
                                    char buf[0x64];
                                    if (type.find("BP_Small") != std::string::npos)
                                    {
                                        FVector location = actor->K2_GetActorLocation();
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward * randomnumber;
                                        loc_mast += up * 40.f;
                                        location = loc_mast;
                                        if (cfg.aim.cannon.randomshots) {
                                            static std::uintptr_t desiredTime = 0;
                                            if (milliseconds_now() > desiredTime){
                                                int randomnumber = 0;
                                                fn();
                                                {
                                                    
                                                    desiredTime = milliseconds_now() + 2000;
                                                }
                                            }
                                            FVector location = actor->K2_GetActorLocation();
                                            const FVector forward = actor->GetActorForwardVector();
                                            const FVector up = actor->GetActorUpVector();
                                            const FVector loc = actor->K2_GetActorLocation();
                                            FVector loc_mast = loc;
                                            loc_mast += forward * randomnumber;
                                            loc_mast += up * 40.f;
                                            location = loc_mast;
                                        }
                                        FRotator low, high;
                                        auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                        float gravity_scale = cannon->ProjectileGravityScale;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                    else if (type.find("BP_Medium") != std::string::npos)
                                    {
                                        FVector location = actor->K2_GetActorLocation();
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward;
                                        loc_mast += up * 22.f;
                                        location = loc_mast;
                                        if (cfg.aim.cannon.randomshots)
                                        {
                                            static std::uintptr_t desiredTime = 0;
                                            if (milliseconds_now() > desiredTime) {
                                                int randomnumber = 0;
                                                fna();
                                                {

                                                    desiredTime = milliseconds_now() + 2000;
                                                }
                                            }
                                            const FVector forward = actor->GetActorForwardVector();
                                            const FVector up = actor->GetActorUpVector();
                                            const FVector loc = actor->K2_GetActorLocation();
                                            FVector loc_mast = loc;
                                            loc_mast += forward * randomnumber;
                                            loc_mast += up * 22.f;
                                            location = loc_mast;
                                        }
                                        FRotator low, high;
                                        auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                        float gravity_scale = cannon->ProjectileGravityScale;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                    else if (type.find("BP_Large") != std::string::npos)
                                    {
                                        if (location.DistTo(cameraLoc) > 127500)
                                        {
                                            break;
                                        }
                                        int amount = 0;
                                        auto water = actor->GetInternalWater();
                                        amount = water->GetNormalizedWaterAmount() * 100.f;
                                        if (amount == 100) {
                                            break;
                                        }
                                        FVector location = actor->K2_GetActorLocation();
                                        const FVector forward = actor->GetActorForwardVector();
                                        const FVector up = actor->GetActorUpVector();
                                        const FVector loc = actor->K2_GetActorLocation();
                                        FVector loc_mast = loc;
                                        loc_mast += forward;
                                        loc_mast += up * -410.f;
                                        location = loc_mast;
                                        if (cfg.aim.cannon.randomshots)
                                        {
                                            static std::uintptr_t desiredTime = 0;
                                            if (milliseconds_now() > desiredTime) {
                                                int randomnumber = 0;
                                                fnb();
                                                {
                                                    desiredTime = milliseconds_now() + 2000;
                                                }
                                            }
                                            const FVector forward = actor->GetActorForwardVector();
                                            const FVector up = actor->GetActorUpVector();
                                            const FVector loc = actor->K2_GetActorLocation();
                                            FVector loc_mast = loc;
                                            loc_mast += forward * randomnumber;
                                            loc_mast += up * -410.f;
                                            location = loc_mast;
                                        }
                                        FRotator low, high;
                                        auto cannon = reinterpret_cast<ACannon*>(attachObject);
                                        float gravity_scale = cannon->ProjectileGravityScale;
                                        int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLoc, attachObject->GetVelocity(), low, high);
                                        if (i_solutions < 1)
                                            break;
                                        low.Clamp();
                                        low -= attachObject->K2_GetActorRotation();
                                        low.Clamp();
                                        float absPitch = abs(low.Pitch);
                                        float absYaw = abs(low.Yaw);
                                        if (absPitch > cfg.aim.cannon.fPitch || absYaw > cfg.aim.cannon.fYaw) { break; }
                                        float sum = absYaw + absPitch;
                                        if (sum < aimBest.best)
                                        {
                                            aimBest.target = actor;
                                            aimBest.location = location;
                                            aimBest.delta = low;
                                            aimBest.best = sum;
                                        }
                                    }
                                } while (false);
                            }
                        }                        
                        else if (!attachObject && isWieldedWeapon)
                        {
                            if (cfg.aim.players.bEnable && actor->isPlayer() && actor != localCharacter && !actor->IsDead())
                            {
                                do
                                {
                                    FVector playerLoc = actor->K2_GetActorLocation();
                                    if (!actor->IsInWater() && localWeapon->WeaponParameters.NumberOfProjectiles == 1)
                                    {
                                        playerLoc.Z += cfg.aim.players.fAimHeight;
                                    }
                                    float dist = localLoc.DistTo(playerLoc);
                                    if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange) { break; }
                                    if (dist > 20000) { break; }
                                    if (cfg.aim.players.bVisibleOnly) if (!localController->LineOfSightTo(actor, cameraLoc, false)) { break; }
                                    if (!cfg.aim.players.bTeam) if (UCrewFunctions::AreCharactersInSameCrew(actor, localCharacter)) break;
                                    FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, playerLoc), cameraRot);
                                    float absYaw = abs(rotationDelta.Yaw);
                                    float absPitch = abs(rotationDelta.Pitch);
                                    if (absYaw > cfg.aim.players.fYaw || absPitch > cfg.aim.players.fPitch) { break; }
                                    float sum = absYaw + absPitch;
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = playerLoc;
                                        aimBest.delta = rotationDelta;
                                        aimBest.best = sum;
                                        aimBest.smoothness = cfg.aim.players.fSmoothness;
                                    }

                                } while (false);
                            
                            }
                            else if (cfg.aim.skeletons.bEnable && actor->isSkeleton() && !actor->IsDead())
                            {
                                do
                                {
                                    FVector playerLoc = actor->K2_GetActorLocation();
                                    if (localWeapon->WeaponParameters.NumberOfProjectiles == 1)
                                    playerLoc.Z += cfg.aim.skeletons.fAimHeight;
                                    const float dist = localLoc.DistTo(playerLoc);
                                    if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange) { break; }
                                    if (dist > 20000) { break; }
                                    if (cfg.aim.skeletons.bVisibleOnly) if (!localController->LineOfSightTo(actor, cameraLoc, false)) break;
                                    const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, playerLoc), cameraRot);
                                    const float absYaw = abs(rotationDelta.Yaw);
                                    const float absPitch = abs(rotationDelta.Pitch);
                                    if (absYaw > cfg.aim.skeletons.fYaw || absPitch > cfg.aim.skeletons.fPitch) break;
                                    const float sum = absYaw + absPitch;
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = playerLoc;
                                        aimBest.delta = rotationDelta;
                                        aimBest.best = sum;
                                        aimBest.smoothness = cfg.aim.skeletons.fSmoothness;
                                    }

                                } while (false);
                            }
                        }
                    }
                }
            }
            if (aimBest.target != nullptr)
            {
                FVector2D screen;
                if (localController->ProjectWorldLocationToScreen(aimBest.location, screen))
                {
                    auto col = ImGui::GetColorU32(IM_COL32(0, 200, 0, 255));
                    drawList->AddLine({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f }, { screen.X, screen.Y }, col);
                    drawList->AddCircle({ screen.X, screen.Y }, 3.f, col);
                }
                if (ImGui::IsMouseDown(1))
                {
                    if (attachObject && attachObject->isCannon())
                    {                       
                        auto cannon = reinterpret_cast<ACannon*>(attachObject);                       
                        if (cannon)
                        {
                            if (((aimBest.delta.Pitch > cannon->PitchRange.max) || (aimBest.delta.Pitch < cannon->PitchRange.min)) || ((aimBest.delta.Yaw > cannon->YawRange.max) || (aimBest.delta.Yaw < cannon->YawRange.min)))
                            {
                                std::string str_text_message = "TARGET IS OUT OF RANGE!";
                                drawList->AddText({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f + 30.f }, 0xFFFFFFFF, str_text_message.c_str());
                            }
                            else
                            {
                                cannon->ForceAimCannon(aimBest.delta.Pitch, aimBest.delta.Yaw); 
                            }
                        }
                    }
                    else
                    {
                        FVector LV = localCharacter->GetVelocity();
                        if (auto const localShip = localCharacter->GetCurrentShip()) LV += localShip->GetVelocity();
                        FVector TV = aimBest.target->GetVelocity();
                        if (auto const targetShip = aimBest.target->GetCurrentShip()) TV += targetShip->GetVelocity();
                        const FVector RV = TV - LV;
                        const float BS = localWeapon->WeaponParameters.AmmoParams.Velocity;
                        const FVector RL = localLoc - aimBest.location;
                        const float a = RV.Size() - BS * BS;
                        const float b = (RL * RV * 2.f).Sum();
                        const float c = RL.SizeSquared();
                        const float D = b*b - 4 * a * c;
                        if (D > 0)
                        {
                            const float DRoot = sqrtf(D);
                            const float x1 = (-b + DRoot) / (2 * a);
                            const float x2 = (-b - DRoot) / (2 * a);
                            if (x1 >= 0 && x1 >= x2) aimBest.location += RV * x1;
                            else if (x2 >= 0) aimBest.location += RV * x2;
                            aimBest.delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, aimBest.location), cameraRot);
                            auto smoothness = 1.f / aimBest.smoothness;
                            localController->AddYawInput(aimBest.delta.Yaw * smoothness);
                            localController->AddPitchInput(aimBest.delta.Pitch * -smoothness);
                        }
                    }
                }
            }
        }while (false);
    }
    catch (...) 
    {
        Logger::Log("Exception\n");
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
#ifdef STEAM
    if (ImGui::IsKeyPressed(VK_INSERT)) bIsOpen = !bIsOpen;
#else
    static const FKey insert("Insert");
    if (cache.localController && cache.localController->WasInputKeyJustPressed(insert)) { bIsOpen = !bIsOpen; }
#endif
    if (bIsOpen) {
        ImGui::SetNextWindowSize(ImVec2(1000, 825), ImGuiCond_Once);
        ImGui::Begin("Trash Paste", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGuiStyle* style = &ImGui::GetStyle();
        style->WindowPadding = ImVec2(8, 8);
        style->WindowRounding = 7.0f;
        style->FramePadding = ImVec2(4, 3);
        style->FrameRounding = 0.0f;
        style->ItemSpacing = ImVec2(6, 4);
        style->ItemInnerSpacing = ImVec2(4, 4);
        style->IndentSpacing = 20.0f;
        style->ScrollbarSize = 14.0f;
        style->ScrollbarRounding = 9.0f;
        style->GrabMinSize = 5.0f;
        style->GrabRounding = 0.0f;
        style->WindowBorderSize = 0;
        style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style->FramePadding = ImVec2(4, 3);
        style->Colors[ImGuiCol_TitleBg] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_TitleBgActive] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_Button] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_ButtonActive] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_ButtonHovered] = ImColor(41, 40, 41, 255);
        style->Colors[ImGuiCol_Separator] = ImColor(70, 70, 70, 255);
        style->Colors[ImGuiCol_SeparatorActive] = ImColor(76, 76, 76, 255);
        style->Colors[ImGuiCol_SeparatorHovered] = ImColor(76, 76, 76, 255);
        style->Colors[ImGuiCol_Tab] = ImColor(1, 32, 230, 225);
        style->Colors[ImGuiCol_TabHovered] = ImColor(1, 32, 238, 225);
        style->Colors[ImGuiCol_TabActive] = ImColor(1, 32, 238, 225);
        style->Colors[ImGuiCol_SliderGrab] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_SliderGrabActive] = ImColor(1, 32, 255, 225);
        style->Colors[ImGuiCol_MenuBarBg] = ImColor(76, 76, 76, 255);
        style->Colors[ImGuiCol_FrameBg] = ImColor(37, 36, 37, 255);
        style->Colors[ImGuiCol_FrameBgActive] = ImColor(37, 36, 37, 255);
        style->Colors[ImGuiCol_FrameBgHovered] = ImColor(37, 36, 37, 255);
        style->Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
        style->Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 0);
        style->Colors[ImGuiCol_HeaderHovered] = ImColor(46, 46, 46, 255);

        if (ImGui::BeginTabBar("Bars")) {
            if (ImGui::BeginTabItem(ICON_FA_EYE " Visuals")) 
            {
                ImGui::Text("Global Visuals");
                if (ImGui::BeginChild("Global", ImVec2(170.f, 50.f), false, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.bEnable);
                    ImGui::Checkbox("Text Outlines", &cfg.visuals.textoutlines);
                }
                ImGui::EndChild();

                ImGui::Columns(3, "CLM1", false);
                const char* boxes[] = { "None", "2DBox", "3DBox" };
                
                ImGui::Text("Players");
                if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    const char* bars[] = { "None", "2DRectLeft", "2DRectRight", "2DRectBottom", "2DRectTop" };
                    ImGui::Checkbox("Enable", &cfg.visuals.players.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.players.bName);
                    ImGui::Combo("Box Type", reinterpret_cast<int*>(&cfg.visuals.players.boxType), boxes, IM_ARRAYSIZE(boxes));
                    ImGui::ColorEdit4("Visible Enemy Color", &cfg.visuals.players.enemyColorVis.x, 0);
                    ImGui::ColorEdit4("Invisible Enemy Color", &cfg.visuals.players.enemyColorInv.x, 0);
                    ImGui::ColorEdit4("Visible Team Color", &cfg.visuals.players.teamColorVis.x, 0);
                    ImGui::ColorEdit4("Invisible Team Color", &cfg.visuals.players.teamColorInv.x, 0);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.players.textCol.x, 0);
                    ImGui::SliderFloat("Draw Distance", &cfg.visuals.players.drawdistance, 0.f, 5000.f, "%.0f");
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Skeletons");
                if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.skeletons.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.skeletons.bName);
                    ImGui::Combo("Box Type", reinterpret_cast<int*>(&cfg.visuals.skeletons.boxType), boxes, IM_ARRAYSIZE(boxes));
                    ImGui::ColorEdit4("Visible Color", &cfg.visuals.skeletons.colorVis.x, 0);
                    ImGui::ColorEdit4("Invisible Color", &cfg.visuals.skeletons.colorInv.x, 0);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.skeletons.textCol.x, 0);
                    ImGui::SliderFloat("Draw Distance", &cfg.visuals.skeletons.drawdistance, 0.f, 5000.f, "%.0f");

                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Islands");
                if (ImGui::BeginChild("IslandsSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.islands.bEnable);
                    ImGui::Checkbox("Draw Names", &cfg.visuals.islands.bName);
                    ImGui::SliderInt("Max Distance", &cfg.visuals.islands.intMaxDist, 100, 2750, "%d", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.islands.textCol.x, 0);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Ships");
                if (ImGui::BeginChild("ShipsSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse)) {

                    ImGui::Checkbox("Enable", &cfg.visuals.ships.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.ships.bName);
                    ImGui::Checkbox("Show Holes", &cfg.visuals.ships.bDamage);
                    ImGui::ColorEdit4("Damage Color", &cfg.visuals.ships.damageColor.x, 0);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.ships.textCol.x, 0);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Items");
                if (ImGui::BeginChild("ItemsSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.items.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.items.bName);
                    ImGui::Checkbox("Barrels", &cfg.visuals.items.barrelitems);
                    ImGui::Checkbox("Ammo Chests", &cfg.visuals.items.ammochests);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.items.textCol.x, 0);
                    ImGui::ColorEdit4("Barrel Text Color", &cfg.visuals.items.barreltextCol.x, 0);
                    ImGui::ColorEdit4("Ammo Chest Text Color", &cfg.visuals.items.ammotextCol.x, 0);
                    ImGui::SliderFloat("Item Draw Distance", &cfg.visuals.items.itemdrawdistance, 0.f, 5000.f, "%.0f");
                    ImGui::SliderFloat("Barrel Draw Distance", &cfg.visuals.items.barreldrawdistance, 0.f, 5000.f, "%.0f");
                    ImGui::SliderFloat("Ammochest Draw Distance", &cfg.visuals.items.ammochestsdrawdistance, 0.f, 5000.f, "%.0f");
                }
                ImGui::EndChild();;

                ImGui::NextColumn();

                ImGui::Text("Client");
                if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.client.bEnable);
                    ImGui::Checkbox("Crosshair", &cfg.visuals.client.bCrosshair);
                    if (cfg.visuals.client.bCrosshair)
                    {
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(75.f);
                        ImGui::SliderFloat("Radius##1", &cfg.visuals.client.fCrosshair, 1.f, 100.f);
                    }
                    ImGui::ColorEdit4("Crosshair Color", &cfg.visuals.client.crosshairColor.x, ImGuiColorEditFlags_DisplayRGB);

                    ImGui::Checkbox("Oxygen Level", &cfg.visuals.client.bOxygen);
                    ImGui::Checkbox("Compass", &cfg.visuals.client.bCompass);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("World Events");
                if (ImGui::BeginChild("WorldSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.world.bEnable);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.world.textCol.x, 0);
                    ImGui::SliderFloat("Draw Distance", &cfg.visuals.world.drawdistance, 0.f, 20000.f, "%.0f");
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Rowboats");
                if (ImGui::BeginChild("RowboatsSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.rowboats.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.rowboats.bName);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.rowboats.textCol.x, 0);
                    ImGui::SliderFloat("Draw Distance", &cfg.visuals.rowboats.drawdistance, 0.f, 5000.f, "%.0f");
                }
                ImGui::EndChild();
                ImGui::NextColumn();

                ImGui::Text("Shipwrecks");
                if (ImGui::BeginChild("ShipwrecksSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.shipwrecks.bEnable);
                    ImGui::Checkbox("Draw Name", &cfg.visuals.shipwrecks.bName);
                    ImGui::ColorEdit4("Text Color", &cfg.visuals.shipwrecks.textCol.x, 0);
                    ImGui::SliderFloat("Draw Distance", &cfg.visuals.shipwrecks.drawdistance, 0.f, 5000.f, "%.0f");
                }
                ImGui::EndChild();
                ImGui::Columns();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(ICON_FA_CROSSHAIRS " Aim")) {

                ImGui::Text("Global Aim");
                if (ImGui::BeginChild("Global", ImVec2(0.f, 29.f), false, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.bEnable);
                }
                ImGui::EndChild();
                
                ImGui::Columns(3, "CLM1", false);
                ImGui::Text("Players");
                if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.players.bEnable);
                    ImGui::Checkbox("Aim At Teammates", &cfg.aim.players.bTeam);
                    ImGui::Checkbox("Visible Only", &cfg.aim.players.bVisibleOnly);
                    ImGui::SliderFloat("Yaw", &cfg.aim.players.fYaw, 1.f, 200.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.players.fPitch, 1.f, 200.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Smoothness", &cfg.aim.players.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Aim Height", &cfg.aim.players.fAimHeight, 0.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                }
                ImGui::EndChild();

                ImGui::NextColumn();
               
                ImGui::Text("Cannon");
                if (ImGui::BeginChild("CannonSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.cannon.bEnable);
                    ImGui::Checkbox("Random Shots", &cfg.aim.cannon.randomshots);
                    ImGui::Checkbox("Player Aimbot", &cfg.aim.cannon.playeraimbot);
                    ImGui::Checkbox("Skeleton Aimbot", &cfg.aim.cannon.skeletonaimbot); 
                    ImGui::Checkbox("Chain Aimbot", &cfg.aim.cannon.b_chain_shots);
                    ImGui::SliderFloat("Yaw", &cfg.aim.cannon.fYaw, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.cannon.fPitch, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);                    
                }
                ImGui::EndChild();                

                ImGui::NextColumn();
                
                ImGui::Text("Skeletons");
                if (ImGui::BeginChild("Skeleton Settings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.skeletons.bEnable);                    
                    ImGui::Checkbox("Visible Only", &cfg.aim.skeletons.bVisibleOnly);
                    ImGui::SliderFloat("Yaw", &cfg.aim.skeletons.fYaw, 1.f, 200.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.skeletons.fPitch, 1.f, 200.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Smoothness", &cfg.aim.skeletons.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Aim Height", &cfg.aim.skeletons.fAimHeight, 0.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                }
                ImGui::EndChild();

                ImGui::Columns();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(ICON_FA_SLIDERS_H " Misc")) {

                ImGui::Text("Global Misc");
                if (ImGui::BeginChild("Global", ImVec2(0.f, 29.f), false, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.misc.bEnable);
                }
                ImGui::EndChild();

                ImGui::Columns(3, "CLM1", false);
                ImGui::Text("Client");
                if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.misc.client.bEnable);
                    ImGui::Checkbox("Ship Info", &cfg.misc.client.bShipInfo);
                    ImGui::Checkbox("Map Pins", &cfg.misc.client.b_map_pins);
                    ImGui::SliderFloat("FOV", &cfg.misc.client.fov, 90.f, 180.f, "%.0f");
                    ImGui::Separator();
                    if (ImGui::Button("Save Settings"))
                    {
                        do {
                            wchar_t buf[MAX_PATH];
                            GetModuleFileNameW(hinstDLL, buf, MAX_PATH);
                            fs::path path = fs::path(buf).remove_filename() / ".settings";
                            auto file = CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (file == INVALID_HANDLE_VALUE) break;
                            DWORD written;
                            if (WriteFile(file, &cfg, sizeof(cfg), &written, 0)) ImGui::OpenPopup("##SettingsSaved");
                            CloseHandle(file);
                        } while (false);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Load Settings")) 
                    {
                        do {
                            wchar_t buf[MAX_PATH];
                            GetModuleFileNameW(hinstDLL, buf, MAX_PATH);
                            fs::path path = fs::path(buf).remove_filename() / ".settings";
                            auto file = CreateFileW(path.wstring().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (file == INVALID_HANDLE_VALUE) break;
                            DWORD readed;
                            if (ReadFile(file, &cfg, sizeof(cfg), &readed, 0))  ImGui::OpenPopup("##SettingsLoaded");
                            CloseHandle(file);
                        } while (false);
                    }
                    ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    if (ImGui::BeginPopupModal("##SettingsSaved", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
                    {
                        ImGui::Text("\nSettings Saved!\n\n");
                        ImGui::Separator();
                        if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
                        ImGui::EndPopup();
                    }
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                    if (ImGui::BeginPopupModal("##SettingsLoaded", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
                    {
                        ImGui::Text("\nSettings Loaded!\n\n");
                        ImGui::Separator();
                        if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
                        ImGui::EndPopup();
                    }
                }
                ImGui::EndChild();
                 
                ImGui::NextColumn();

                ImGui::Text("Macros");
                if (ImGui::BeginChild("MacrosSettings", ImVec2(0.f, 200.f), true, 0 | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImGui::Checkbox("Enable", &cfg.misc.macro.bEnable);
                    ImGui::Checkbox("Bunny Hop", &cfg.misc.macro.b_bunnyhop);
                }
                ImGui::EndChild();
                ImGui::Columns();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        };
        ImGui::End();
    }
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return PresentOriginal(swapChain, syncInterval, flags);
}

HRESULT Cheat::Renderer::ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{
    if (renderTargetView)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    if (context)
    {
        context->Release();
        context = nullptr;
    }
    if (device)
    {
        device->Release();
        device = nullptr;
    }
    return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

inline bool Cheat::Renderer::Init()
{
    HMODULE dxgi = GetModuleHandleA("dxgi.dll");
    Logger::Log("dxgi: %p\n", dxgi);
    static BYTE PresentSig[] = { 0x55, 0x57, 0x41, 0x56, 0x48, 0x8d, 0x6c, 0x24, 0x90, 0x48, 0x81, 0xec, 0x70, 0x01 };
    fnPresent = reinterpret_cast<decltype(fnPresent)>(Tools::FindFn(dxgi, PresentSig, sizeof(PresentSig)));
    Logger::Log("IDXGISwapChain::Present: %p\n", fnPresent);
    if (!fnPresent) return false;
    static BYTE ResizeSig[] = { 0x48, 0x81, 0xec, 0xc0, 0x00, 0x00, 0x00, 0x48, 0xc7, 0x45, 0x1f };
    fnResize = reinterpret_cast<decltype(fnResize)>(Tools::FindFn(dxgi, ResizeSig, sizeof(ResizeSig)));
    Logger::Log("IDXGISwapChain::ResizeBuffers: %p\n", fnResize);
    if (!fnResize) return false;
    if (!SetHook(fnPresent, PresentHook, reinterpret_cast<void**>(&PresentOriginal)))
    {
        return false;
    };
    Logger::Log("PresentHook: %p\n", PresentHook);
    Logger::Log("PresentOriginal: %p\n", PresentOriginal);
    if (!SetHook(fnResize, ResizeHook, reinterpret_cast<void**>(&ResizeOriginal)))
    {
        return false;
    };
    Logger::Log("ResizeHook: %p\n", ResizeHook);
    Logger::Log("ResizeOriginal: %p\n", ResizeOriginal);
    if (!SetHook(SetCursorPos, SetCursorPosHook, reinterpret_cast<void**>(&SetCursorPosOriginal)))
    {
        Logger::Log("Can't hook SetCursorPos\n");
        return false;
    };
    if (!SetHook(SetCursor, SetCursorHook, reinterpret_cast<void**>(&SetCursorOriginal)))
    {
        Logger::Log("Can't hook SetCursor\n");
        return false;
    };
    return true;
}

inline bool Cheat::Renderer::Remove()
{
    Renderer::RemoveInput(); 
    if (!RemoveHook(PresentOriginal) || !RemoveHook(ResizeOriginal) || !RemoveHook(SetCursorPosOriginal) || !RemoveHook(SetCursorOriginal))
    {
        return false;
    }
    if (renderTargetView)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    if (context)
    {
        context->Release();
        context = nullptr;
    }
    if (device)
    {
        device->Release();
        device = nullptr;
    }
    return true;
}

inline bool Cheat::Tools::CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size)
{
    for (SIZE_T i = 0; i < size; i++) {
        if (data[i] != sig[i]) {
            if (sig[i] == 0x00) continue;
            return false;
        }
    }
    return true;
}

inline BYTE* Cheat::Tools::FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size)
{
    for (BYTE* it = start; it < end - size; it++) {
        if (CompareByteArray(it, sig, size)) {
            return it;
        };
    }
    return 0;
}

void* Cheat::Tools::FindPointer(BYTE* sig, SIZE_T size, int addition = 0)
{
    auto base = static_cast<BYTE*>(gBaseMod.lpBaseOfDll);
    auto address = FindSignature(base, base + gBaseMod.SizeOfImage - 1, sig, size);
    if (!address) return nullptr;
    auto k = 0;
    for (; sig[k]; k++);
    auto offset = *reinterpret_cast<UINT32*>(address + k);
    return address + k + 4 + offset + addition;
}

inline BYTE* Cheat::Tools::FindFn(HMODULE mod, BYTE* sig, SIZE_T sigSize)
{
    if (!mod || !sig || !sigSize) return 0;
    MODULEINFO modInfo;
    if (!K32GetModuleInformation(GetCurrentProcess(), mod, &modInfo, sizeof(MODULEINFO))) return 0;
    auto base = static_cast<BYTE*>(modInfo.lpBaseOfDll);
    auto fn = Tools::FindSignature(base, base + modInfo.SizeOfImage - 1, sig, sigSize);
    if (!fn) return 0;
    for (; *fn != 0xCC && *fn != 0xC3; fn--);
    fn++;
    return fn;
}

inline bool Cheat::Tools::PatchMem(void* address, void* bytes, SIZE_T size)
{
    DWORD oldProtection;
    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtection))
    {
        memcpy(address, bytes, size);
        return VirtualProtect(address, size, oldProtection, &oldProtection);
    };
    return false;
}

inline BYTE* Cheat::Tools::PacthFn(HMODULE mod, BYTE* sig, SIZE_T sigSize, BYTE* bytes, SIZE_T bytesSize)
{
    if (!mod || !sig || !sigSize || !bytes || !bytesSize) return 0;
    auto fn = FindFn(mod, sig, sigSize);
    if (!fn) return 0;
    return Tools::PatchMem(fn, bytes, bytesSize) ? fn : 0;
}

inline bool Cheat::Tools::FindNameArray()
{
    static BYTE sig[] = { 0x48, 0x8b, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xff, 0x75, 0x3c };
    auto address = reinterpret_cast<decltype(FName::GNames)*>(FindPointer(sig, sizeof(sig)));
    if (!address) return 0;
    Logger::Log("%p\n", address);
    FName::GNames = *address;
    return FName::GNames;
}

inline bool Cheat::Tools::FindObjectsArray()
{
    static BYTE sig[] = { 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDF, 0x48, 0x89, 0x5C, 0x24 };
    UObject::GObjects = reinterpret_cast<decltype(UObject::GObjects)>(FindPointer(sig, sizeof(sig), 16));
    return UObject::GObjects;
}

inline bool Cheat::Tools::FindWorld()
{
    static BYTE sig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC9, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x70 };
    UWorld::GWorld = reinterpret_cast<decltype(UWorld::GWorld)>(FindPointer(sig, sizeof(sig)));
    return UWorld::GWorld;
}

inline bool Cheat::Tools::InitSDK()
{
    if (!UCrewFunctions::Init()) return false;
    if (!UKismetMathLibrary::Init()) return false;
    return true;
}

inline bool Cheat::Logger::Init()
{
    fs::path log;
#ifdef STEAM
    wchar_t buf[MAX_PATH];
    if (!GetModuleFileNameW(hinstDLL, buf, MAX_PATH)) return false;
    log = fs::path(buf).remove_filename() / "log.txt";
#else
#ifdef UWPDEBUG
    log = "C:\\Users\\Maxi\\AppData\\Local\\Packages\\Microsoft.SeaofThieves_8wekyb3d8bbwe\\TempState\\log.txt";
#else
    return true;
#endif
#endif
    file = CreateFileW(log.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    return file != INVALID_HANDLE_VALUE;
}

inline bool Cheat::Logger::Remove()
{
    if (!file) return true;
    return CloseHandle(file);
}

void Cheat::Logger::Log(const char* format, ...)
{
#if defined STEAM || defined UWPDEBUG
    SYSTEMTIME rawtime;
    GetSystemTime(&rawtime);
    char buf[MAX_PATH];
    auto size = GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, 0, &rawtime, "[HH':'mm':'ss] ", buf, MAX_PATH) - 1;
    size += snprintf(buf + size, sizeof(buf) - size, "[TID: 0x%X] ", GetCurrentThreadId());
    va_list argptr;
    va_start(argptr, format);
    size += vsnprintf(buf + size, sizeof(buf) - size, format, argptr);
#if defined LOGFILE
    WriteFile(file, buf, size, NULL, NULL);
#endif
    printf("%s", buf);
    va_end(argptr);
#endif
}

bool Cheat::Init(HINSTANCE _hinstDLL)
{
    hinstDLL = _hinstDLL;
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    if (!Logger::Init())
    {
        return false;
    };
    if (!K32GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &gBaseMod, sizeof(MODULEINFO))) 
    {
        return false;
    };
    Logger::Log("Base address: %p\n", gBaseMod.lpBaseOfDll);
    if (!Tools::FindNameArray()) 
    {
        Logger::Log("Can't find NameArray!\n");
        return false;
    }
    Logger::Log("NameArray: %p\n", FName::GNames);
    if (!Tools::FindObjectsArray()) 
    {
        Logger::Log("Can't find ObjectsArray!\n");
        return false;
    } 
    Logger::Log("ObjectsArray: %p\n", UObject::GObjects);
    if (!Tools::FindWorld())
    {
        Logger::Log("Can't find World!\n");
        return false;
    }
    Logger::Log("World: %p\n", UWorld::GWorld);
    if (!Tools::InitSDK())
    {
        Logger::Log("Can't find important objects!\n");
        return false;
    };
    if (!Renderer::Init())
    {
        Logger::Log("Can't initialize renderer\n");
        return false;
    }
    Hacks::Init();
#ifdef STEAM
    auto t = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ClearingThread), nullptr, 0, nullptr);
    if (t) CloseHandle(t);
#endif
    return true;
}

void Cheat::ClearingThread()
{
    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            FreeLibraryAndExitThread(hinstDLL, 0);
        }
        Sleep(20);
    }
}

void Cheat::Tests()
{

}

bool Cheat::Remove()
{
    Logger::Log("Removing cheat...\n");
    if (!Renderer::Remove() || !Logger::Remove())
    {
        return false;
    };
    Hacks::Remove();
    FreeConsole();
    return true;
}