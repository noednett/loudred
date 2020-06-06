#include <stdio.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <stringapiset.h>
#include <ksmedia.h> //macro for conversion between subformat GUIDs
#include <WcnFunctionDiscoveryKeys.h>

//--------------------------------------------------------------------------------------------------------------------
#ifndef GUID_SECT
#define GUID_SECT
#endif
#define __DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __DEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define PA_DEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __DEFINE_CLSID(pa_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8) //we should probably remove the pa_ tags at some point
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

//----------------------------------------------------------------------------------------------------------------------

//defining macro to 'escape' functions on error

#define EXIT_ON_ERROR(hres) \
            if(FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(prisoner) \
            if((prisoner) != NULL) \
            {(prisoner)->lpVtbl->Release(prisoner); prisoner = NULL; }

#define REFTIMES_PER_SEC  10000000 //
#define REFTIMES_PER_MILLISEC 10000

//DECLARATIONS (this belongs into a header file)
//----------------------------------------------------------------------------------------------------------------------
HRESULT getDefaultEndpoint(IMMDevice**, const EDataFlow);
HRESULT initializeCapture(IMMDevice*, IAudioClient**, IAudioCaptureClient**, WAVEFORMATEX**, const REFERENCE_TIME, UINT32*);
HRESULT initializeRender(IMMDevice*, IAudioClient**, IAudioRenderClient**, WAVEFORMATEX**, const REFERENCE_TIME, UINT32*);
HRESULT getCaptureData(IAudioCaptureClient*, UINT32*, DWORD*, BYTE**);
HRESULT readCaptureData(BYTE const *,  UINT32 const * const , WAVEFORMATEX const * const, BYTE**);

//debug and testing
void GUID_OUT(GUID const * const);
//----------------------------------------------------------------------------------------------------------------------


//POST return HRESULT, **ppDevice is set to the default specified capture endpoint.
HRESULT getDefaultEndpoint(IMMDevice **ppDevice, const EDataFlow dataFlow) { //reference to pointers are only available in cpp, not in c.

    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pEnumerator = NULL;

    hr = CoCreateInstance(&pa_CLSID_IMMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &pa_IID_IMMDeviceEnumerator, (void **) &pEnumerator);
    EXIT_ON_ERROR(hr);

    //we retrieve the default audio endpoint

    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, dataFlow, eConsole, ppDevice);
    EXIT_ON_ERROR(hr); //for consistency, but not needed here.

    Exit:
        SAFE_RELEASE(pEnumerator);

    return hr;
}

