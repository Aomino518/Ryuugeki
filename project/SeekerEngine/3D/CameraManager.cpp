#include "CameraManager.h"
#include <algorithm>
#include <numbers>
#include <unordered_set>
#include <cmath>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>
#include "Logger.h"

namespace
{
	// SaveCamera()の形式と照合し、LoadCamera()の型変換前に検証する。
	bool ValidateCameraJson(const nlohmann::json& data,
		const nlohmann::json& schema, const std::string& path, std::string& error)
	{
		if (schema.is_object()) {
			if (!data.is_object()) {
				error = "オブジェクトが必要です: " + path;
				return false;
			}
			for (auto it = schema.begin(); it != schema.end(); ++it) {
				const auto value = data.find(it.key());
				const std::string field = path + "." + it.key();
				if (value == data.end()) {
					error = "項目がありません: " + field;
					return false;
				}
				if (!ValidateCameraJson(*value, it.value(), field, error)) {
					return false;
				}
			}
			return true;
		}

		if (!schema.is_number() || !data.is_number()) {
			error = "数値が必要です: " + path;
			return false;
		}
		const double value = data.get<double>();
		if (!std::isfinite(value) || std::abs(value) > (std::numeric_limits<float>::max)()) {
			error = "数値が範囲外です: " + path;
			return false;
		}
		return true;
	}
}

CameraManager* CameraManager::GetInstance() {
	static CameraManager instance;
	return &instance;
}

void CameraManager::Init()
{
	CreateCamera("MainCamera");
	cameras_[0].camera->SetTranslate({0.0f, 23.0f, -10.0f});
	cameras_[0].camera->SetRotate(
		{ std::numbers::pi_v<float> / 3.0f,
		 0.0f,
		 0.0f }
	);
	cameras_[0].camera->SetScale({ 1.0f, 1.0f, 1.0f });
	CreateDebugCamera();
	SetActiveCamera(false, 0);
}

void CameraManager::Update()
{
	if (activeIsDebug_) {
		if (debugCamera_) {
			debugCamera_->Update();
		}
	} else {
		if (cameras_.empty() || activeCamIndex_ < 0 || activeCamIndex_ >= static_cast<int>(cameras_.size())) {
			return;
		}

		if (!isManualEdit_ || cameras_[activeCamIndex_].camera->GetIsLerp()) {
			cameras_[activeCamIndex_].camera->UpdateLerpCamera();
		}
		cameras_[activeCamIndex_].camera->Update();
	}
}

void CameraManager::Shutdown()
{
	cameras_.clear();
	debugCamera_.reset();
}	

void CameraManager::CreateCamera(const std::string& cameraName)
{
	for (int i = 0; i < cameras_.size(); ++i) {
		if (cameras_[i].name == cameraName) {
			return;
		}
	}

	CameraInfo cameraInfo;
	cameraInfo.name = cameraName;
	cameraInfo.camera = std::make_unique<Camera>();
	cameraInfo.camera->SetTranslate({0.0f, 10.0f, -10.0f});
	cameraInfo.camera->SetRotate({0.0f, 0.0f, 0.0f});
	cameras_.push_back(std::move(cameraInfo));
}

void CameraManager::CreateDebugCamera()
{
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
}

void CameraManager::SetActiveCamera(bool isDebug, int index)
{
	if (cameras_.empty()) {
		activeIsDebug_ = true;
		activeCamIndex_ = 0;
	}

	activeIsDebug_ = isDebug;
	activeCamIndex_ = std::clamp(index, 0, static_cast<int>(cameras_.size()) - 1);
}

void CameraManager::SetActiveCameraByName(const std::string& name)
{
	for (int i = 0; i < cameras_.size(); ++i) {
		if (cameras_[i].name == name) {
			activeCamIndex_ = i;
			activeIsDebug_ = false;
			return;
		}
	}
	assert(false && "指定された名前のカメラが存在しません");
}

Camera* CameraManager::GetActiveCamera() const
{
	if (activeIsDebug_) {
		return nullptr;
	}

	if (cameras_.empty() || activeCamIndex_ < 0 || activeCamIndex_ >= static_cast<int>(cameras_.size())) {
		return nullptr;
	}

	return  cameras_[activeCamIndex_].camera.get();
}

Camera* CameraManager::GetCamera(const std::string& name)
{
	for(auto& cam : cameras_) {
		if (cam.name == name) {
			return cam.camera.get();
		}
	}

	assert(false && "指定された名前のカメラが見つかりません");
	return nullptr;
}

nlohmann::json CameraManager::SaveToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto& info : cameras_) {
		nlohmann::json camJson;
		camJson["name"] = info.name;
		if (info.camera) {
			camJson["camera"] = info.camera->SaveToJson();
		}
		j.push_back(camJson);
	}

	return j;
}

