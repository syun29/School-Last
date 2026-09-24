#include "../GLLibrary.h"
#ifdef USE_VIDEO_LIB
#include "CFPS.h"
#include <list>

// av_err2str returns a temporary array. This doesn't work in gcc.
// This function can be used as a replacement for av_err2str.
static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static AVPixelFormat correct_for_deprecated_pixel_format(AVPixelFormat pix_fmt) {
    // Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (pix_fmt) {
    case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
    case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
    default:                  return pix_fmt;
    }
}

bool CVideoTextrue::video_reader_open(const char* filename) {

    m_file_name = filename;
    // Unpack members of state
    auto& width = m_state.width;
    auto& height = m_state.height;
    auto& time_base = m_state.time_base;
    auto& av_format_ctx = m_state.av_format_ctx;
    auto& video_codec_ctx = m_state.video_codec_ctx;
    auto& audio_codec_ctx = m_state.audio_codec_ctx;
    auto& video_stream_index = m_state.video_stream_index;
    auto& audio_stream_index = m_state.audio_stream_index;
    auto& av_frame = m_state.av_frame;
    auto& av_packet = m_state.av_packet;
    auto& swr_ctx = m_state.swr_ctx;
    auto& frame_number = m_state.frame_number;

    // Open the file using libavformat
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        printf("Couldn't created AVFormatContext\n");
        return false;
    }

    if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0) {
        printf("Couldn't open video file\n");
        return false;
    }

    // Find the first valid video stream inside the file
    video_stream_index = -1;
    audio_stream_index = -1;
    AVCodecParameters* av_codec_params = nullptr;
    AVCodecParameters* audio_codec_params = nullptr;
    AVCodecParameters* video_codec_params = nullptr;
    AVStream* audio_stream = nullptr;
    const AVCodec* av_codec = nullptr;
    const AVCodec* video_codec = nullptr;
    const AVCodec* audio_codec = nullptr;
    // ìÆâÊèÓïÒÇéÊìæ
    if (avformat_find_stream_info(av_format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream info\n";
        avformat_close_input(&av_format_ctx);
        return -1;
    }

    for (unsigned int i = 0; i < av_format_ctx->nb_streams; ++i) {
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            video_codec = av_codec;
            video_codec_params = av_codec_params;
            width = av_codec_params->width;
            height = av_codec_params->height;
            time_base = av_format_ctx->streams[i]->time_base;
        }

        if (av_codec_params->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            audio_codec_params = av_codec_params;
            audio_stream = av_format_ctx->streams[i];
            audio_codec = av_codec;
        }
    }

    av_frame = av_frame_alloc();
    av_packet = av_packet_alloc();
    if (audio_stream != nullptr) {


        audio_codec_ctx = avcodec_alloc_context3(audio_codec);
        if (audio_codec_ctx == NULL) {
            printf("avcodec_alloc_context3 error.\n");
            return false;
        }
        if (avcodec_parameters_to_context(audio_codec_ctx, av_codec_params) < 0) {
            printf("Couldn't initialize AVCodecContext\n");
            return false;
        }

        if (avcodec_open2(audio_codec_ctx, audio_codec, NULL) < 0) {
            printf("avcodec_open2 error.\n");
            return false;
        }
        video_decode_audio();


    }
    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return false;
    }

    // Set up a codec context for the decoder
    video_codec_ctx = avcodec_alloc_context3(video_codec);
    if (!video_codec_ctx) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(video_codec_ctx, video_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(video_codec_ctx, video_codec, NULL) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }

    frame_number = 0;

    return true;
}
bool CVideoTextrue::video_decode_audio() {

    // Unpack members of state
    auto& width = m_state.width;
    auto& height = m_state.height;
    auto& av_format_ctx = m_state.av_format_ctx;
    auto& video_codec_ctx = m_state.video_codec_ctx;
    auto& audio_codec_ctx = m_state.audio_codec_ctx;
    auto& video_stream_index = m_state.video_stream_index;
    auto& audio_stream_index = m_state.audio_stream_index;
    auto& av_frame = m_state.av_frame;
    auto& av_packet = m_state.av_packet;
    auto& swr_ctx = m_state.swr_ctx;
    auto& sws_scaler_ctx = m_state.sws_scaler_ctx;


    int response;
    std::list<uint8_t*> buffer_list;
    uint8_t* buf = NULL;
    int buf_size = 0;
    int buf_count = 0;
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index == video_stream_index) {

        }
        else if (av_packet->stream_index == audio_stream_index) {

            response = avcodec_send_packet(audio_codec_ctx, av_packet);
            if (response < 0) {
                printf("Failed to decode packet: %s\n", av_make_error(response));
                return false;
            }

            response = avcodec_receive_frame(audio_codec_ctx, av_frame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(av_packet);
                continue;
            }
            else if (response < 0) {
                printf("Failed to decode packet: %s\n", av_make_error(response));
                return false;
            }
            else {

                if (swr_ctx == NULL) {
                    swr_ctx = swr_alloc();
                    if (swr_ctx == NULL) {
                        printf("swr_alloc error.\n");
                        break;
                    }
                    av_opt_set_chlayout(swr_ctx,"ichl", &av_frame->ch_layout, 0);
                    av_opt_set_chlayout(swr_ctx, "ochl", &av_frame->ch_layout, 0);
                    av_opt_set_int(swr_ctx, "in_sample_rate", av_frame->sample_rate, 0);
                    av_opt_set_int(swr_ctx, "out_sample_rate", av_frame->sample_rate, 0);
                    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", (AVSampleFormat)av_frame->format, 0);
                    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
                    if (swr_init(swr_ctx) < 0) {
                        printf("swr_init error\n");
                        break;
                    }
                    buf_size = av_frame->nb_samples * av_frame->ch_layout.nb_channels * 2;
                    // the 2 means S16


                }
                buf = new uint8_t[buf_size];
                if (swr_convert(swr_ctx, &buf, av_frame->nb_samples, (const uint8_t**)av_frame->extended_data, av_frame->nb_samples) < 0) {
                    printf("swr_convert error\n");
                }
                buffer_list.push_back(buf);
                buf_count++;


            }

        }
        av_packet_unref(av_packet);

    }
    if (!swr_ctx) return false;
    CSoundBase* s = nullptr;
    m_sound = SOUND(m_file_name.c_str());
    m_swr_buf_len = buf_size * buf_count;
    uint8_t* p = m_swr_buf = new uint8_t[m_swr_buf_len];
    for (auto b : buffer_list) {
        memcpy(p, b, buf_size);
        p += buf_size;
        delete b;
    }
    WAVEFORMATEX format = { 0 };
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = av_frame->ch_layout.nb_channels;
    format.wBitsPerSample = 16;
    format.nSamplesPerSec = av_frame->sample_rate;
    format.nBlockAlign = format.wBitsPerSample / 8 * format.nChannels;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    m_sound->Create(m_swr_buf, m_swr_buf_len, AL_FORMAT_STEREO16, av_frame->sample_rate);


    av_seek_frame(av_format_ctx, video_stream_index, 0, 0);
    return true;
}
bool CVideoTextrue::video_reader_read_frame(uint8_t* frame_buffer, int64_t* pts) {

    // Unpack members of state
    auto& width = m_state.width;
    auto& height = m_state.height;
    auto& av_format_ctx = m_state.av_format_ctx;
    auto& video_codec_ctx = m_state.video_codec_ctx;
    auto& audio_codec_ctx = m_state.audio_codec_ctx;
    auto& video_stream_index = m_state.video_stream_index;
    auto& audio_stream_index = m_state.audio_stream_index;
    auto& av_frame = m_state.av_frame;
    auto& av_packet = m_state.av_packet;
    auto& sws_scaler_ctx = m_state.sws_scaler_ctx;
    auto& swr_ctx = m_state.swr_ctx;
    auto& frame_number = m_state.frame_number;

    // Decode one frame
    int response;
    //   av_seek_frame(av_format_ctx, video_stream_index, *pts, AVSEEK_FLAG_BACKWARD);
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index == video_stream_index) {
            response = avcodec_send_packet(video_codec_ctx, av_packet);
            if (response < 0) {
                printf("Failed to decode packet: %s\n", av_make_error(response));
                return false;
            }

            response = avcodec_receive_frame(video_codec_ctx, av_frame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(av_packet);
                continue;
            }
            else if (response < 0) {
                printf("Failed to decode packet: %s\n", av_make_error(response));
                return false;
            }
            *pts = av_frame->pts;

        }
        else if (av_packet->stream_index == audio_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }
        av_packet_unref(av_packet);
        break;
    }
    // Set up sws scaler
    if (!sws_scaler_ctx) {
        auto source_pix_fmt = correct_for_deprecated_pixel_format(video_codec_ctx->pix_fmt);
        sws_scaler_ctx = sws_getContext(width, height, source_pix_fmt,
            width, height, AV_PIX_FMT_RGB0,
            SWS_BILINEAR, NULL, NULL, NULL);
    }
    if (!sws_scaler_ctx) {
        printf("Couldn't initialize sw scaler\n");
        return false;
    }

    uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
    int dest_linesize[4] = { width * 4, 0, 0, 0 };
    sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);


    return true;
}

