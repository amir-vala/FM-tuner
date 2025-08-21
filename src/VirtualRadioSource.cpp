#include "VirtualRadioSource.h"
#include <shlwapi.h>
#include <fstream>
#include <cmath>        // fabs
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// BaseClasses helpers
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
extern HINSTANCE g_hInst; // from dllmain.cpp

// ---- AMOVIESETUP (in same TU as g_Templates so registration writes pins) ----
static AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM
};

static AMOVIESETUP_PIN sudPins[] =
{
    { L"Out", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 1, &sudPinTypes }
};

static AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_VirtualRadioSource,
    L"Virtual Radio Source",
    (DWORD)(MERIT_DO_NOT_USE + 1),
    1, sudPins
};

CFactoryTemplate g_Templates[] =
{
    { L"Virtual Radio Source", &CLSID_VirtualRadioSource, VirtualRadioSource::CreateInstance, NULL, &sudFilter }
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

// ---------------- VirtualRadioSource ----------------
CUnknown* WINAPI VirtualRadioSource::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    auto p = new VirtualRadioSource(pUnk, phr);
    if (!p && phr) *phr = E_OUTOFMEMORY;
    return p;
}

VirtualRadioSource::VirtualRadioSource(LPUNKNOWN pUnk, HRESULT* phr)
    : CSource(L"Virtual Radio Source", pUnk, CLSID_VirtualRadioSource)
{
    CAutoLock cObjectLock(m_pLock);
    stream_ = new VirtualRadioStream(phr, this);
}

VirtualRadioSource::~VirtualRadioSource()
{
    delete stream_;
    stream_ = nullptr;
}

STDMETHODIMP VirtualRadioSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IVirtualRadioControl) return GetInterface((IVirtualRadioControl*)this, ppv);
    return CSource::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT VirtualRadioSource::SetFrequency(double mhz)
{
    // load stations.json next to DLL
    WCHAR path[MAX_PATH]; 
    if (!GetModuleFileNameW(g_hInst, path, MAX_PATH)) return E_FAIL;
    PathRemoveFileSpecW(path);
    std::wstring jsonPath = std::wstring(path) + L"\\stations.json";

    std::ifstream f(jsonPath);
    if (!f) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    json j; 
    try { f >> j; } catch (...) { return E_FAIL; }

    std::wstring url;
    for (auto& it : j)
    {
        if (!it.contains("freqMHz") || !it.contains("url")) continue;
        double fr = 0.0;
        try { fr = it["freqMHz"].get<double>(); }
        catch (...) { continue; }

        if (std::fabs(fr - mhz) < 1e-3)
        {
            try {
                std::string u = it["url"].get<std::string>();
                url.assign(u.begin(), u.end());
                break;
            } catch (...) { /* ignore and continue */ }
        }
    }
    if (url.empty()) return E_INVALIDARG;

    setUrl(url);
    return S_OK;
}

HRESULT VirtualRadioSource::SetDirectUrl(BSTR url)
{
    setUrl(std::wstring(url, SysStringLen(url)));
    return S_OK;
}

void VirtualRadioSource::setUrl(const std::wstring& u)
{
    CAutoLock cObjectLock(m_pLock);
    url_ = u;
    if (stream_) stream_->SetUrl(u);
}

// ---------------- VirtualRadioStream ----------------
VirtualRadioStream::VirtualRadioStream(HRESULT* phr, VirtualRadioSource* pParent)
    : CSourceStream(L"VirtualRadioStream", phr, pParent, L"Out")
{
    fmt_.sampleRate   = 48000;
    fmt_.channels     = 2;
    fmt_.bitsPerSample= 16;
}

VirtualRadioStream::~VirtualRadioStream()
{
    // ensure decoder closed even if graph forgot
    dec_.close();
}

void VirtualRadioStream::SetUrl(const std::wstring& url)
{
    url_ = url;
    // if graph is active, reopen immediately; otherwise, leave for Active()
    FILTER_STATE state = State_Running;
    m_pFilter->GetState(0, &state);
    if (state == State_Running || state == State_Paused)
    {
        dec_.close();
        dec_.open(url_, fmt_);
        rtNext_ = 0;
    }
}

HRESULT VirtualRadioStream::Active()
{
    // Called on Run/Paused. Start decoder if URL already set.
    HRESULT hr = CSourceStream::Active();
    if (FAILED(hr)) return hr;

    if (!url_.empty() && !dec_.isRunning())
    {
        dec_.open(url_, fmt_);
    }
    rtNext_ = 0;
    return S_OK;
}

HRESULT VirtualRadioStream::Inactive()
{
    // Called on Stop. Close decoder thread so module can unload cleanly.
    dec_.close();
    return CSourceStream::Inactive();
}

HRESULT VirtualRadioStream::GetMediaType(CMediaType* pmt)
{
    WAVEFORMATEX wfx{};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = (WORD)fmt_.channels;
    wfx.nSamplesPerSec  = fmt_.sampleRate;
    wfx.wBitsPerSample  = (WORD)fmt_.bitsPerSample;
    wfx.nBlockAlign     = (WORD)(wfx.nChannels * wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Audio);
    pmt->SetSubtype(&MEDIASUBTYPE_PCM);
    pmt->SetFormatType(&FORMAT_WaveFormatEx);
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSampleSize(wfx.nBlockAlign);

    // Copy format blob into the media type
    pmt->SetFormat((BYTE*)&wfx, sizeof(WAVEFORMATEX));
    return S_OK;
}

HRESULT VirtualRadioStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pprop)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    const LONG block = (fmt_.channels * (fmt_.bitsPerSample/8));
    const LONG bytesPerSec = fmt_.sampleRate * block;

    pprop->cbBuffer = bytesPerSec / 10; // 100ms chunk
    pprop->cBuffers = 8;

    ALLOCATOR_PROPERTIES actual{};
    HRESULT hr = pAlloc->SetProperties(pprop, &actual);
    if (FAILED(hr)) return hr;
    if (actual.cbBuffer < pprop->cbBuffer) return E_FAIL;
    return S_OK;
}

HRESULT VirtualRadioStream::FillBuffer(IMediaSample* pS)
{
    BYTE* pData = nullptr;
    long cb = 0;
    if (FAILED(pS->GetPointer(&pData))) return E_POINTER;
    cb = pS->GetSize();

    size_t got = 0;
    while (got < (size_t)cb)
    {
        size_t n = dec_.readPcm(pData + got, cb - (long)got);
        if (n == 0)
        {
            // Avoid busy loop if decoder not yet fed
            Sleep(5);
            continue;
        }
        got += n;
    }
    pS->SetActualDataLength((long)got);

    // timestamps (100ns units)
    REFERENCE_TIME rtStart = rtNext_;
    const double bytesPerSec = (double)fmt_.sampleRate * fmt_.channels * (fmt_.bitsPerSample/8);
    REFERENCE_TIME rtDur = (REFERENCE_TIME)((10000000.0 * got) / bytesPerSec);
    REFERENCE_TIME rtStop = rtStart + rtDur;

    pS->SetTime(&rtStart, &rtStop);
    pS->SetSyncPoint(TRUE);
    rtNext_ = rtStop;

    return S_OK;
}
