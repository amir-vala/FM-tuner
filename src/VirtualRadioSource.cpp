#include "VirtualRadioSource.h"
#include <shlwapi.h>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// BaseClasses helpers
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

CFactoryTemplate g_Templates[] = {
    { L"Virtual Radio Source", &CLSID_VirtualRadioSource, VirtualRadioSource::CreateInstance, NULL, NULL }
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

// --------------- VirtualRadioSource ---------------
CUnknown* WINAPI VirtualRadioSource::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr) {
    auto p = new VirtualRadioSource(pUnk, phr);
    if (!p && phr) *phr = E_OUTOFMEMORY;
    return p;
}

VirtualRadioSource::VirtualRadioSource(LPUNKNOWN pUnk, HRESULT* phr)
    : CSource(L"Virtual Radio Source", pUnk, CLSID_VirtualRadioSource) {
    CAutoLock cObjectLock(m_pLock);
    stream_ = new VirtualRadioStream(phr, this);
}

VirtualRadioSource::~VirtualRadioSource() {
    delete stream_;
}

STDMETHODIMP VirtualRadioSource::NonDelegatingQueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IVirtualRadioControl) return GetInterface((IVirtualRadioControl*)this, ppv);
    return CSource::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT VirtualRadioSource::SetFrequency(double mhz) {
    // load stations.json next to DLL
    WCHAR path[MAX_PATH]; GetModuleFileNameW(g_hInst, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    std::wstring jsonPath = std::wstring(path) + L"\\stations.json";
    std::ifstream f(jsonPath);
    if (!f) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    json j; f >> j;
    std::wstring url;
    for (auto& it : j) {
        double fr = it["freqMHz"].get<double>();
        if (fabs(fr - mhz) < 1e-3) { std::string u = it["url"].get<std::string>(); url.assign(u.begin(), u.end()); break; }
    }
    if (url.empty()) return E_INVALIDARG;
    setUrl(url);
    return S_OK;
}

HRESULT VirtualRadioSource::SetDirectUrl(BSTR url) {
    setUrl(std::wstring(url, SysStringLen(url)));
    return S_OK;
}

void VirtualRadioSource::setUrl(const std::wstring& u) {
    CAutoLock cObjectLock(m_pLock);
    url_ = u;
    if (stream_) stream_->SetUrl(u);
}

// --------------- VirtualRadioStream ---------------
VirtualRadioStream::VirtualRadioStream(HRESULT* phr, VirtualRadioSource* pParent)
    : CSourceStream(L"VirtualRadioStream", phr, pParent, L"Out") {
    fmt_.sampleRate = 48000; fmt_.channels = 2; fmt_.bitsPerSample = 16;
}

VirtualRadioStream::~VirtualRadioStream() {}

void VirtualRadioStream::SetUrl(const std::wstring& url) {
    dec_.open(url, fmt_);
    rtNext_ = 0;
}

HRESULT VirtualRadioStream::GetMediaType(CMediaType* pmt) {
    WAVEFORMATEX* pwf = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));
    ZeroMemory(pwf, sizeof(WAVEFORMATEX));
    pwf->wFormatTag = WAVE_FORMAT_PCM;
    pwf->nChannels = (WORD)fmt_.channels;
    pwf->nSamplesPerSec = fmt_.sampleRate;
    pwf->wBitsPerSample = (WORD)fmt_.bitsPerSample;
    pwf->nBlockAlign = pwf->nChannels * pwf->wBitsPerSample / 8;
    pwf->nAvgBytesPerSec = pwf->nSamplesPerSec * pwf->nBlockAlign;

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Audio);
    pmt->SetSubtype(&MEDIASUBTYPE_PCM);
    pmt->SetFormatType(&FORMAT_WaveFormatEx);
    pmt->SetFormat((BYTE*)pwf, sizeof(WAVEFORMATEX));
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSampleSize(pwf->nBlockAlign);
    CoTaskMemFree(pwf);
    return S_OK;
}

HRESULT VirtualRadioStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pprop) {
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    WAVEFORMATEX wfx{}; wfx.nSamplesPerSec = fmt_.sampleRate; wfx.nChannels=fmt_.channels; wfx.wBitsPerSample=fmt_.bitsPerSample; wfx.nBlockAlign=wfx.nChannels*wfx.wBitsPerSample/8;
    pprop->cbBuffer = wfx.nBlockAlign * (wfx.nSamplesPerSec/10); // 100ms
    pprop->cBuffers = 8;
    ALLOCATOR_PROPERTIES actual{};
    HRESULT hr = pAlloc->SetProperties(pprop, &actual);
    if (FAILED(hr)) return hr;
    if (actual.cbBuffer < pprop->cbBuffer) return E_FAIL;
    return S_OK;
}

HRESULT VirtualRadioStream::FillBuffer(IMediaSample* pS) {
    BYTE* pData = nullptr; long cb = 0; pS->GetPointer(&pData); cb = pS->GetSize();
    size_t got = 0; while (got < (size_t)cb) {
        size_t n = dec_.readPcm(pData+got, cb-got);
        if (n==0) { Sleep(5); continue; }
        got += n;
    }
    pS->SetActualDataLength((long)got);

    // timestamps (100ns units)
    REFERENCE_TIME rtStart = rtNext_;
    REFERENCE_TIME rtDur = (REFERENCE_TIME)((10000000.0 * got) / (fmt_.sampleRate * fmt_.channels * (fmt_.bitsPerSample/8)));
    REFERENCE_TIME rtStop = rtStart + rtDur;
    pS->SetTime(&rtStart, &rtStop);
    pS->SetSyncPoint(TRUE);
    rtNext_ = rtStop;
    return S_OK;
}
