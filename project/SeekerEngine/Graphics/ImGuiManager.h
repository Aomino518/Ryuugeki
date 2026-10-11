#pragma once
#include <stdlib.h>
#include <string>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif

class Application;
class Graphics;
class CameraManager;
class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	void Init();
	void BeginFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void DrawSoundWindow();
	void DrawCameraWindow(CameraManager* cameraManager);
	void DrawMainMenuBar();
	void DrawLoggerWindow();
	void DrawSceneWindow(uint32_t srvIndex, uint32_t textureWidth, uint32_t textureHeight);

	void DrawEditor();

	void Stats();
	void ShowMemoryUsage();
	void BeginInspector();
	void EndInspector();
	void SaveScenesJson();
	void LoadScenesJson();
	void ClearScenesJson();
	void DrawConfirmPopup();

	nlohmann::json SaveEditorJson() const;
	void LoadEditorJson(const nlohmann::json& j);
	
private:

	// メンバ関数
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void ApplyStyle();
	void DrawDockSpace();
	void DrawCameraFileControls(CameraManager* cameraManager);
	void RefreshCameraFiles();
	bool SaveCameraToPath(CameraManager* cameraManager, const std::filesystem::path& path);

	struct CameraFileUiState {
		std::string sceneName;
		std::filesystem::path currentPath;
		std::filesystem::path pendingSavePath;
		char fileName[128] = "";
		std::vector<std::filesystem::path> files;
		int selectedFile = -1;
		bool confirmOverwrite = false;
		std::string message;
	};
	CameraFileUiState cameraFileUi_;

	// メンバ変数
	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;

	// 表示するWindowのフラグ
	struct WindowFlags {
		bool showStats = true;
		bool showCamera = true;
		bool showSound = true;
		bool showConsole = true;
	};

	WindowFlags windowState_;
	bool requestSavePopup_ = false;
	bool requestLoadPopup_ = false;
	bool requestClearPopup_ = false;
};

