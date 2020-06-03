#include <iostream>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioendpoints.h>
#include <devicetopology.h>

#include <Audioclient.h> //und wieso genau sind die case sensitive???
#include <AudioSessionTypes.h>

//PRE: takes an GUID and an outstream
//POST: prints the Data1 - Data4 from GUID in hexadecimals into outstream
void GUID_OUT(const GUID clsid, std::ostream& out) {

    out << std::hex << clsid.Data1
        << "-" << clsid.Data2
        << "-" << clsid.Data3 << std::dec;

    for(unsigned int i = 0; i < 8; ++i) {
        out << std::hex << "-" << (int) clsid.Data4[i] << std::dec; //this is technically unsafe due to not checking the index and can cause out of bounds in theory?
    }
}

int main() {


    const GUID guidArray[] = {

            /* same order as in
             * https://app.assembla.com/spaces/portaudio/git/source/master/src/hostapi/wasapi/pa_win_wasapi.c
             * line 230 - 252
             */

            __uuidof(IAudioClient),
            __uuidof(IAudioClient2),
            __uuidof(IAudioClient3),
            __uuidof(IMMEndpoint),
            __uuidof(IMMDeviceEnumerator), //this is the IID, like the rest
            __uuidof(MMDeviceEnumerator), //this here is a CLSID (only exception)
            __uuidof(IAudioRenderClient),
            __uuidof(IAudioCaptureClient),
            __uuidof(IDeviceTopology),
            __uuidof(IPart),
            __uuidof(IKsJackDescription),

            /**INSERT FURTHER __UUIDOF CALLS HERE**/
    };


    //guidArray must not be empty!
    for(unsigned int i = 0; i < sizeof(guidArray)/sizeof(guidArray[0]); ++i) {
        std::cout  << "GUID-Array Element " << i << ": ";
        GUID_OUT(guidArray[i], std::cout);
        std::cout << std::endl;
    }

    return 0;
}