bool CVideoTextrue::video_reader_seek_frame(int64_t ts) {

    // Unpack members of state
    auto& av_format_ctx = m_state.av_format_ctx;
    auto& video_codec_ctx = m_state.video_codec_ctx;
    auto& audio_codec_ctx = m_state.audio_codec_ctx;
    auto& video_stream_index = m_state.video_stream_index;
    auto& av_frame = m_state.av_frame;
    auto& av_packet = m_state.av_packet;

    av_seek_frame(av_format_ctx, video_stream_index, ts, AVSEEK_FLAG_BACKWARD);

    // av_seek_frame takes effect after one frame, so I'm decoding one here
    // so that the next call to video_reader_read_frame() will give the correct
    // frame
    int response;
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(video_codec_ctx, av_packet);
        if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        response = avcodec_receive_frame(video_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
        }
        else if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        av_packet_unref(av_packet);
        break;
    }

    return true;
}

void CVideoTextrue::video_reader_close() {
    sws_freeContext(m_state.sws_scaler_ctx);
    avformat_close_input(&m_state.av_format_ctx);
    avformat_free_context(m_state.av_format_ctx);
    av_frame_free(&m_state.av_frame);
    av_packet_free(&m_state.av_packet);
    av_frame_free(&m_state.av_frame);
    avcodec_free_context(&m_state.video_codec_ctx);
    avcodec_free_context(&m_state.audio_codec_ctx);
}

