#include "RenderTexture.h"
#include "Vector4.h"
#include "SrvManager.h"
#include "CreateResorceUtils.h"
#include <cassert>

void RenderTexture::Create(uint32_t width, uint32_t height)
{
    assert(width > 0);
    assert(height > 0);

    assert(!resource_);

    if (!descriptorAllocated_) {
        // Own the RTV heap so shutdown/recreation does not consume shared RTV slots.
        rtvHeap_ = Graphics::CreateDescriptorHeap(Graphics::GetDevice(),
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
        rtvHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
        srvIndex_ = SrvManager::GetInstance()->Allocate();
        descriptorAllocated_ = true;
    }

    CreateResource(width, height);
}

void RenderTexture::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) {
        return;
    }

    if (width_ == width && height_ == height) {
        return;
    }

    assert(descriptorAllocated_);

    // Call before recording commands for the new frame.
    Graphics::GetInstance()->WaitGPU();
    resource_.Reset();

    CreateResource(width, height);
}

void RenderTexture::BeginWrite(ID3D12GraphicsCommandList* cmdList)
{
    assert(resource_);

    if (state_ != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource_.Get();
        barrier.Transition.StateBefore = state_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);
        state_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    Graphics* graphics = Graphics::GetInstance();
    assert(width_ == Graphics::GetWidth() && height_ == Graphics::GetHeight());
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = graphics->DsvHandle();
    // TransitionBarrierを張る
    const float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
    cmdList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle);
    cmdList->ClearRenderTargetView(rtvHandle_, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    SrvManager::GetInstance()->PreDraw();

    D3D12_VIEWPORT viewport = graphics->GetViewport();
    D3D12_RECT scissorRect = graphics->GetScissorRect();

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);
}

void RenderTexture::EndWrite(ID3D12GraphicsCommandList* cmdList)
{
    assert(resource_);

    if (state_ != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource_.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = state_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        cmdList->ResourceBarrier(1, &barrier);
        state_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetRTV() const
{
	return rtvHandle_;
}

uint32_t RenderTexture::GetSrvIndex() const
{
	return srvIndex_;
}

ID3D12Resource* RenderTexture::GetResource() const
{
	return resource_.Get();
}

void RenderTexture::Shutdown()
{
    if (!descriptorAllocated_) {
        return;
    }
    Graphics::GetInstance()->WaitGPU();
    resource_.Reset();
    rtvHeap_.Reset();
    SrvManager::GetInstance()->Free(srvIndex_);
    descriptorAllocated_ = false;
    width_ = height_ = 0;
    srvIndex_ = 0;
    rtvHandle_ = {};
}

void RenderTexture::CreateResource(uint32_t width, uint32_t height)
{
    ID3D12Device* device = Graphics::GetDevice();

    const Vector4 clearColor{ 0.1f, 0.25f, 0.5f, 1.0f };

    resource_ = CreateRenderTextureResource(
        device,
        width,
        height,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        clearColor
    );

    assert(resource_);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    device->CreateRenderTargetView(
        resource_.Get(),
        &rtvDesc,
        rtvHandle_
    );

    SrvManager::GetInstance()->CreateSRVforTexture2D(
        srvIndex_,
        resource_.Get(),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        1
    );

    width_ = width;
    height_ = height;
    state_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}