HRESULT initializeCapture(

        IMMDevice *pDevice,
        IAudioClient **ppAudioClient,
        IAudioCaptureClient **ppCaptureClient,
        WAVEFORMATEX **ppWaveFormat,
        const REFERENCE_TIME hnsRequestedDuration,
        UINT32 *bufferFrameCount)   {

    HRESULT hr = S_OK;

    hr = pDevice->lpVtbl->Activate(pDevice, &pa_IID_IAudioClient, CLSCTX_ALL, NULL, (void**) ppAudioClient);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->GetMixFormat(*ppAudioClient, ppWaveFormat);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->Initialize(*ppAudioClient, AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, *ppWaveFormat, NULL);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->GetBufferSize(*ppAudioClient, bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->GetService(*ppAudioClient, &pa_IID_IAudioCaptureClient, (void**) ppCaptureClient);
    EXIT_ON_ERROR(hr)

    Exit:

    return hr;
}

//PRE: initialized CaptureClient, pCaptureClient->GetNextPacketSize != 0
//POST: BUFFER HAS TO BE RELEASED!!
HRESULT getCaptureData(IAudioCaptureClient *pCaptureClient, UINT32 *pNumFramesAvailable, DWORD *flags, BYTE **ppData) {

    HRESULT hr = S_OK;

    hr = pCaptureClient->lpVtbl->GetBuffer(pCaptureClient,ppData, pNumFramesAvailable, flags, NULL, NULL);
    EXIT_ON_ERROR(hr)

    if(*flags & AUDCLNT_BUFFERFLAGS_SILENT) {
        *ppData = NULL;
    }

    Exit:

    return hr;
}

//POST: copies the shared buffer into a Sink buffer.
HRESULT readCaptureData(BYTE const *pData,  UINT32 const * const pNumFramesAvailable, WAVEFORMATEX const * const waveFormat, BYTE **ppSink) {

    HRESULT hr = S_OK;

    //calculating the amount of bytes in the array:
    //Amount of samples per frame
    //DWORD smpPerFrame = waveFormat->nBlockAlign;

    /*
     * if the wave the waveFormat tag is WAVE_FORMAT_PCM, each sample consists of either 8 or 16 bits (usually). then cbSize = 0.
     * if the waveFormat tag is WAVE_FORMAT_EXTENSIBLE, the call ->wBitsPerSample can return anything from 0, 22 or higher. this that the
     * returned value does not actually have to be the BitsPerSample size, instead additional information is appended to the waveFormat.
     * source: https://docs.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex
     */

    //DWORD bitsPerSample = 0; //we throw an error if this amount is still 0 at the end. (otherwise this wont make any sense)

    if(waveFormat->wFormatTag == WAVE_FORMAT_PCM) {

        printf("PCM, not yet supported");

        //Bits per sample
        //DWORD bitsPerSample = waveFormat->wBitsPerSample;
    } else if(waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {

        /* we can safely convert WAVEFORMATEX to WAVEFORMATEXTENSIBLE
         * https://docs.microsoft.com/en-us/windows-hardware/drivers/audio/converting-between-format-tags-and-subformat-guids
         */

        //magic
        WAVEFORMATEXTENSIBLE const * const waveFormatExt = waveFormat;

        //printf("valid bits per sample: %d\nreserved bits per sample: %d\n", waveFormatExt->Samples.wValidBitsPerSample, waveFormatExt->Samples.wReserved);
        //printf("actual bits per sample: %d\n", waveFormat->wBitsPerSample);
        //GUID_OUT(&waveFormatExt->SubFormat); //this is the test that really confirms this: https://docs.microsoft.com/en-us/windows/win32/directshow/audio-subtypes

        /*  For magic adepts: this cast works due to the way structs are allocated in the memory. the members of a struct
         *  are all aligned in the memory. this means, that the representation of the WAVEFORMATEX can be depicted as
         *
         *     pointer -> [header| DWORD wFormatTag| ... | WORD cbSize]
         *  now if WAVEFORMATEX has the tag EXTENSIBLE, there is more data allocated directly behind the struct.
         *
         *  pointer -> [header| DWORD wFormatTag| ... | WORD cbSize][DWORD dwChannelMask| ... ]
         *
         *  by casting WAVEFORMEXTENSIBLE to WAVEFORMATEX, we give permission to also read the extra memory behind the
         *  WAVEFORMATEX.
         *
         *  the danger here is, that as far as i know, we shouldn't copy WAVEFORMATEX if we plan on maybe casting it to WAVEFORMATEXTENSIBLE later
         *  but instead use the pointer, since we do not want too lose the memory behind it.
         *  (it could still work if the copyconstructor is configured correctly, but i dont think this is the case in C but maybe in C++)
         */


        //this is a hack due to us both having pcm inputs with 32 bits per sample:
        DWORD bytesPerSample = waveFormatExt->Samples.wValidBitsPerSample/8; //this here should be an integer division as far as i know.

        const unsigned long numBytesToRead = bytesPerSample * waveFormat->nBlockAlign * *pNumFramesAvailable;
        //printf("numFramesToRead: %d\n", *pNumFramesAvailable); //confusing since both of these values dont seem to change, even if you change the recording duration.
        //printf("numBytesToRead: %d", numBytesToRead);

        (*ppSink) = malloc(numBytesToRead);


        for(unsigned long i = 0; i < numBytesToRead; ++i) {
            (*ppSink)[i] = pData[i];
            printf("%x ", pData[i]);
        }

    } else {
        hr = E_INVALIDARG; //what ever bullshit the wave format is, its not supported.
        EXIT_ON_ERROR(hr)
    }


    Exit:

    return hr;
}



HRESULT initializeRender(IMMDevice *pDevice, IAudioClient **ppAudioClient, IAudioRenderClient **ppRenderClient, WAVEFORMATEX **ppWaveFormat, const REFERENCE_TIME hnsRequestedDuration, UINT32 *bufferFrameCount) {

    HRESULT hr = S_OK;

    hr = pDevice->lpVtbl->Activate(pDevice, &pa_IID_IAudioClient, CLSCTX_ALL, NULL, (void**) ppAudioClient);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->GetMixFormat(*ppAudioClient, *ppWaveFormat);
    EXIT_ON_ERROR(hr)

    hr = (*ppAudioClient)->lpVtbl->Initialize(*ppAudioClient, AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, *ppWaveFormat, NULL);
    EXIT_ON_ERROR(hr)

    Exit:

    return hr;
}


//PRE: takes an GUID and an outstream
//POST: prints the Data1 - Data4 from GUID in hexadecimals into outstream
void GUID_OUT(GUID const * const clsid) {

    printf("%08lx - %04x - %04x - %02x - %02x - %02x - %02x - %02x - %02x - %02x - %02x",
            clsid->Data1, clsid->Data2, clsid->Data3,
            clsid->Data4[0], clsid->Data4[1], clsid->Data4[2], clsid->Data4[3], clsid->Data4[4], clsid->Data4[5], clsid->Data4[6],clsid->Data4[7]);

}



int main (int argc, const char * argv[]) {

    CoInitialize(0);

    HRESULT hr = S_OK;

    IMMDevice *pCaptureDevice = NULL;

    hr = getDefaultEndpoint(&pCaptureDevice, eCapture);
    EXIT_ON_ERROR(hr)

    IAudioClient *pAudioCaptureClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    WAVEFORMATEX *pCaptureWaveFormat = NULL;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    UINT32 bufferFrameCount;

    hr = initializeCapture(pCaptureDevice, &pAudioCaptureClient, &pCaptureClient, &pCaptureWaveFormat, hnsRequestedDuration, &bufferFrameCount);
    EXIT_ON_ERROR(hr)
    hr = pAudioCaptureClient->lpVtbl->Start(pAudioCaptureClient);
    EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    UINT32 hnsActualDuration = (double)REFTIMES_PER_SEC *
                        bufferFrameCount / pCaptureWaveFormat->nSamplesPerSec;

    Sleep(hnsActualDuration/REFTIMES_PER_MILLISEC/2);

    UINT32 numFramesAvailable;
    DWORD flags;
    BYTE *pData;
    BYTE *sink = NULL;

    hr = getCaptureData(pCaptureClient, &numFramesAvailable, &flags, &pData);
    EXIT_ON_ERROR(hr)
    hr = readCaptureData(pData, &numFramesAvailable, pCaptureWaveFormat, &sink);
    EXIT_ON_ERROR(hr);

    hr = pCaptureClient->lpVtbl->ReleaseBuffer(pCaptureClient, numFramesAvailable);
    EXIT_ON_ERROR(hr)

    hr = pAudioCaptureClient->lpVtbl->Stop(pAudioCaptureClient);
    EXIT_ON_ERROR(hr)

    SAFE_RELEASE(pCaptureDevice);

    //PLAYBACK
    IMMDevice *pRenderDevice = NULL;
    IAudioClient *pAudioRenderClient = NULL;
    IAudioRenderClient *pRenderClient = NULL; //nur liecht irrefüehrendi näme...
    WAVEFORMATEX *pRenderWaveFormat = NULL;

    hr = getDefaultEndpoint(&pRenderDevice, eRender);
    EXIT_ON_ERROR(hr)

    hr = initializeRender(pRenderDevice, &pAudioRenderClient, &pRenderClient, &pRenderWaveFormat, hnsRequestedDuration, &bufferFrameCount)



    return 0;

    Exit:
        printf("%04x\n", hr);
        CoTaskMemFree(pCaptureWaveFormat);
        CoTaskMemFree(pRenderWaveFormat);
        free(sink);
        SAFE_RELEASE(pCaptureDevice)
        SAFE_RELEASE(pAudioCaptureClient)
        SAFE_RELEASE(pCaptureClient)
        SAFE_RELEASE(pRenderDevice)
        SAFE_RELEASE(pAudioRenderClient)
        SAFE_RELEASE(pRenderClient)

        return 1;
}
