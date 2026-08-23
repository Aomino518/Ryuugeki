#pragma once
#include "Enemy.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct EnemySpawnData {
	float spawnTime;
	EnemyPattern enemyPattern;
	Vector3 position;
};

class Player;
class Enemy;
class EnemyManager
{
public:
	// 初期化
	void Init(std::shared_ptr<Player> player);

	// 更新
	void Update();

	// 描画
	void Draw();

	void DebugDraw();

	bool GetIsFinished() const { return isFinished_; }

	void SetIsMoveStop(bool flag);

	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }

private:
	// メンバ関数
	// JSONから出現スケジュールを読み込む
	bool LoadSpawnSchedule(const std::string& filePath);
	// 文字列をEnemyPatternに変換する
	EnemyPattern ConvertEnemyPattern(const std::string& patternName) const;

	void SpawnEnemy(const EnemySpawnData& data);

	// メンバ変数
	std::vector<std::unique_ptr<Enemy>> enemies_;
	std::vector<EnemySpawnData> spawnList_;
	std::weak_ptr<Player> player_;

	float elapsedTime_ = 0.0f;
	float endTime_ = 140.0f;
	bool isFinished_ = false;
	bool isStopFrame_ = false;
};

