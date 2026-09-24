#pragma once
#ifdef USE_VIDEO_LIB
#include "GL.h"
#include "CImage.h"
#include "CSound.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
//b#include <inttypes.h>
}

#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libswscale.a")
#pragma comment(lib, "libswresample.a")
#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavformat.a")
#pragma comment(lib, "libavfilter.a")
#pragma comment(lib, "libavdevice.a")

class CVideoTextrue : public CTexture {
    struct VideoReaderState {
        // Public things for other parts of the program to read from
        int width, height;
        AVRational time_base;

        // Private internal state
        AVFormatContext* av_format_ctx;
        AVCodecContext* video_codec_ctx;
        AVCodecContext* audio_codec_ctx;
        int video_stream_index;
        int audio_stream_index;
        AVFrame* av_frame;
        AVPacket* av_packet;
        SwsContext* sws_scaler_ctx;
        int frame_number;
        SwrContext* swr_ctx;
    };
    uint8_t* m_swr_buf = 0;
    int m_swr_buf_len = 0;
    float m_time;
    float m_duration;
    float m_speed_scale;
    bool m_loop;
    std::string m_file_name;
    CSoundBase* m_sound;
    VideoReaderState m_state;
    bool video_reader_open(const char* filename);
    bool video_reader_read_frame(uint8_t* frame_buffer, int64_t* pts);
    bool video_decode_audio();
    bool video_reader_seek_frame(int64_t ts);
    void video_reader_close();

public:
    CVideoTextrue(const char* filename);
    void RenderFrame();
    void MapTexture();
    void Release();
    void Play(bool loop);
    void Stop();
    bool isEnd();
    friend class CVideo;
};

class CVideo : private CImage {
    CVideoTextrue* mp_video_texture;
public:
    CVideo(const char* filename);
    void Draw();
    void Play(bool lopp = true);
    void Stop();
    bool isEnd() {
        return mp_video_texture->isEnd();
    }
    bool isPlay() {
        return mp_video_texture->m_speed_scale == 0 ? false : true;
    }
    void SetSize(float w, float h) {
        CImage::SetSize(w, h);
    }
    void SetPos(float x, float y) {
        CImage::SetPos(x, y);
    }
    void SetPos(CVector2D pos) {
        CImage::SetPos(pos);
    }
};
#endif