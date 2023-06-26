#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include "ImGui/imconfig.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "Kiero/kiero.h"
#include <string>
typedef HRESULT(_stdcall* Present)(IDXGISwapChain* pSwapChain, UINT syncinterval, UINT Flags);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Present pPresent;
ID3D11Device* pDevice = NULL;
ID3D11RenderTargetView* MainRenderTargetView;
ID3D11DeviceContext* pContext = NULL;
HWND hwnd = NULL;
WNDPROC oWndProc;

//窗口回调
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}
	return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

//hook
bool init_hook = false;
HRESULT _stdcall hkEndScene(IDXGISwapChain* pSwapChain, UINT Syncinterval, UINT Flags) {
	if (!init_hook) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC st;
			pSwapChain->GetDesc(&st);
			hwnd = st.OutputWindow;
			ID3D11Texture2D* Buffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&Buffer);
			if (Buffer == 0) {
				MessageBox(NULL, "It Fucked.", "Warning", MB_OK);
				return pPresent(pSwapChain, Syncinterval, Flags);//dont care
			}
			pDevice->CreateRenderTargetView(Buffer, NULL, &MainRenderTargetView);
			Buffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			// Init ImGui 当然如果说有别的东西要初始化，也可以放在这里
			ImGui::CreateContext();
			ImGui_ImplWin32_Init(hwnd);
			ImGui_ImplDX11_Init(pDevice, pContext);

			init_hook = true;
		}
		else {
			return pPresent(pSwapChain, Syncinterval, Flags);
		}
	}
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	/*开始绘制*/
	ImGui::Begin("There is a Example");
	ImGui::End();
	/*绘制结束*/
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &MainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return pPresent(pSwapChain, Syncinterval, Flags);
}

DWORD WINAPI Thread(LPVOID lpParams) {
	bool init = false;
	do {
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
			kiero::bind(8, (void**)&pPresent, hkEndScene);
			init = true;
		}
	} while (!init);
	return 0L;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, Thread, NULL, 0, NULL);
	}
	return TRUE;
}