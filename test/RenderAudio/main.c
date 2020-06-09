#include <stdio.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

#define EXIT_ON_ERROR(hres, errorText) \
    if (FAILED(hres)) {printf("Error: %40x, %s\n", hres, errorText); goto Exit;}

#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL)  \
        { (punk)->lpVtbl->Release(punk); (punk) = NULL; }

#ifndef GUID_SECT
#define GUID_SECT
#endif
#define __DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define PA_DEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __DEFINE_CLSID(pa_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define PA_DEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __DEFINE_IID(pa_IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
// "1CB9AD4C-DBFA-4c32-B178-C2F568A703B2"
PA_DEFINE_IID(IAudioClient,         1cb9ad4c, dbfa, 4c32, b1, 78, c2, f5, 68, a7, 03, b2);
// "726778CD-F60A-4EDA-82DE-E47610CD78AA"
PA_DEFINE_IID(IAudioClient2,        726778cd, f60a, 4eda, 82, de, e4, 76, 10, cd, 78, aa);
// "7ED4EE07-8E67-4CD4-8C1A-2B7A5987AD42"
PA_DEFINE_IID(IAudioClient3,        7ed4ee07, 8e67, 4cd4, 8c, 1a, 2b, 7a, 59, 87, ad, 42);
// "1BE09788-6894-4089-8586-9A2A6C265AC5"
PA_DEFINE_IID(IMMEndpoint,          1be09788, 6894, 4089, 85, 86, 9a, 2a, 6c, 26, 5a, c5);
// "A95664D2-9614-4F35-A746-DE8DB63617E6"
PA_DEFINE_IID(IMMDeviceEnumerator,  a95664d2, 9614, 4f35, a7, 46, de, 8d, b6, 36, 17, e6);
// "BCDE0395-E52F-467C-8E3D-C4579291692E"
PA_DEFINE_CLSID(IMMDeviceEnumerator,bcde0395, e52f, 467c, 8e, 3d, c4, 57, 92, 91, 69, 2e);
// "F294ACFC-3146-4483-A7BF-ADDCA7C260E2"
PA_DEFINE_IID(IAudioRenderClient,   f294acfc, 3146, 4483, a7, bf, ad, dc, a7, c2, 60, e2);
// "C8ADBD64-E71E-48a0-A4DE-185C395CD317"
PA_DEFINE_IID(IAudioCaptureClient,  c8adbd64, e71e, 48a0, a4, de, 18, 5c, 39, 5c, d3, 17);
// *2A07407E-6497-4A18-9787-32F79BD0D98F*  Or this??
PA_DEFINE_IID(IDeviceTopology,      2A07407E, 6497, 4A18, 97, 87, 32, f7, 9b, d0, d9, 8f);
// *AE2DE0E4-5BCA-4F2D-AA46-5D13F8FDB3A9*
PA_DEFINE_IID(IPart,                AE2DE0E4, 5BCA, 4F2D, aa, 46, 5d, 13, f8, fd, b3, a9);
// *4509F757-2D46-4637-8E62-CE7DB944F57B*
PA_DEFINE_IID(IKsJackDescription,   4509F757, 2D46, 4637, 8e, 62, ce, 7d, b9, 44, f5, 7b);

HRESULT getDeviceEnumerator(IMMDeviceEnumerator**);
HRESULT getDefaultAudioEndpoint(IMMDeviceEnumerator*, EDataFlow, IMMDevice**);
HRESULT activateDevice(IMMDevice*, IAudioClient**);
HRESULT initializeAudioClient(IAudioClient*, REFERENCE_TIME, WAVEFORMATEX*);
HRESULT getAudioRenderClient(IAudioClient *, IAudioRenderClient**);
void loadData(FILE* , UINT32, BYTE *, WAVEFORMATEX *, DWORD*);


