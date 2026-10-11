#include "ImGuiManager.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include <Psapi.h>
#include <numbers>
#include <algorithm>
#include <cstdio>
#include <system_error>
#include "Application.h"
#include "Graphics.h"
#include "SoundManager.h"
#include "LightManager.h"
#include "EmitterManager.h"
#include "SceneManager.h"
#include "ParticleManager.h"
#include "Particle2DManager.h"
#include "Emitter2DManager.h"
#include "SrvManager.h"
#include "Editor.h"
#include "Vector3.h"
#include <nlohmann/json.hpp>

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Init()
{
#ifdef USE_IMGUI
	app_ = Application::GetInstance();
	graphics_ = Graphics::GetInstance();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.Fonts->AddFontFromFileTTF(
		"resources/fonts/NotoSansJP-Regular.ttf",
		18.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(app_->GetHWND());
	ImGui_ImplDX12_Init(graphics_->GetDevice(),
		graphics_->GetSwapChainDesc().BufferCount,
		graphics_->GetRTVDesc().Format,
		SrvManager::GetInstance()->GetDiscriptorHeap(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);
	Logger::Write("ImGui初期化");

	// スタイルを設定
	ApplyStyle();
#endif
}

void ImGuiManager::BeginFrame()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	DrawDockSpace();
#endif
}

void ImGuiManager::EndFrame()
{
#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
#endif
}

void ImGuiManager::Draw()
{
#ifdef USE_IMGUI
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), graphics_->GetCmdList());
#endif
}

void ImGuiManager::Shutdown()
{
#ifdef USE_IMGUI
	// ImGuiの終了処理。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	Logger::Write("ImGuiManager Shutdown");
#endif
}

void ImGuiManager::Stats()
{
#ifdef USE_IMGUI
	if (windowState_.showStats) {
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::SeparatorText("Frame");
		float fps = ImGui::GetIO().Framerate;
		ImGui::Text("FPS: %.2f", fps);
		ImGui::Text("Frame Time: %.2f ms", 1000.0f / fps);

		ImGui::SeparatorText("Rendering");
		ImGui::Text("DrawCalls: %u", Graphics::GetInstance()->GetDrawCallCount());

		ImGui::SeparatorText("Particle");
		ImGui::Text("Particles: %u", ParticleManager::GetInstance()->GetTotalParticleCount());
		ImGui::Text("Particle Groups: %u", ParticleManager::GetInstance()->GetParticleGroupCount());
		ImGui::Text("Emitters: %u", EmitterManager::GetInstance()->GetEmitterCount());

		ImGui::SeparatorText("Particle2D");
		ImGui::Text("Particles: %u", Particle2DManager::GetInstance()->GetTotalParticleCount());
		ImGui::Text("Particle Groups: %u", Particle2DManager::GetInstance()->GetParticleGroupCount());
		ImGui::Text("Emitters: %u", Emitter2DManager::GetInstance()->GetEmitterCount());

		ImGui::SeparatorText("Memory");
		ShowMemoryUsage();
		ImGui::End();
	}
#endif
}

void ImGuiManager::ShowMemoryUsage() {
#ifdef USE_IMGUI
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		// メモリ使用量をMB単位で計算
		double memoryUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);

		ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
	}
#endif
}

void ImGuiManager::BeginInspector()
{
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");
#endif
}

void ImGuiManager::EndInspector()
{
#ifdef USE_IMGUI
	ImGui::End();
#endif
}

namespace
{
    const std::filesystem::path kCameraDirectory = "resources/json/cameras";

    std::string CameraPathText(const std::filesystem::path& path)
    {
		// utf-8として取り出す
        const auto utf8 = path.generic_u8string();
        return std::string(utf8.begin(), utf8.end());
    }

