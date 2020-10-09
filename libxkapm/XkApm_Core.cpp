//
// Created by Admin on 2019/7/5.
//

#include "XkAPM_Core.h"

#include <android/log.h>
#undef	LOG_TAG
#define LOG_TAG "XkAPM"
#define LOGI(...) __android_log_print (ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOCAL_NN 80
#define MAX_FRAME_LEGTH    640
XkAPM_Core::XkAPM_Core() {
    memset(&m_config, 0, sizeof(m_config));
    memset(m_vadInst,0,sizeof(m_vadInst));
    memset(m_nsHandle,0,sizeof(m_nsHandle));
    memset(m_agcInst,0,sizeof(m_agcInst));
    m_aecInst = NULL;
    m_samplerate = 8000;
    m_bOpen = false;

    m_config.bAecEnable = true;
    m_config.bAgcEnable = true;
    m_config.bNsEnable = true;
    m_config.iNsMode = 2;
    m_config.iVadMode = 2;
    m_config.iAecDelay = 0;
    m_config.iAecMode = 2;
    m_config.fAgcDb = 10.f;
}

XkAPM_Core::~XkAPM_Core() {
}

int XkAPM_Core::open() {
	AutoLock lock(&m_mutex);
    if (m_bOpen == true) {
        return -1;
    }

    int ret = 0;

    LOGI("bAecEnable = %d,bAgcEnable = %d,bNsEnable = %d",m_config.bAecEnable,m_config.bAgcEnable,m_config.bNsEnable);
    LOGI("iAecDelay = %d,iAecMode = %d,iNsMode = %d,iVadMode = %d",m_config.iAecDelay,m_config.iAecMode,m_config.iNsMode,m_config.iVadMode);

    if(m_config.bAecEnable == true){
        m_aecInst = WebRtcAec_Create();
        if (m_aecInst == NULL) {
            LOGI("WebRtcAec_Create error here!");
            return -1;
        }

        ret = WebRtcAec_Init(m_aecInst, m_samplerate, m_samplerate);
        if(ret == -1){
            LOGI("WebRtcAec_Init error here! ret %d",ret);
            return -1;
        }
        LOGI("m_config.iAecMode = %d", m_config.iAecMode);
        AecConfig config;
        memset(&config, 0, sizeof(config));
        config.metricsMode = kAecFalse;
        config.skewMode = kAecFalse;
        config.nlpMode = m_config.iAecMode;
        ret = WebRtcAec_set_config(m_aecInst, config);
        if(ret != 0){
            LOGI("WebRtcAec_set_config error here! ret %d",ret);
            return -1;
        }

        LOGI("init AEC success!");
    }


    for(int i = 0;i < 2;i++) {
    	if(m_config.bNsEnable == true){
            m_nsHandle[i] = WebRtcNs_Create();
            if(m_nsHandle[i] == NULL){
                LOGI("WebRtcNs_Create error here!");
                return -1;
            }

            ret = WebRtcNs_Init(m_nsHandle[i], m_samplerate);
            if(ret != 0){
                LOGI("WebRtcNs_Init error here! ret %d",ret);
                return -1;
            }

            int ret = WebRtcNs_set_policy(m_nsHandle[i], m_config.iNsMode);
            if(ret != 0){
                LOGI("WebRtcNs_set_policy error here! ret %d",ret);
                return -1;
            }

            LOGI("init NS success!");
    	}


        m_vadInst[i] = WebRtcVad_Create();
        if(m_vadInst[i] == NULL){
            LOGI("WebRtcVad_Create error here!");
            return -1;
        }

        ret = WebRtcVad_Init(m_vadInst[i]);
        if(ret != 0){
            LOGI("WebRtcVad_Init error here! ret %d",ret);
            return -1;
        }

        ret = WebRtcVad_set_mode(m_vadInst[i], m_config.iVadMode);
        if(ret != 0){
            LOGI("WebRtcVad_set_mode error here! ret %d",ret);
            return -1;
        }

        ret = WebRtcVad_ValidRateAndFrameLength(m_samplerate, LOCAL_NN);
        if(ret != 0){
            LOGI("WebRtcVad_ValidRateAndFrameLength error here! ret %d",ret);
            return -1;
        }
        LOGI("init VAD success!");

        if(m_config.bAgcEnable == true){
            m_agcInst[i] = new GainController2();
            m_agcInst[i]->Initialize(AudioProcessing::kSampleRate8kHz);

    		AudioProcessing::Config::GainController2 config;
    		// Check that some gain is applied if there is no margin.
    		config.enabled = true;
    		config.fixed_digital.gain_db = (float)m_config.fAgcDb;
    		config.adaptive_digital.enabled = false;
    		m_agcInst[i]->ApplyConfig(config);
        }
    }
    m_bOpen = true;
    return 0;
}

void XkAPM_Core::close() {
	AutoLock lock(&m_mutex);
    if(m_bOpen == false)
        return;
    if(m_aecInst != NULL){
        WebRtcAec_Free(m_aecInst);
        m_aecInst = NULL;
    }

    for(int i = 0; i < 2; i++){
        if(m_nsHandle[i] != NULL){
            WebRtcNs_Free(m_nsHandle[i]);
            m_nsHandle[i] = NULL;
        }

        if(m_vadInst[i] != NULL){
            WebRtcVad_Free(m_vadInst[i]);
            m_vadInst[i] = NULL;
        }

        if(m_agcInst[i] != NULL){
        	delete m_agcInst[i];
        	m_agcInst[i] = NULL;
        }
    }
    m_bOpen = false;
}
// returns              : 1 - (Active Voice),
//                        0 - (Non-active Voice),
//                       -1 - (Error)
int XkAPM_Core::vad_process(XkAPM_Dir dir, short *pBuf, size_t frame_length) {
	AutoLock lock(&m_mutex);
    if(m_vadInst[dir] == NULL)
        return -1;

    int frameLength_10ms = frame_length / 4;
    int voice_ms = 0;
    for(int i = 0;i< 4;i++){
        int res = WebRtcVad_Process(m_vadInst[dir], 8000, pBuf+LOCAL_NN*i, LOCAL_NN);
        if(res == 1){
            voice_ms += 10;
        }
    }
    return voice_ms;
}

