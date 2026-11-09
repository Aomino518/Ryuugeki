#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "CreateResorceUtils.h"
#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include "MathFunc.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Picking.h"

class Model
{
public:
	void Init(const std::string& directoryPath, const std::string& filename, const std::string& path);

	void Draw();
	bool IntersectRay(const Picking::LocalRay& ray, float& distance) const;

	void LoadObjFile(const std::string& directoryPath, const std::string& filename, const std::string& extension);

	ModelData GetRootNode() const { return modelData_; }

private:
	void CreateBufferResources();
	Node ReadNode(aiNode* node);

	// モデルデータ
	ModelData modelData_ = {};
	DirectX::BoundingBox boundingBox_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