    std::string LowerCameraFileName(std::string text)
    {
        for (char& c : text) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        return text;
    }

#ifdef USE_IMGUI
    bool MakeCameraSavePath(const char* input, std::filesystem::path& result, std::string& error)
    {
        const std::string name(input);
        if (name.empty() || name.back() == ' ' || name.back() == '.') {
            error = "Enter a file name without a trailing space or dot.";
            return false;
        }

        if (name.find_first_of("<>:\"/\\|?*") != std::string::npos ||
            std::any_of(name.begin(), name.end(), [](unsigned char c) { return c < 32; })) {
            error = "Use a file name only, without path separators or reserved characters.";
            return false;
        }

        const std::string base = LowerCameraFileName(name.substr(0, name.find('.')));
        if (base == "con" || base == "prn" || base == "aux" || base == "nul" ||
            (base.size() == 4 && (base.substr(0, 3) == "com" || base.substr(0, 3) == "lpt") &&
                base[3] >= '1' && base[3] <= '9')) {
            error = "This file name is reserved by Windows.";
            return false;
        }

        auto fileName = std::filesystem::path(std::u8string(name.begin(), name.end()));
        const std::string extension = LowerCameraFileName(CameraPathText(fileName.extension()));
        if (extension.empty()) {
            fileName += ".json";
        } else if (extension != ".json") {
            error = "The file extension must be .json.";
            return false;
        }

        result = kCameraDirectory / fileName;
        return true;
    }
#endif
}

void ImGuiManager::RefreshCameraFiles()
{
    auto& ui = cameraFileUi_;
    ui.files.clear();
    ui.selectedFile = -1;
    ui.message.clear();

    std::error_code ec;
    std::filesystem::create_directories(kCameraDirectory, ec);
    if (ec) {
        ui.message = "Could not open camera folder: " + ec.message();
        return;
    }

    std::filesystem::directory_iterator it(kCameraDirectory, ec);
    const std::filesystem::directory_iterator end;
    while (!ec && it != end) {
        const bool regular = it->is_regular_file(ec);
        if (ec) {
            break;
        }
        if (regular && LowerCameraFileName(CameraPathText(it->path().extension())) == ".json") {
            ui.files.push_back(it->path());
        }
        it.increment(ec);
    }
    if (ec) {
        ui.files.clear();
        ui.message = "Could not list camera files: " + ec.message();
        return;
    }
    std::sort(ui.files.begin(), ui.files.end());
}

bool ImGuiManager::SaveCameraToPath(CameraManager* cameraManager, const std::filesystem::path& path)
{
    std::string error;
    if (!cameraManager->SaveCameraFile(path, error)) {
        cameraFileUi_.message = "Save failed: " + error;
        return false;
    }
    cameraFileUi_.currentPath = path;
    cameraFileUi_.message = "Saved: " + CameraPathText(path.filename());
    return true;
}

