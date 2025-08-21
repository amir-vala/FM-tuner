#pragma once
#include <dshow.h>
#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include "FfmpegDecoder.h"

// {8F6C1B1A-4EFA-4C69-9D02-4B8E4E6A9A11}
static const CLSID CLSID_VirtualRadioSource =
{ 0x8f6c1b1a, 0x4efa, 0x4c69, { 0x9d, 0x02, 0x4b, 0x8e, 0x4e, 0x6a, 0x9a, 0x11 } };

// custom interface to set frequency
// {12CA9C4D-0D30-4571-8FDE-2BA5B4C7F4D3}
static const IID IID_IVirtualRadioControl =
{ 0x12ca9c4d, 0x0d30, 0x4571, { 0x8f, 0xde, 0x2b, 0xa5, 0xb4, 0xc7, 0xf4, 0xd3 } };

struct __declspec(uuid("12CA9C4D-0D30-4571-8FDE-2BA5B4C7F4D3")) IVirtualRadioControl : public IUnknown {
    virtual HRESULT __stdcall SetFrequency(double mhz) = 0; // maps to URL via stations.json
    virtual HRESULT __stdcall SetDirectUrl(BSTR url) = 0;   // bypass mapping
};

class VirtualRadioStream;

class VirtualRadioSource : public CSource, public IVirtualRadioControl
{
public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

    // IVirtualRadioControl
    HRESULT __stdcall SetFrequency(double mhz) override;
    HRESULT __stdcall SetDirectUrl(BSTR url) override;

    void setUrl(const std::wstring& u);

private:
    VirtualRadioSource(LPUNKNOWN pUnk, HRESULT* phr);
    ~VirtualRadioSource();

    VirtualRadioStream* stream_ = nullptr;
    std::wstring url_;
};

class VirtualRadioStream : public CSourceStream
{
public:
    VirtualRadioStream(HRESULT* phr, VirtualRadioSource* pParent);
    ~VirtualRadioStream();

    HRESULT FillBuffer(IMediaSample* pS) override;
    HRESULT GetMediaType(CMediaType* pmt) override;
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pprop) override;

    // lifecycle: ensure decoder thread is managed with graph run/stop
    HRESULT Active() override;
    HRESULT Inactive() override;

    void SetUrl(const std::wstring& url);

private:
    REFERENCE_TIME rtNext_ = 0; // 100ns units
    DecodedFormat fmt_{};
    FfmpegDecoder dec_;
    std::wstring url_; // keep last URL to (re)open on Active
};