void XkAPM_Core::aec_process(XkAPM_Dir dir, short *pBuf, size_t frame_length) {
	AutoLock lock(&m_mutex);
	if(m_aecInst == NULL)
		return;
	for(int i= 0; i< 4;i++){
		if(dir == XKAPM_DIR_FAR_END){
		    aec_10msFarEndFrame(pBuf+i*LOCAL_NN,LOCAL_NN);
		} else if(dir == XKAPM_DIR_NEAR_END){
		    aec_10msNearEndFrame(pBuf+i*LOCAL_NN,LOCAL_NN);
		}
	}

}

void XkAPM_Core::ns_process(XkAPM_Dir dir, short* pBuf, size_t frame_length) {
	AutoLock lock(&m_mutex);
    if(m_nsHandle[dir] == NULL)
        return;
    int frameLength_10ms = frame_length / 4;
    for(int i = 0;i< 4;i++){
        ns_10msFrame(m_nsHandle[dir], pBuf + i * LOCAL_NN, LOCAL_NN);
    }
}

void XkAPM_Core::agc_process(XkAPM_Dir dir,short* pBuf, size_t frame_length)
{
	AutoLock lock(&m_mutex);
    if(m_agcInst[dir] == NULL)
        return;
	int agc_sample_len = 80;
	AudioBuffer ab(agc_sample_len, 1, agc_sample_len, 1, agc_sample_len);
	for (int i = 0; i < 4; i++) {
		int16_t* out_ptr_;
		out_ptr_ = (int16_t*) (pBuf + i * 80);
		memcpy(ab.channels()[0], out_ptr_, agc_sample_len * 2);
		m_agcInst[dir]->Process(&ab);
		memcpy(out_ptr_, ab.channels()[0], agc_sample_len * 2);
	}
}

void XkAPM_Core::ns_10msFrame(NsHandle *nsHandle, short *pBuf, size_t frame_length) {
    int NN = frame_length;
    const float* near_ptr_;
    float* out_ptr_;
    float near_frame[MAX_FRAME_LEGTH]= { 0 };
    float out_frame[MAX_FRAME_LEGTH]= { 0 };
    memset(near_frame,0,MAX_FRAME_LEGTH*sizeof(float));
    memset(out_frame,0,MAX_FRAME_LEGTH*sizeof(float));
    near_ptr_ = near_frame;
    out_ptr_ = out_frame;

    for (int i = 0; i < LOCAL_NN; ++i)
    {
        near_frame[i] = pBuf[i];
    }
    WebRtcNs_Analyze(nsHandle,near_frame);
    WebRtcNs_Process(nsHandle,
                     &near_ptr_,
                     1,
                     &out_ptr_);
    for (int i = 0; i < LOCAL_NN; ++i)
    {
        pBuf[i] = out_frame[i];
    }

}

void XkAPM_Core::aec_10msFarEndFrame(short *pBuf, size_t frame_length) {
	float far_frame[MAX_FRAME_LEGTH]= { 0 };
    for (int i = 0; i < LOCAL_NN; ++i)
    {
        far_frame[i] = pBuf[i];
    }
    int32_t ret = WebRtcAec_BufferFarend(m_aecInst,
                                         (const float*)far_frame,
                                         LOCAL_NN);

}

void XkAPM_Core::aec_10msNearEndFrame(short *pData, size_t frame_length) {
    const float* near_ptr_;
    float* out_ptr_;
    float near_frame[MAX_FRAME_LEGTH]= { 0 };
    float out_frame[MAX_FRAME_LEGTH]= { 0 };
//    float* near_frame= (float *)malloc(sizeof(float) * frame_length);
    memset(near_frame,0,MAX_FRAME_LEGTH*sizeof(float));
//    float* out_frame = (float *)malloc(sizeof(float) * frame_length);
    memset(out_frame,0,MAX_FRAME_LEGTH*sizeof(float));

    near_ptr_ = near_frame;
    out_ptr_ = out_frame;

//	LOGI("ProcessStream UpdateFrame before");


    int delay = m_config.iAecDelay*10;
//	LOGI("frame_length = %d",frame_length);
//	LOGI("delay = %d",delay);
    for (int i = 0; i < LOCAL_NN; ++i)
    {
        near_frame[i] = pData[i];
    }
    int32_t ret =  WebRtcAec_Process(m_aecInst,
                                     &near_ptr_,
                                     1,
                                     &out_ptr_,
                                     LOCAL_NN,
                                     delay,
                                     0);

//	LOGI("ProcessStream UpdateFrame after  ret = %d,lv_aec_delay_param  = %d",ret, lv_aec_delay_param);
    if(ret == 0){
        for (int i = 0; i < LOCAL_NN; ++i)
        {
            pData[i] = out_frame[i];
        }
    }

//    if(near_frame){
//        free(near_frame);
//        near_frame = NULL;
//    }
//
//    if(out_frame){
//        free(out_frame);
//        out_frame = NULL;
//    }
}
