#include <stdio.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <audioclient.h>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

#define EXIT_ON_ERROR(hres) \
    if (FAILED(hres)) {printf("%40x\n", hres); goto Exit;}

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

HRESULT  copyData(BYTE *pData, UINT32 numFramesAvailable, WAVEFORMATEX *waveFormat);

int main() {

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pwfx = NULL;
    BYTE *pData;
    DWORD flags;

    CoInitialize(NULL);
    hr = CoCreateInstance(&pa_CLSID_IMMDeviceEnumerator, NULL, CLSCTX_ALL, &pa_IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator,eCapture,eConsole,&pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->lpVtbl->Activate(pDevice, &pa_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->lpVtbl->GetMixFormat(pAudioClient, &pwfx);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->lpVtbl->Initialize(pAudioClient, AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->lpVtbl->GetBufferSize(pAudioClient, &bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->lpVtbl->GetService(pAudioClient, &pa_IID_IAudioCaptureClient, (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->lpVtbl->Start(pAudioClient);
    EXIT_ON_ERROR(hr)

    // Sleep for half the buffer duration.
    Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);

    hr = pCaptureClient->lpVtbl->GetBuffer(pCaptureClient,&pData, &numFramesAvailable, &flags, NULL, NULL);
    EXIT_ON_ERROR(hr)

    if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
    {
        pData = NULL;
    }

    hr = copyData(pData, numFramesAvailable, pwfx);
    EXIT_ON_ERROR(hr)

    hr = pCaptureClient->lpVtbl->ReleaseBuffer(pCaptureClient, numFramesAvailable);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->lpVtbl->Stop(pAudioClient);
    EXIT_ON_ERROR(hr)

    Exit:
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pCaptureClient)
        return 0;
}

HRESULT copyData(BYTE *pData, UINT32 numFramesAvailable, WAVEFORMATEX *waveFormat)
{
    HRESULT hr = S_OK;

    WAVEFORMATEXTENSIBLE const * const waveFormatExtensible = waveFormat;

    // Frame count (the number of audio frames available in the data packet)
    printf("Number of frames available: %d\n", numFramesAvailable);

    WORD wSamplesPerBlock = waveFormatExtensible->Samples.wSamplesPerBlock;
    // Number of samples contained in one compressed block of audio data. This value is used with
    // compressed formats that have a fixed number of samples within each block.
    // This value can be set to 0 if a variable number of samples is contained in each block of compressed audio data.
    printf("Samples per block: %d\n", wSamplesPerBlock);
    if(wSamplesPerBlock == 0)
    {
        //TODO: Calculate wBitsPerSample
    }

    WORD wBlockAlign = waveFormat->nBlockAlign;
    // The block alignment is the minimum atomic unit of data for the audio format.
    // For PCM audio formats, the block alignment is equal to the number of audio channels
    // multiplied by the number of bytes per audio sample.
    printf("Block alignment: %d\n", wBlockAlign);

    WORD wBitsPerSample = waveFormatExtensible->Format.wBitsPerSample;
    // The total size of each audio sample, including padding bits
    printf("Bits per sample: %d\n", wBitsPerSample);

    WORD wValidBitsPerSample = waveFormatExtensible->Samples.wValidBitsPerSample;
    // If an audio format does not contain any padding bits, then this attribute should not be set,
    // or should be set to the same value as the MF_MT_AUDIO_BITS_PER_SAMPLE attribute.
    printf("Valid bits per sample: %d\n", wValidBitsPerSample);

    WORD wReserved = waveFormatExtensible->Samples.wReserved;
    // Reserved for internal use by operating system. Set to 0.
    printf("Reserved bits per sample: %d\n", wReserved);

    const short byteSize = 8;
    UINT32 ByteSize = (numFramesAvailable*wSamplesPerBlock*wValidBitsPerSample)/byteSize;
    printf("ByteSize: %d\n", ByteSize);

    for (int i = 0; i < ByteSize; i++){
        printf("%hhx ", (pData[i]));
    }

    Exit:

    return hr;
}