void ImGuiManager::DrawCameraFileControls(CameraManager* cameraManager)
{
#ifdef USE_IMGUI
    auto& ui = cameraFileUi_;
    const char* scene = SceneManager::GetInstance()->GetCurrentSceneName();
    const std::string sceneName = scene ? scene : "";
    // 別シーンの編集を、前のシーンの保存先へ上書きしない。
    if (ui.sceneName != sceneName) {
        ui = CameraFileUiState{};
        ui.sceneName = sceneName;
    }

    const auto& cameras = cameraManager->GetCameras();
    const bool playing = std::any_of(cameras.begin(), cameras.end(), [](const auto& info) {
        return info.camera && info.camera->GetIsLerp();
    });
    const bool hasCamera = std::any_of(cameras.begin(), cameras.end(), [](const auto& info) {
        return info.camera != nullptr;
    });

    ImGui::SeparatorText("Camera Files");
    ImGui::TextWrapped("Current file: %s", ui.currentPath.empty() ? "(not saved)" : CameraPathText(ui.currentPath).c_str());
    ImGui::TextDisabled("Stores all normal cameras, poses A/B and duration.");
    ImGui::BeginDisabled(playing || !hasCamera);
    bool openSaveAs = false;
    if (ImGui::Button("Save##CameraFile")) {
        if (ui.currentPath.empty()) {
            openSaveAs = true;
        } else {
            SaveCameraToPath(cameraManager, ui.currentPath);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save As...##CameraFile")) {
        openSaveAs = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Load...##CameraFile")) {
        RefreshCameraFiles();
        ImGui::OpenPopup("Load Camera File");
    }

    ImGui::EndDisabled();
    if (playing) {
        ImGui::TextDisabled("Stop playback before saving or loading.");
    }

    if (!ui.message.empty()) {
        ImGui::TextWrapped("%s", ui.message.c_str());
    }

    if (openSaveAs) {
        ui.confirmOverwrite = false;
        ui.pendingSavePath.clear();
        ui.message.clear();
        const std::string name = ui.currentPath.empty() ? "" : CameraPathText(ui.currentPath.filename());
        std::snprintf(ui.fileName, sizeof(ui.fileName), "%s", name.c_str());
        ImGui::OpenPopup("Save Camera File As");
    }

    ImGui::SetNextWindowSize(ImVec2(480.0f, 0.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Save Camera File As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Folder: resources/json/cameras/");
        if (ui.confirmOverwrite) {
            ImGui::TextWrapped("Replace existing file: %s?", CameraPathText(ui.pendingSavePath.filename()).c_str());
            ImGui::BeginDisabled(playing || !hasCamera);
            if (ImGui::Button("Overwrite")) {
                if (SaveCameraToPath(cameraManager, ui.pendingSavePath)) {
                    ui.confirmOverwrite = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Back")) {
                ui.confirmOverwrite = false;
                ui.message.clear();
            }
        } else {
            ImGui::InputText("File Name", ui.fileName, sizeof(ui.fileName));
            ImGui::TextDisabled("Example: title_intro.json (.json is added if omitted)");
            ImGui::BeginDisabled(playing || !hasCamera);
            if (ImGui::Button("Save File")) {
                ui.message.clear();
                if (MakeCameraSavePath(ui.fileName, ui.pendingSavePath, ui.message)) {
                    std::error_code ec;
                    const bool exists = std::filesystem::exists(ui.pendingSavePath, ec);
                    if (ec) {
                        ui.message = "Could not check file: " + ec.message();
                    } else if (exists) {
                        ui.confirmOverwrite = true;
                    } else if (SaveCameraToPath(cameraManager, ui.pendingSavePath)) {
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ui.confirmOverwrite = false;
            ImGui::CloseCurrentPopup();
        }

        if (!ui.message.empty()) {
            ImGui::TextWrapped("%s", ui.message.c_str());
        }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowSize(ImVec2(480.0f, 0.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Load Camera File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Folder: resources/json/cameras/");
        if (ImGui::Button("Refresh")) {
            RefreshCameraFiles();
        }

        if (ImGui::BeginListBox("##CameraFiles", ImVec2(440.0f, 180.0f))) {
            for (int i = 0; i < static_cast<int>(ui.files.size()); ++i) {
                const std::string name = CameraPathText(ui.files[i].filename());
                ImGui::PushID(i);
                if (ImGui::Selectable(name.c_str(), ui.selectedFile == i)) {
                    ui.selectedFile = i;
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        if (ui.files.empty()) {
            ImGui::TextDisabled("No camera JSON files found.");
        }

        ImGui::TextWrapped("Loading replaces matching camera settings and stops playback.");
        const bool selected = ui.selectedFile >= 0 && ui.selectedFile < static_cast<int>(ui.files.size());
        ImGui::BeginDisabled(playing || !hasCamera || !selected);
        if (ImGui::Button("Load File")) {
            const auto path = ui.files[ui.selectedFile];
            std::string error;
            if (cameraManager->LoadCameraFile(path, error)) {
                ui.currentPath = path;
                ui.message = "Loaded: " + CameraPathText(path.filename());
                ImGui::CloseCurrentPopup();
            } else {
                ui.message = "Load failed: " + error;
            }
        }

        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        if (!ui.message.empty()) {
            ImGui::TextWrapped("%s", ui.message.c_str());
        }
        ImGui::EndPopup();
    }
#endif
}

void ImGuiManager::DrawCameraWindow(CameraManager* cameraManager)
{
#ifdef USE_IMGUI
	if (!windowState_.showCamera || !cameraManager) {
		return;
	}
	if (!ImGui::Begin("Camera Manager", &windowState_.showCamera)) {
		ImGui::End();
		return;
	}

	DrawCameraFileControls(cameraManager);

	const auto& cameras = cameraManager->GetCameras();
	Camera* camera = cameraManager->GetActiveCamera();
	const bool playing = camera && camera->GetIsLerp();
	const int activeIndex = cameraManager->GetActtiveCamIndex();
	const char* cameraName = "No camera";
	if (cameraManager->GetIsDebug()) {
		cameraName = "Debug Camera";
	} else if (activeIndex >= 0 && activeIndex < static_cast<int>(cameras.size())) {
		cameraName = cameras[activeIndex].name.c_str();
	}

	ImGui::SeparatorText("Camera");
	ImGui::BeginDisabled(playing);
	if (ImGui::BeginCombo("Active Camera", cameraName)) {
		if (ImGui::Selectable("Debug Camera", cameraManager->GetIsDebug())) {
			cameraManager->SetActiveCamera(true);
		}
		for (int i = 0; i < static_cast<int>(cameras.size()); ++i) {
			ImGui::PushID(i);
			if (ImGui::Selectable(cameras[i].name.c_str(),
				!cameraManager->GetIsDebug() && cameraManager->GetActtiveCamIndex() == i)) {
				cameraManager->SetActiveCamera(false, i);
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	camera = cameraManager->GetActiveCamera();
	if (!camera) {
		ImGui::TextWrapped("Select a normal camera to edit poses and preview a transition.");
		ImGui::End();
		return;
	}

	const bool isPlaying = camera->GetIsLerp();
	bool manualEdit = cameraManager->GetIsManualEdit();
	ImGui::BeginDisabled(isPlaying);
	if (ImGui::Checkbox("Manual Edit", &manualEdit)) {
		cameraManager->SetIsManualEdit(manualEdit);
	}
	ImGui::EndDisabled();
	ImGui::Text("Status: %s", isPlaying ? "Playing A -> B" : (manualEdit ? "Manual editing" : "Scene control"));

	// 入力用の配列にコピーし、変更された構図だけを書き戻す。
	const auto editPose = [](Vector3& position, Vector3& rotation) {
		constexpr float radToDeg = 180.0f / std::numbers::pi_v<float>;
		constexpr float degToRad = std::numbers::pi_v<float> / 180.0f;
		float xyz[3] = { position.x, position.y, position.z };
		float degrees[3] = { rotation.x * radToDeg, rotation.y * radToDeg, rotation.z * radToDeg };
		const bool positionChanged = ImGui::DragFloat3("Position", xyz, 0.1f);
		const bool rotationChanged = ImGui::DragFloat3("Rotation (deg)", degrees, 0.1f);
		if (positionChanged) {
			position = { xyz[0], xyz[1], xyz[2] };
		}

		if (rotationChanged) {
			rotation = { degrees[0] * degToRad, degrees[1] * degToRad, degrees[2] * degToRad };
		}

		return positionChanged || rotationChanged;
	};

	if (ImGui::CollapsingHeader("Current Pose", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("Current");
		ImGui::BeginDisabled(isPlaying || !manualEdit);
		Vector3 position = camera->GetTranslate();
		Vector3 rotation = camera->GetRotate();
		if (editPose(position, rotation)) {
			camera->SetTranslate(position);
			camera->SetRotate(rotation);
		}

		ImGui::EndDisabled();
		if (!manualEdit && !isPlaying) {
			ImGui::TextDisabled("Enable Manual Edit to adjust the current pose.");
		}

		ImGui::PopID();
	}

	ImGui::BeginDisabled(isPlaying);
	if (ImGui::CollapsingHeader("A - Start Pose", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("Start");
		if (ImGui::Button("Capture Current")) {
			camera->SetStartPos(camera->GetTranslate());
			camera->SetStartRot(camera->GetRotate());
		}

		ImGui::SameLine();
		if (ImGui::Button("Go to A")) {
			cameraManager->SetIsManualEdit(true);
			camera->SetTranslate(camera->GetStartPos());
			camera->SetRotate(camera->GetStartRot());
		}

		Vector3 position = camera->GetStartPos();
		Vector3 rotation = camera->GetStartRot();
		if (editPose(position, rotation)) {
			camera->SetStartPos(position);
			camera->SetStartRot(rotation);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("B - Target Pose", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("Target");
		if (ImGui::Button("Capture Current")) {
			camera->SetTergetPos(camera->GetTranslate());
			camera->SetTergetRot(camera->GetRotate());
		}

		ImGui::SameLine();
		if (ImGui::Button("Go to B")) {
			cameraManager->SetIsManualEdit(true);
			camera->SetTranslate(camera->GetTergetPos());
			camera->SetRotate(camera->GetTergetRot());
		}

		Vector3 position = camera->GetTergetPos();
		Vector3 rotation = camera->GetTergetRot();
		if (editPose(position, rotation)) {
			camera->SetTergetPos(position);
			camera->SetTergetRot(rotation);
		}

		ImGui::PopID();
	}

	ImGui::SeparatorText("Playback");
	float duration = camera->GetDuration();
	if (ImGui::DragFloat("Duration (sec)", &duration, 0.1f, 0.01f, 120.0f,
		"%.2f", ImGuiSliderFlags_AlwaysClamp)) {
		camera->SetDuration(duration);
	}
	ImGui::TextDisabled("Ease in/out. Play restarts from A.");
	ImGui::TextDisabled("Stop holds the current pose.");
	if (ImGui::Button("Play A -> B")) {
		cameraManager->SetIsManualEdit(true);
		camera->StartLerp();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!isPlaying);
	if (ImGui::Button("Stop")) {
		camera->SetIsLerp(false);
		cameraManager->SetIsManualEdit(true);
	}
	ImGui::EndDisabled();

	ImGui::End();
#endif
}

void ImGuiManager::DrawMainMenuBar()
{
#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				requestSavePopup_ = true;
			}

			if (ImGui::MenuItem("Load")) {
				requestLoadPopup_ = true;
			}

			if (ImGui::MenuItem("Clear")) {
				requestClearPopup_ = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Stats", nullptr, &windowState_.showStats);
			ImGui::MenuItem("Camera", nullptr, &windowState_.showCamera);
			ImGui::MenuItem("Sound", nullptr, &windowState_.showSound);
			ImGui::MenuItem("Console", nullptr, &windowState_.showConsole);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (requestSavePopup_) {
		ImGui::OpenPopup("Confirm Save");
		requestSavePopup_ = false;
	}

	if (requestLoadPopup_) {
		ImGui::OpenPopup("Confirm Load");
		requestLoadPopup_ = false;
	}

	if (requestClearPopup_) {
		ImGui::OpenPopup("Confirm Clear");
		requestClearPopup_ = false;
	}

	DrawConfirmPopup();
#endif
}

void ImGuiManager::DrawLoggerWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showConsole) {
		ImGui::Begin("Console");

		if (ImGui::Button("Clear")) {
			Logger::ClearHistory();
		}

		ImGui::SameLine();

		static bool autoScroll = true;
		ImGui::Checkbox("Auto Scroll", &autoScroll);

		ImGui::Separator();

		ImGui::BeginChild("Log Region", ImVec2(0, 0), true);

		const auto logs = Logger::GetHistory();
		for (const auto& log : logs) {
			ImGui::TextUnformatted(log.c_str());
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}
#endif
}

void ImGuiManager::DrawSceneWindow(uint32_t srvIndex, uint32_t textureWidth, uint32_t textureHeight)
{
#ifdef USE_IMGUI
	if (ImGui::Begin("SceneWindow")) {
		const ImVec2 available = ImGui::GetContentRegionAvail();

		if (available.x > 0.0f && available.y > 0.0f &&
			textureWidth > 0 && textureHeight > 0) {
			const float scale = (std::min)(
				available.x / static_cast<float>(textureWidth),
				available.y / static_cast<float>(textureHeight));

			const ImVec2 imageSize(
				textureWidth * scale,
				textureHeight * scale
			);

			// 画像を中央へ配置
			const ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(
				cursor.x + (available.x - imageSize.x) * 0.5f,
				cursor.y + (available.y - imageSize.y) * 0.5f
			));

			const auto handle = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);

			ImGui::Image(reinterpret_cast<ImTextureID>(handle.ptr), imageSize);
		}
	}

	ImGui::End();
#endif
}

void ImGuiManager::DrawEditor()
{
	Editor::GetInstance()->Draw();
}

void ImGuiManager::DrawSoundWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showSound) {
		ImGui::Begin("Sound Manager");

		SoundManager* soundManager = SoundManager::GetInstance();
		auto volumeBGM = soundManager->GetCurrentBGMVolume();
		auto volumeSE = soundManager->GetCurrentSEVolume();
		auto volumeMaster = soundManager->GetCurrentMasterVolume();

		if (ImGui::DragFloat("BGM Volume", &volumeBGM, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeBGM(volumeBGM);
		}

		if (ImGui::DragFloat("SE Volume", &volumeSE, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeSE(volumeSE);
		}

		if (ImGui::DragFloat("Master Volume", &volumeMaster, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeMaster(volumeMaster);
		}

		ImGui::End();
	}
#endif
}

void ImGuiManager::ApplyStyle()
{
#ifdef USE_IMGUI
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* c = style.Colors;

	// --- 基本背景・テキスト ---
	c[ImGuiCol_Text] = ImVec4(0.90f, 0.96f, 1.00f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.55f, 0.65f, 1.00f);

	c[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.96f);
	c[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.03f, 1.00f);
	c[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.02f, 0.03f, 1.00f);

	c[ImGuiCol_Border] = ImVec4(0.20f, 0.70f, 1.00f, 0.80f);
	c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// --- フレーム（入力欄、スライダーなど） ---
	c[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.24f, 0.33f, 1.00f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.31f, 0.42f, 1.00f);

	// --- タイトルバー・ウィンドウ装飾 ---
	c[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.15f, 0.22f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.32f, 0.46f, 1.00f);
	c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.12f, 0.18f, 0.75f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.12f, 0.18f, 1.00f);

	// --- ボタン ---
	c[ImGuiCol_Button] = ImVec4(0.00f, 0.42f, 0.60f, 1.00f);
	c[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.00f, 0.25f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.80f, 0.00f, 1.00f);

	// --- ヘッダー（ツリーノード、リスト選択など） ---
	c[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.03f, 0.05f, 0.90f);
	c[ImGuiCol_Header] = ImVec4(0.00f, 0.40f, 0.60f, 0.40f); // 非選択時
	c[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.60f, 0.90f, 0.80f); // ホバーで発光
	c[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f); // 選択中で強発光

	// --- スクロールバー ---
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.05f, 0.07f, 1.00f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.38f, 0.52f, 1.00f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.50f, 0.68f, 1.00f);

	// --- スライダー ---
	c[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.25f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.35f, 0.00f, 1.00f);

	// --- チェックマーク ---
	c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.80f, 0.00f, 1.00f);

	// --- セパレーター ---
	c[ImGuiCol_Separator] = ImVec4(0.20f, 0.70f, 1.00f, 0.60f);
	c[ImGuiCol_SeparatorHovered] = ImVec4(0.28f, 0.52f, 0.72f, 1.00f);
	c[ImGuiCol_SeparatorActive] = ImVec4(0.38f, 0.68f, 0.90f, 1.00f);

	// --- リサイズグリップ ---
	c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.52f, 0.74f, 0.55f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.66f, 0.92f, 0.80f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f);

	// --- タブ ---
	c[ImGuiCol_Tab] = ImVec4(0.06f, 0.20f, 0.28f, 1.00f);
	c[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.45f, 0.65f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(0.00f, 0.58f, 0.82f, 1.00f);
	c[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.13f, 0.18f, 1.00f);
	c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.26f, 0.36f, 1.00f);

	// --- ドッキング・テーブル ---
	c[ImGuiCol_TableHeaderBg] = ImVec4(0.06f, 0.18f, 0.26f, 1.00f);
	c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.40f, 0.56f, 1.00f);
	c[ImGuiCol_TableBorderLight] = ImVec4(0.10f, 0.24f, 0.34f, 1.00f);
	c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	c[ImGuiCol_TableRowBgAlt] = ImVec4(0.07f, 0.10f, 0.14f, 0.60f);

	// --- プロット・ドラッグ・ドロップ ---
	c[ImGuiCol_PlotLines] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_PlotHistogram] = ImVec4(0.10f, 0.72f, 0.95f, 1.00f);
	c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.35f, 0.85f, 1.00f, 1.00f);
	c[ImGuiCol_DragDropTarget] = ImVec4(0.20f, 0.85f, 1.00f, 1.00f);

	// --- ナビゲーション・モーダル背景 ---
	c[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 0.70f);
	c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.03f, 0.06f, 0.60f);
	c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.82f);

	// --- テーマ全体の丸み・間隔調整 ---
	style.WindowRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.ChildRounding = 2.0f;
	style.PopupRounding = 2.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 2.0f;

	// 余白を少しコンパクトに
	style.WindowPadding = ImVec2(6.0f, 5.0f);
	style.FramePadding = ImVec2(5.0f, 2.0f);
	style.ItemSpacing = ImVec2(5.0f, 3.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 2.0f);

	// インデント・スクロールバー
	style.IndentSpacing = 14.0f;
	style.ScrollbarSize = 10.0f;
	style.GrabMinSize = 8.0f;

	// 境界線
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

#endif
}

void ImGuiManager::DrawDockSpace()
{
#ifdef USE_IMGUI
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", nullptr, windowFlags);

	ImGui::PopStyleVar(3);

	ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

	ImGui::End();
#endif
}

void ImGuiManager::DrawConfirmPopup()
{
#ifdef USE_IMGUI
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Confirm Save", nullptr, flags)) {
		ImGui::Text("現在のシーンを保存しますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			SaveScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Confirm Load", nullptr, flags)) {
		ImGui::Text("現在の編集内容を破棄してロードしますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			LoadScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Confirm Clear", nullptr, flags)) {
		ImGui::Text("現在のシーンの保存内容を破棄しますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ClearScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}

void ImGuiManager::SaveScenesJson()
{
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/clearScene.json");
	}
}

void ImGuiManager::LoadScenesJson() {
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/clearScene.json");
	}
}

void ImGuiManager::ClearScenesJson()
{
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/clearScene.json");
	}
}

json ImGuiManager::SaveEditorJson() const
{
	return nlohmann::json{
			{"showStats", windowState_.showStats},
			{"showCamera", windowState_.showCamera},
			{"showSound", windowState_.showSound},
	};
}

void ImGuiManager::LoadEditorJson(const nlohmann::json& j)
{
	if (j.contains("showStats")) {
		windowState_.showStats = j["showStats"].get<bool>();
	}

	if (j.contains("showCamera")) {
		windowState_.showCamera = j["showCamera"].get<bool>();
	}

	if (j.contains("showSound")) {
		windowState_.showSound = j["showSound"].get<bool>();
	}

}
