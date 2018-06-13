// sender.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <d3d11.h>
#include <d3d9.h>
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


bool createSharedTexture(ID3D11Device* device, ID3D11Texture2D** pSharedTexture,
	HANDLE *handle) {
	D3D11_TEXTURE2D_DESC texDesc;



	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = WIDTH;
	texDesc.Height = HEIGHT;

	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // This texture will be shared

	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	texDesc.SampleDesc.Quality = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;


	unsigned char *pixels = new unsigned char[WIDTH*HEIGHT * 4];

	unsigned char r = rand() % 0xff;
	unsigned char g = rand() % 0xff;
	unsigned char b = rand() % 0xff;
	for (int i = 0; i< WIDTH*HEIGHT*4;i+=4)
	{
		pixels[i + 0] = b;
		pixels[i + 1] = g;
		pixels[i + 2] = r;
		pixels[i + 3] = 0xff;
	}

	ID3D11Texture2D *shared_texture = NULL;
	D3D11_SUBRESOURCE_DATA data;
	data.SysMemPitch = WIDTH * 4;
	data.SysMemSlicePitch = WIDTH * 4;
	data.pSysMem = pixels;
	auto hr = device->CreateTexture2D(&texDesc, &data, &shared_texture);
	delete pixels;

	if (hr != S_OK) {
		return false;
	}



	IDXGIResource* pOtherResource(NULL);
	hr = shared_texture->QueryInterface(__uuidof(IDXGIResource), (void**)&pOtherResource);
	if (hr != S_OK) {
		return false;
	}

	hr = pOtherResource->GetSharedHandle(handle);

	pOtherResource->Release();

	*pSharedTexture = shared_texture;

	return true;
}


int main()
{
	auto device = init();
	if (device)
	{

		SharedMemoryUtils smu;
		smu.createSharedMemory();

		while (1)
		{
			ID3D11Texture2D* pSharedTexture = nullptr;
			HANDLE handle = nullptr;
			if (createSharedTexture(device, &pSharedTexture, &handle)) {

				printf("handle : %p\n", handle);

				Sleep(1000);
				smu.Send(handle);

				pSharedTexture->Release();


			}


			Sleep(1000);
		}
	}

    return 0;
}

