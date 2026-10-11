#pragma once
#include "Graphics.h"

class RenderTexture
{
public:
	RenderTexture() = default;
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	void Create(uint32_t width, uint32_t height);
	// Shares Graphics' depth buffer; keep dimensions equal to the main window.
	// Resize before recording a frame; Shutdown before SrvManager/Graphics shutdown.
	void Resize(uint32_t width, uint32_t height);
	void Shutdown();
	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }

	void BeginWrite(ID3D12GraphicsCommandList* cmdList);
	void EndWrite(ID3D12GraphicsCommandList* cmdList);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const;
	uint32_t GetSrvIndex() const;
	ID3D12Resource* GetResource() const;

private:
	void CreateResource(uint32_t width, uint32_t height);

	uint32_t width_ = 0;
	uint32_t height_ = 0;
	bool descriptorAllocated_ = false;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
	uint32_t srvIndex_ = 0;
	D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
};

