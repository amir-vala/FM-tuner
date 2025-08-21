#include "FfmpegDecoder.h"
#include <Windows.h>
#include <codecvt>
#include <locale>

static std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr);
    std::string out(size,'\0');
    WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),out.data(),size,nullptr,nullptr);
    return out;
}

FfmpegDecoder::FfmpegDecoder() { avformat_network_init(); }
FfmpegDecoder::~FfmpegDecoder() { close(); avformat_network_deinit(); }

bool FfmpegDecoder::open(const std::wstring& url, const DecodedFormat& outFmt) {
    close();            // ensure previous thread gone
    stop_ = false;
    running_ = true;
    th_ = std::thread(&FfmpegDecoder::threadMain, this, WideToUtf8(url), outFmt);
    return true;
}

void FfmpegDecoder::close() {
    stop_ = true;
    if (th_.joinable()) th_.join();
    running_ = false;
}

size_t FfmpegDecoder::readPcm(uint8_t* dst, size_t bytes) {
    return ring_.read(dst, bytes);
}

void FfmpegDecoder::threadMain(std::string url, DecodedFormat outFmt) {
    fmt_ = avformat_alloc_context();
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "user_agent", "VirtualRadioSource/1.0", 0);
    av_dict_set(&opts, "reconnect", "1", 0);
    av_dict_set(&opts, "reconnect_streamed", "1", 0);
    av_dict_set(&opts, "reconnect_delay_max", "5", 0);

    if (avformat_open_input(&fmt_, url.c_str(), nullptr, &opts) < 0) goto done;
    if (avformat_find_stream_info(fmt_, nullptr) < 0) goto done;

    // find audio stream
    for (unsigned i=0;i<fmt_->nb_streams;i++) {
        if (fmt_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) { audioStreamIndex_ = (int)i; break; }
    }
    if (audioStreamIndex_ < 0) goto done;

    {
        AVCodecParameters* par = fmt_->streams[audioStreamIndex_]->codecpar;
        const AVCodec* dec = avcodec_find_decoder(par->codec_id);
        if (!dec) goto done;
        dec_ = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(dec_, par);
        if (avcodec_open2(dec_, dec, nullptr) < 0) goto done;

        swr_ = swr_alloc_set_opts(nullptr,
            av_get_default_channel_layout(outFmt.channels), AV_SAMPLE_FMT_S16, outFmt.sampleRate,
            av_get_default_channel_layout(dec_->channels), dec_->sample_fmt, dec_->sample_rate,
            0, nullptr);
        if (!swr_ || swr_init(swr_)<0) goto done;

        AVPacket* pkt = av_packet_alloc();
        AVFrame*  frm = av_frame_alloc();
        // ~0.5s output buffer
        std::vector<uint8_t> tmp( (outFmt.sampleRate * outFmt.channels * (outFmt.bitsPerSample/8)) / 2 );

        while (!stop_) {
            if (av_read_frame(fmt_, pkt) < 0) { Sleep(10); continue; }
            if (pkt->stream_index != audioStreamIndex_) { av_packet_unref(pkt); continue; }
            if (avcodec_send_packet(dec_, pkt) < 0) { av_packet_unref(pkt); continue; }
            av_packet_unref(pkt);
            while (avcodec_receive_frame(dec_, frm) == 0) {
                uint8_t* outBuf[1] = { tmp.data() };
                const int maxOutSamples = (int)(tmp.size() / (outFmt.channels * 2));
                int outSamples = swr_convert(swr_, outBuf, maxOutSamples,
                                             (const uint8_t**)frm->extended_data, frm->nb_samples);
                if (outSamples>0) {
                    size_t bytes = (size_t)outSamples * outFmt.channels * 2;
                    ring_.write(tmp.data(), bytes); // if full, drop excess silently (streaming)
                }
                av_frame_unref(frm);
            }
        }
        av_frame_free(&frm);
        av_packet_free(&pkt);
    }

done:
    if (swr_) { swr_free(&swr_); swr_ = nullptr; }
    if (dec_) { avcodec_free_context(&dec_); dec_ = nullptr; }
    if (fmt_) { avformat_close_input(&fmt_); avformat_free_context(fmt_); fmt_ = nullptr; }
}
