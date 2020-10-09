//
// Created by Admin on 2019/7/5.
//

#ifndef LIBXKCODEC_AS_XKAPM_H
#define LIBXKCODEC_AS_XKAPM_H

#include "XkApm.h"
#include "common_audio/vad/include/webrtc_vad.h"
#include "modules/audio_processing/ns/noise_suppression.h"
#include "modules/audio_processing/aec/echo_cancellation.h"
#include "modules/audio_processing/gain_controller2.h"
#include "modules/audio_processing/audio_buffer.h"
#include "framework.h"

using namespace webrtc;

class XkAPM_Core {
public:
    XkAPM_Core();
    ~XkAPM_Core();

    int open();
    void close();

    void reset(){
        close();
        open();
    }

    /**
     * 仅支持8000采样率的40ms数据,size = 640
     * @param dir 方向
     * @param pBuf
     * @param size
     * @return
     */
    int vad_process(XkAPM_Dir dir,short* pBuf, size_t frame_length);
    void aec_process(XkAPM_Dir dir,short* pBuf, size_t frame_length);
    void ns_process(XkAPM_Dir dir,short* pBuf, size_t frame_length);
    void agc_process(XkAPM_Dir dir,short* pBuf, size_t frame_length);

    void config(XkApm_Param config){
        m_config = config;
    }

private:
    void ns_10msFrame(NsHandle *nsHandle, short *pBuf, size_t frame_length);
    void aec_10msFarEndFrame(short *pBuf, size_t frame_length);
    void aec_10msNearEndFrame(short *pBuf, size_t frame_length);

private:
    VadInst* m_vadInst[2];
    NsHandle* m_nsHandle[2];
    GainController2* m_agcInst[2];
    void* m_aecInst;
    bool m_bOpen;
    int m_samplerate;
    XkApm_Param m_config;
    XN_Mutex m_mutex;
};


#endif //LIBXKCODEC_AS_XKAPM_H
