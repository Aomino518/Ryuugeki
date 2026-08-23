#include "EnemyManager.h"
#include "Player.h"
#include "Enemy.h"

void EnemyManager::Init(std::shared_ptr<Player> player) {
	enemies_.clear();
	spawnList_.clear();
	player_ = player;

	if (!LoadSpawnSchedule("resources/json/enemySpawn.json")) {
		Logger::Write(Logger::LogLevel::Error, "敵の出現スケジュールを読み込めませんでした");
	}
}

void EnemyManager::Update() {
	if (!isStopFrame_) {
		elapsedTime_ += Time::GetDeltaTime();
	}

	if (elapsedTime_ >= endTime_ && enemies_.empty()) {
		isFinished_ = true;
	}

	// 出現チェック
	for (auto it = spawnList_.begin(); it != spawnList_.end();) {
		if (elapsedTime_ >= it->spawnTime) {
			SpawnEnemy(*it);
			it = spawnList_.erase(it);
		} else {
			it++;
		}
	}

	auto player = player_.lock();
	if (!player) {
		return;
	}

	auto& bullets = player->GetBullets();

	for (auto& bullet : bullets) {
		if (bullet->GetIsShot()) {
			for (auto it = enemies_.begin(); it != enemies_.end();) {
				Enemy* enemy = it->get();

				if (!enemy->GetIsAlive()) {
					++it;
					continue;
				}

				if (IsCollision(bullet->GetSphere(), enemy->GetSphere())) {
					bullet->SetDebugHit();
					bullet->SetIsShot(false);
					enemy->SetIsDebugHit();
					it = enemies_.erase(it);
					break;
				} else {
					it++;
				}
			}
		}
	}

	if (player->GetIsAlive()) {
		for (auto it = enemies_.begin(); it != enemies_.end();) {
			Enemy* enemy = it->get();

			if (!enemy->GetIsAlive()) {
				++it;
				continue;
			}

			auto& enemyBullet = enemy->GetBullet();

			if (IsCollision(player->GetSphere(), enemyBullet->GetSphere())) {
				player->SetIsDebugHit();
				player->SetIsAlive(false);
				enemyBullet->SetIsDebugHit();
				enemyBullet->SetIsShot(false);
			}

			if (IsCollision(player->GetSphere(), enemy->GetSphere())) {
				player->SetIsAlive(false);
				it = enemies_.erase(it);
			} else {
				it++;
			}
		}
	}

	// 敵の更新
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		(*it)->Update();

		if ((*it)->GetPosition().z < -50.0f) {
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::DebugDraw()
{
	for (auto& enemy : enemies_) {
		enemy->DebugDraw();
	}
}

void EnemyManager::SetIsMoveStop(bool flag)
{
	isStopFrame_ = flag;
	for (auto& enemy : enemies_) {
		enemy->SetIsMoveStop(flag);
	}
}

bool EnemyManager::LoadSpawnSchedule(const std::string& filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	json root;
	file >> root;

	for (const json& enemyJson : root.at("enemies")) {
		EnemySpawnData data{};

		data.spawnTime = enemyJson.at("spawnTime").get<float>();
		const std::string patternName = enemyJson.at("pattern").get<std::string>();
		data.enemyPattern = ConvertEnemyPattern(patternName);
		const json& positionJson = enemyJson.at("position");
		data.position = {
			positionJson.at("x").get<float>(),
			positionJson.at("y").get<float>(),
			positionJson.at("z").get<float>()
		};

		Logger::Write(
			Logger::LogLevel::Debug,
			std::format(
				"敵スケジュール読込: "
				"time={:.3f}, pattern={}, "
				"position=({:.2f}, {:.2f}, {:.2f})",
				data.spawnTime,
				patternName,
				data.position.x,
				data.position.y,
				data.position.z));

		spawnList_.push_back(data);
	}

	return true;
}

EnemyPattern EnemyManager::ConvertEnemyPattern(const std::string& patternName) const
{
	if (patternName == "Straight") {
		return EnemyPattern::Straight;
	}

	if (patternName == "SinWave") {
		return EnemyPattern::SinWave;
	}

	if (patternName == "ZigZag") {
		return EnemyPattern::ZigZag;
	}

	if (patternName == "SlowFast") {
		return EnemyPattern::SlowFast;
	}

	throw std::runtime_error("未対応のEnemyPatternです: " + patternName);
}

void EnemyManager::SpawnEnemy(const EnemySpawnData& data)
{
	auto player = player_.lock();
	if (!player) {
		Logger::Write(Logger::LogLevel::Error, "敵生成時にplayerを取得できませんでした");
		return;
	}

	const Vector3 playerPos = player->GetPosition();
	const Vector3 spawnPos = {
		data.position.x,
		data.position.y,
		playerPos.z - data.position.z
	};

	Logger::Write(
		Logger::LogLevel::Debug,
		std::format(
			"敵生成: player=({:.2f}, {:.2f}, {:.2f}), "
			"offset=({:.2f}, {:.2f}, {:.2f}), "
			"spawn=({:.2f}, {:.2f}, {:.2f})",
			playerPos.x,
			playerPos.y,
			playerPos.z,
			data.position.x,
			data.position.y,
			data.position.z,
			spawnPos.x,
			spawnPos.y,
			spawnPos.z));

	auto enemy = std::make_unique<Enemy>();
	enemy->Init(data.enemyPattern, data.position);
	enemies_.push_back(std::move(enemy));
}