void CameraManager::LoadFromJson(const nlohmann::json& j)
{
	if (!j.is_array()) {
		Logger::Write(Logger::LogLevel::Warning, "Not Camera Array");
		return;
	}

	if (j.empty()) {
		Logger::Write(Logger::LogLevel::Warning, "Camera Array is empty. Keep current cameras.");
		return;
	}

	std::vector<CameraInfo> loadedCameras;
	for (const auto& item : j) {
		if (!item.is_object() || !item.contains("camera") || !item["camera"].is_object()) {
			continue;
		}

		CameraInfo info;
		info.name = item.value("name", "untitled");

		info.camera = std::make_unique<Camera>();
		if (item["camera"].contains("transform")) {
			info.camera->LoadFromJson(item["camera"]);
		}
		loadedCameras.push_back(std::move(info));
	}

	if (loadedCameras.empty()) {
		Logger::Write(Logger::LogLevel::Warning, "No valid cameras in JSON. Keep current cameras.");
		return;
	}

	cameras_ = std::move(loadedCameras);
	SetActiveCamera(false, activeCamIndex_);
}

nlohmann::json CameraManager::SaveLerpCamera() const
{
	// カメラ名とデータをまとめる
	nlohmann::json cameraList = nlohmann::json::array();

	for (const auto& info : cameras_) {
		if (!info.camera) {
			continue;
		}

		cameraList.push_back({
			{"name", info.name},
			{"camera", info.camera->SaveCamera()}
			});
	}

	return {
		{"version", 1},
		{"cameras", cameraList}
	};
}

bool CameraManager::LoadLerpCamera(const nlohmann::json& j, std::string& error)
{
	error.clear();

	if (!j.is_object()) {
		error = "カメラファイルの型式が正しくありません";
		return false;
	}

	const auto version = j.find("version");
	if (version == j.end() || !version->is_number_integer() || *version != 1) {
		error = "対応していないファイルバージョンです";
		return false;
	}

	const auto cameraList = j.find("cameras");
	if (cameraList == j.end() || !cameraList->is_array() || cameraList->empty()) {
		error = "カメラ一覧がありません";
		return false;
	}

	// 反映ポインターと読み込み確認用コピー
	std::vector<std::pair<Camera*, Camera>> pending;
	std::unordered_set<std::string> names;

	for (const auto& item : *cameraList) {
		if (!item.is_object()) {
			error = "カメラ一覧の要素がオブジェクトではありません";
			return false;
		}
		const auto nameValue = item.find("name");
		if (nameValue == item.end() || !nameValue->is_string()) {
			error = "カメラ名がないか、文字列ではありません";
			return false;
		}
		const std::string name = nameValue->get<std::string>();

		if (!names.insert(name).second) {
			error = "カメラ名が重複しています: " + name;
			return false;
		}

		Camera* destination = nullptr;

		for (auto& info : cameras_) {
			if (info.name == name && info.camera) {
				destination = info.camera.get();
				break;
			}
		}

		if (!destination) {
			error = "カメラが見つかりません: " + name;
			return false;
		}

		const auto data = item.find("camera");
		if (data == item.end() || !data->is_object()) {
			error = "カメラデータがないか、形式が不正です: " + name;
			return false;
		}
		if (!ValidateCameraJson(*data, destination->SaveCamera(), name, error)) {
			return false;
		}

		Camera temporary = *destination;
		temporary.LoadCamera(*data);

		const float duration = temporary.GetDuration();
		if (!std::isfinite(duration) || duration <= 0.0f) {
			error = "移動時間は正の有限値で指定してください: " + name;
			return false;
		}
		const Vector3& scale = temporary.GetScale();
		if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
			error = "カメラのスケールに0は指定できません: " + name;
			return false;
		}

		temporary.SetIsLerp(false);
		temporary.Update();

		pending.emplace_back(destination, temporary);
	}

	for (const auto& entry : pending) {
		*entry.first = entry.second;
	}

	SetIsManualEdit(true);

	return true;
}

bool CameraManager::SaveCameraFile(const std::filesystem::path& path, std::string& error) const
{
	error.clear();

	if (path.has_parent_path()) {
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);

		if (ec) {
			error = "保存フォルダーを作成できません: " + ec.message();
			return false;
		}
	}

	const nlohmann::json data = SaveLerpCamera();
	const std::string text = data.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
	std::ofstream file(path);

	if (!file.is_open()) {
		error = "保存ファイルを開けません";
		return false;
	}

	file << text;
	file.close();

	if (file.fail()) {
		error = "ファイルの書き込みに失敗しました";
		return false;
	}

	return true;
}

bool CameraManager::LoadCameraFile(const std::filesystem::path& path, std::string& error)
{
	error.clear();

	std::ifstream file(path);

	if (!file.is_open()) {
		error = "読み込みファイルを開けません";
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(file, nullptr, false);

	if (file.bad()) {
		error = "ファイルの読み取りに失敗しました";
		return false;
	}

	if (data.is_discarded()) {
		error = "JSONの書式が正しくありません";
		return false;
	}

	return LoadLerpCamera(data, error);
}

bool CameraManager::RemoveCamera(const std::string& cameraName)
{
	for (int i = 0; i < static_cast<int>(cameras_.size()); i++) {
		if (cameras_[i].name != cameraName) {
			continue;
		}

		// 使用中のカメラまたは最後の通常カメラを削除する場合
		if (cameras_.size() == 1 || (!activeIsDebug_ && activeCamIndex_ == i)) {
			SetActiveCamera(true);
			activeCamIndex_ = 0;
		} else if(i < activeCamIndex_) {
			--activeCamIndex_;
		}

		cameras_.erase(cameras_.begin() + i);
		return true;
	}

	return false;
}
