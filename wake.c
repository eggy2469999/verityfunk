#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

static const CLSID CLSID_PolicyConfigClient =
    {0x870af99c,0x171d,0x4f9e,{0xaf,0x0d,0xe6,0x3d,0xf4,0x0d,0xb4,0x1b}};
static const IID IID_IPolicyConfig =
    {0xf8679f50,0x850a,0x41cf,{0x9c,0x72,0x43,0x0f,0x29,0x06,0x0d,0x21}};

typedef struct IPolicyConfigVtbl IPolicyConfigVtbl;
typedef struct IPolicyConfig { IPolicyConfigVtbl *lpVtbl; } IPolicyConfig;

struct IPolicyConfigVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IPolicyConfig*, const IID*, void**);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IPolicyConfig*);
    ULONG   (STDMETHODCALLTYPE *Release)(IPolicyConfig*);
    HRESULT (STDMETHODCALLTYPE *GetMixFormat)(IPolicyConfig*, LPCWSTR, void**);
    HRESULT (STDMETHODCALLTYPE *GetDeviceFormat)(IPolicyConfig*, LPCWSTR, INT, void**);
    HRESULT (STDMETHODCALLTYPE *ResetDeviceFormat)(IPolicyConfig*, LPCWSTR);
    HRESULT (STDMETHODCALLTYPE *SetDeviceFormat)(IPolicyConfig*, LPCWSTR, void*, void*);
    HRESULT (STDMETHODCALLTYPE *GetProcessingPeriod)(IPolicyConfig*, LPCWSTR, INT, void*, void*);
    HRESULT (STDMETHODCALLTYPE *SetProcessingPeriod)(IPolicyConfig*, LPCWSTR, void*);
    HRESULT (STDMETHODCALLTYPE *GetShareMode)(IPolicyConfig*, LPCWSTR, void*);
    HRESULT (STDMETHODCALLTYPE *SetShareMode)(IPolicyConfig*, LPCWSTR, void*);
    HRESULT (STDMETHODCALLTYPE *GetPropertyValue)(IPolicyConfig*, LPCWSTR, const void*, void*);
    HRESULT (STDMETHODCALLTYPE *SetPropertyValue)(IPolicyConfig*, LPCWSTR, const void*, void*);
    HRESULT (STDMETHODCALLTYPE *SetDefaultEndpoint)(IPolicyConfig*, LPCWSTR, int);
    HRESULT (STDMETHODCALLTYPE *SetEndpointVisibility)(IPolicyConfig*, LPCWSTR, int);
};

static void force_speakers(void) {
    IMMDeviceEnumerator *en = NULL;
    IMMDeviceCollection *coll = NULL;
    IPolicyConfig *pol = NULL;
    HRESULT hr;

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &IID_IMMDeviceEnumerator, (void**)&en);
    if (FAILED(hr) || !en) goto done;

    hr = en->lpVtbl->EnumAudioEndpoints(en, eRender, DEVICE_STATE_ACTIVE, &coll);
    if (FAILED(hr) || !coll) goto done;

    UINT n = 0;
    coll->lpVtbl->GetCount(coll, &n);

    for (UINT i = 0; i < n; i++) {
        IMMDevice *dev = NULL;
        IPropertyStore *props = NULL;
        PROPVARIANT pv;
        PropVariantInit(&pv);

        if (FAILED(coll->lpVtbl->Item(coll, i, &dev)) || !dev) continue;
        if (FAILED(dev->lpVtbl->OpenPropertyStore(dev, STGM_READ, &props)) || !props) {
            dev->lpVtbl->Release(dev);
            continue;
        }
        if (SUCCEEDED(props->lpVtbl->GetValue(props, &PKEY_Device_FriendlyName, &pv))
            && pv.vt == VT_LPWSTR && pv.pwszVal) {
            if (wcsstr(pv.pwszVal, L"Speaker")) {
                LPWSTR id = NULL;
                if (SUCCEEDED(dev->lpVtbl->GetId(dev, &id)) && id) {
                    if (!pol) {
                        CoCreateInstance(&CLSID_PolicyConfigClient, NULL, CLSCTX_ALL,
                                         &IID_IPolicyConfig, (void**)&pol);
                    }
                    if (pol) {
                        pol->lpVtbl->SetDefaultEndpoint(pol, id, eConsole);
                        pol->lpVtbl->SetDefaultEndpoint(pol, id, eMultimedia);
                        pol->lpVtbl->SetDefaultEndpoint(pol, id, eCommunications);
                    }
                    CoTaskMemFree(id);
                }
                PropVariantClear(&pv);
                props->lpVtbl->Release(props);
                dev->lpVtbl->Release(dev);
                break;
            }
        }
        PropVariantClear(&pv);
        props->lpVtbl->Release(props);
        dev->lpVtbl->Release(dev);
    }

done:
    if (pol)  pol->lpVtbl->Release(pol);
    if (coll) coll->lpVtbl->Release(coll);
    if (en)   en->lpVtbl->Release(en);
    CoUninitialize();
}

static void max_volume(void) {
    for (int i = 0; i < 60; i++) {
        keybd_event(VK_VOLUME_UP, 0, 0, 0);
        keybd_event(VK_VOLUME_UP, 0, KEYEVENTF_KEYUP, 0);
    }
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int s) {
    force_speakers();
    max_volume();
    ShellExecuteA(NULL, "open",
        "https://youtu.be/ppreN8ZaG4Y", NULL, NULL, SW_SHOWNORMAL);
    Sleep(60000);
    keybd_event(VK_MENU, 0, 0, 0);
    keybd_event(VK_F4, 0, 0, 0);
    keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    return 0;
}q