CVideoTextrue::CVideoTextrue(const char* filename) : m_time(0), m_speed_scale(1),m_sound(nullptr)
{
    if (!video_reader_open(filename)) {
        char str[256] = "";
        sprintf_s(str, 256, "%sÇÃì«Ç›çûÇ›Ç…é∏îsÇµÇ‹ÇµÇΩ\n", filename);
        MessageBox(GL::hWnd, str, "", MB_OK);
    }

    constexpr int ALIGNMENT = 128;
    m_width = m_state.width;
    m_height = m_state.height;
   // m_duration = m_state.av_format_ctx->duration / (float)AV_TIME_BASE;
    m_duration = m_state.av_format_ctx->duration / (float)AV_TIME_BASE;
    if (!(m_data = (unsigned char*)_aligned_malloc(static_cast<size_t>(m_state.width) * m_state.height * 4, ALIGNMENT))) {
        printf("Couldn't allocate frame buffer\n");
        return;
    }

}

void CVideoTextrue::RenderFrame()
{

    if (m_speed_scale == 0) return;
    if (m_sound) {
        m_time = m_sound->GetOffset();
    }
    else {
        m_time += CFPS::GetDeltaTime() * m_speed_scale;
    }
    // m_state.av_frame->pts = uint64_t(m_time * (double)m_state.time_base.den / (double)m_state.time_base.num);
    int64_t next_pts = uint64_t(m_time * (double)m_state.time_base.den / (double)m_state.time_base.num);
    while (next_pts > m_state.av_frame->pts) {
        int64_t pts = m_state.av_frame->pts = next_pts;
        if (!video_reader_read_frame(m_data, &pts)) {
            printf("Couldn't load video frame\n");
            return;
        }
    }
    int64_t end_pts = uint64_t(m_duration * (double)m_state.time_base.den / (double)m_state.time_base.num);
    if (((m_sound && m_sound->CheckEnd()) || (next_pts >= end_pts)) && m_loop)
        Play(m_loop);




}

void CVideoTextrue::MapTexture()
{
    if (m_bufID == 0) {
        glGenTextures(1, &m_bufID);
        glBindTexture(GL_TEXTURE_2D, m_bufID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    glBindTexture(GL_TEXTURE_2D, m_bufID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_state.width, m_state.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
    glEnable(GL_TEXTURE_2D);
}

void CVideoTextrue::Release()
{
    video_reader_close();
    CTexture::Release();
}

void CVideoTextrue::Play(bool loop)
{

    video_reader_seek_frame(0);
    if (m_sound) m_sound->Play();
    m_speed_scale = 1.0f;
    m_time = 0;
    m_loop = loop;
    m_state.av_frame->pts = 0;
    av_seek_frame(m_state.av_format_ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(m_state.video_codec_ctx);
}

void CVideoTextrue::Stop()
{

    if (m_sound) m_sound->Stop();
    m_speed_scale = 0;
}

bool CVideoTextrue::isEnd()
{
    int64_t next_pts = uint64_t(m_time * (double)m_state.time_base.den / (double)m_state.time_base.num);
    int64_t end_pts = uint64_t(m_duration * (double)m_state.time_base.den / (double)m_state.time_base.num);
    if (((m_sound && m_sound->CheckEnd()) || (next_pts >= end_pts)) && !m_loop)
        return true;

    return false;
}

CVideo::CVideo(const char* filename)
{
    mp_texture = mp_video_texture = new CVideoTextrue(filename);
    m_width = mp_video_texture->m_width;
    m_height = mp_video_texture->m_height;
    SetPos(0, 0);
    SetRect(0, 0, (float)m_width, (float)m_height);
    SetSize((float)m_width, (float)m_height);
}

void CVideo::Draw()
{
    mp_video_texture->RenderFrame();
    CImage::Draw();
}

void CVideo::Play(bool loop)
{
    mp_video_texture->Play(loop);
}

void CVideo::Stop()
{
    mp_video_texture->Stop();
}
#endif