int main() {
    FILE *input;
    //TODO: SET PATH
    input = fopen("C:\\Users\\user\\Desktop\\audioOutput.txt", "rb");

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;

    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pRenderDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;

    WAVEFORMATEX *pRenderWaveFormat = NULL;

    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE *pData;
    DWORD flags = 0;

    hr = getDeviceEnumerator(&pEnumerator);
    EXIT_ON_ERROR(hr, "getDeviceEnumerator")

    hr = getDefaultAudioEndpoint(pEnumerator, eRender, &pRenderDevice);
    EXIT_ON_ERROR(hr, "getDefaultAudioEndpoint")

    hr = activateDevice(pRenderDevice, &pAudioClient);
    EXIT_ON_ERROR(hr, "activate Device")

    hr = pAudioClient->lpVtbl->GetMixFormat(pAudioClient, &pRenderWaveFormat);
    EXIT_ON_ERROR(hr, "GetMixFormat")

    hr = initializeAudioClient(pAudioClient, hnsRequestedDuration, pRenderWaveFormat);
    EXIT_ON_ERROR(hr, "  initializeAudioClient")

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->lpVtbl->GetBufferSize(pAudioClient, &bufferFrameCount);
    EXIT_ON_ERROR(hr, "GetBufferSize")

    hr = getAudioRenderClient(pAudioClient, &pRenderClient);
    EXIT_ON_ERROR(hr, "getAudioRenderClient")

    // Grab the entire buffer for the initial fill operation.
    hr = pRenderClient->lpVtbl->GetBuffer(pRenderClient, bufferFrameCount, &pData);
    EXIT_ON_ERROR(hr, "GetBuffer #1")

    loadData(input, bufferFrameCount, pData,pRenderWaveFormat, &flags);

    hr = pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient, bufferFrameCount, flags);
    EXIT_ON_ERROR(hr, "ReleaseBuffer #1")

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
                        bufferFrameCount / pRenderWaveFormat->nSamplesPerSec;

    hr = pAudioClient->lpVtbl->Start(pAudioClient);  // Start playing.
    EXIT_ON_ERROR(hr, "start")

    while(flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        // Sleep for half the buffer duration.
        Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

        // See how much buffer space is available.
        hr = pAudioClient->lpVtbl->GetCurrentPadding(pAudioClient, &numFramesPadding);
        EXIT_ON_ERROR(hr, "GetCurrentPadding");

        numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->lpVtbl->GetBuffer(pRenderClient, numFramesAvailable, &pData);
        EXIT_ON_ERROR(hr, "GetBuffer #2")

        // Get next 1/2-second of data from the audio source.
        loadData(input, numFramesAvailable, pData,pRenderWaveFormat, &flags);

        hr = pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient, numFramesAvailable, flags);
        EXIT_ON_ERROR(hr, "ReleaseBuffer #2");

    }

    // We don't need this, We make one extra call to loadData because of how we check for end of file
    // Wait for last data in buffer to play before stopping.
    //Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

    hr = pAudioClient->lpVtbl->Stop(pAudioClient);  // Stop playing.
    EXIT_ON_ERROR(hr, "stop")




Exit:
    CoTaskMemFree(pRenderWaveFormat);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pRenderDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pRenderClient)

    CoUninitialize();
    fclose(input);

    return 0;
}

HRESULT getDeviceEnumerator(IMMDeviceEnumerator** ppEnumerator){
    CoInitialize(0);
    return CoCreateInstance(&pa_CLSID_IMMDeviceEnumerator, NULL, CLSCTX_ALL, &pa_IID_IMMDeviceEnumerator, (void**)ppEnumerator);
}

HRESULT getDefaultAudioEndpoint(IMMDeviceEnumerator* pEnumerator, EDataFlow dataFlow, IMMDevice ** ppDevice){
    return pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, dataFlow, eConsole, ppDevice);
}

HRESULT activateDevice(IMMDevice* pDevice, IAudioClient** ppAudioClient){
    return pDevice->lpVtbl->Activate(pDevice, &pa_IID_IAudioClient,  CLSCTX_ALL, NULL, (void**) ppAudioClient);
}

HRESULT initializeAudioClient(IAudioClient * pAudioClient, REFERENCE_TIME hnsRequestedDuration, WAVEFORMATEX *pwfx){
    return pAudioClient->lpVtbl->Initialize(pAudioClient, AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
}

HRESULT getAudioRenderClient(IAudioClient * pAudioClient, IAudioRenderClient **ppRenderClient){
    return pAudioClient->lpVtbl->GetService(pAudioClient, &pa_IID_IAudioRenderClient, (void**) ppRenderClient);
}

void loadData(FILE * input, UINT32 numFramesAvailable, BYTE *pData, WAVEFORMATEX * waveFormat, DWORD* flags)
{

    WORD wBlockAlign = waveFormat->nBlockAlign;

    const unsigned long numBytesToRead = wBlockAlign * numFramesAvailable;

    int numberOfBytesRead = fread(pData, 1, numBytesToRead, input);
    if( 0 >= numberOfBytesRead){
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    }
}
