// sender.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <d3d11.h>
#include <d3d9.h>
#include "Bitmap.h"
#include "shared_memory_utils.h"

#define WIDTH 512
#define HEIGHT 512

int						g_AdapterIndex = D3DADAPTER_DEFAULT;
IDXGIAdapter*			g_pAdapterDX11 = nullptr;
ID3D11DeviceContext*	g_pImmediateContext = nullptr;
D3D_DRIVER_TYPE			g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL		g_featureLevel = D3D_FEATURE_LEVEL_11_0;


ID3D11Device* init()
{
	ID3D11Device* pd3dDevice = NULL;
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;
	IDXGIAdapter* pAdapterDX11 = g_pAdapterDX11;

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);


	D3D_FEATURE_LEVEL featureLevels[] = {
		// D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	hr = D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,
		&pd3dDevice,
		&g_featureLevel,
		&g_pImmediateContext);


	if (FAILED(hr))
		return NULL;

	return pd3dDevice;

} // end CreateDX11device




bool textureToBMP(ID3D11DeviceContext *context, ID3D11Device* device, ID3D11Texture2D* tex) {
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = WIDTH;
	texDesc.Height = HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D *spD3D11Texture2D = NULL;
	auto hr = device->CreateTexture2D(&texDesc, NULL, &spD3D11Texture2D);
	if (hr != S_OK) {
		return false;
	}

	context->CopyResource(spD3D11Texture2D, tex);
	Sleep(1000);

	IDXGISurface *hStagingSurf = NULL;
	hr = spD3D11Texture2D->QueryInterface(__uuidof(IDXGISurface), (void **)(&hStagingSurf));
	if (hr != S_OK)
	{
		return false;
	}

	DXGI_MAPPED_RECT mappedRect;
	hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);

	if (SUCCEEDED(hr))
	{
		static int i = 0;
		char name[1024] = { 0 };
		sprintf(name, "receiver_%6d.bmp", i++);
		SaveARGB(name, (BYTE*)mappedRect.pBits, WIDTH, HEIGHT, WIDTH);




		hStagingSurf->Unmap();
	}

	hStagingSurf->Release();
}

int main()
{
	auto device = init();
	if (device)
	{

		SharedMemoryUtils smu;
		smu.openSharedMemory();

		while (1)
		{
			HANDLE handle = 0;
			if (smu.Get(handle))
			{
				ID3D11Texture2D* pTargetTexture = nullptr;
				auto hr = device->OpenSharedResource(handle, __uuidof(ID3D11Resource), (void**)(&pTargetTexture));
				if (hr != S_OK) {
					break;
				}
				textureToBMP(g_pImmediateContext, device, pTargetTexture);
				pTargetTexture->Release();
			}
		}
	}

	return 0;
}

