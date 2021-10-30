/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <cstring>
#include <string>
#include <mutex>
#include <thread>

#include <rapidjson/document.h>

#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include "SampleApp/PortAudioMicrophoneWrapper.h"
#include "SampleApp/ConsolePrinter.h"
#include <alsa/asoundlib.h>

#ifdef OLLI_IMPL
#ifdef __cplusplus
extern "C"
{
#endif
#define KWS_FRAMES 256
#define KWS_KEYWORD_LEN 8000

#define AEC_SERVER_DEVNAME "mtk_ec"
#define AEC_CLIENT_DEVNAME1 "mtk_ipc_ufb"
#define AEC_CLIENT_DEVNAME2 "mtk_ipc_qsr"
#define AEC_CLIENT_DEVNAME3 "mtk_ipc_qcom"

#define AEC_PERIOD_SIZE KWS_FRAMES
#define AEC_PERIODS 64
#define AEC_BITS 16
#define AEC_CHANNELS 1
#define AEC_SAMPLE_RATE 16000
#define kOLLI_WW_AUDIO_FRAME_SIZE 2

static struct olli_config {
    snd_pcm_t *handle_pcm;
} l_olli_data;

static void close_alsa_device(snd_pcm_t *pcm)
{
   if (pcm != NULL)
	   snd_pcm_close(pcm);
}

static int open_alsa_device(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream,
                         snd_pcm_format_t format, snd_pcm_access_t access,
                         unsigned int channels, unsigned int rate, unsigned long period_size, unsigned int periods)
{
    int err = 0;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams = NULL;
    unsigned long exact_period_size;
    unsigned int exact_rate, exact_periods;

    printf("device name: %s\n", name);
    err = snd_pcm_open(&handle, name, stream, 0);
    if (err < 0) {
        printf("Failed to snd_pcm_open(%s): strerr=%s\n", name, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    snd_pcm_hw_params_alloca(&hwparams);
    err = snd_pcm_hw_params_any(handle, hwparams);
    if (err < 0){
        printf("Failed to snd_pcm_hw_params_any. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_hw_params_set_format(handle, hwparams, format);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_format. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_hw_params_set_access(handle, hwparams, access);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_access. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_hw_params_set_channels(handle, hwparams, channels);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_channels(%d). strerr=%s\n", channels, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    exact_rate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &exact_rate, 0);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_rate_near(%d). strerr=%s\n", exact_rate, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    exact_period_size = period_size;
    err = snd_pcm_hw_params_set_period_size_near(handle, hwparams, &exact_period_size, 0);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_period_size_near(%ld). strerr=%s\n", exact_period_size, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    exact_periods = periods;
    err = snd_pcm_hw_params_set_periods_near(handle, hwparams, &exact_periods, 0);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params_set_periods_near. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_hw_params(handle, hwparams);
    if (err < 0) {
        printf("Failed to snd_pcm_hw_params. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }

    snd_pcm_sw_params_t *swparams = NULL;
    unsigned long start_threshold = 0;
    unsigned long stop_threshold = 0;

    if (stream == SND_PCM_STREAM_CAPTURE) {
        start_threshold = exact_period_size * exact_periods / 2;
        stop_threshold = exact_period_size * exact_periods;
    } else {
        start_threshold = exact_period_size * exact_periods / 2;
        stop_threshold = exact_period_size * exact_periods;
    }
    snd_pcm_sw_params_alloca(&swparams);
    snd_pcm_sw_params_current(handle, swparams);
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
    if (err < 0) {
        printf("Failed to snd_pcm_sw_params_set_start_threshold(%ld). strerr=%s\n", start_threshold, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    if (err < 0) {
        printf("Failed to snd_pcm_sw_params_set_stop_threshold(%ld). strerr=%s\n", stop_threshold, snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        printf("Failed to snd_pcm_sw_params. strerr=%s\n", snd_strerror(err));
        close_alsa_device(handle);
        return -1;
    }
    if (stream == SND_PCM_STREAM_CAPTURE) {
        err = snd_pcm_start(handle);
        if (err < 0) {
            printf("Failed to snd_pcm_start: strerr=%s\n", snd_strerror(err));
            close_alsa_device(handle);
            return -1;
        }
    }
    *pcm = handle;
    return 0;
}
#ifdef __cplusplus
}
#endif
#endif

namespace alexaClientSDK {
namespace sampleApp {

using avsCommon::avs::AudioInputStream;

static const int NUM_INPUT_CHANNELS = 1;
static const int NUM_OUTPUT_CHANNELS = 0;
static const double SAMPLE_RATE = 16000;
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK = paFramesPerBufferUnspecified;

static const std::string SAMPLE_APP_CONFIG_ROOT_KEY("sampleApp");
static const std::string PORTAUDIO_CONFIG_ROOT_KEY("portAudio");
static const std::string PORTAUDIO_CONFIG_SUGGESTED_LATENCY_KEY("suggestedLatency");

void PortAudioMicrophoneWrapper::OLLIStartThreadrReadData(void *userData)
{
    int ret;
    short data[KWS_FRAMES];
    PortAudioMicrophoneWrapper* wrapper = static_cast<PortAudioMicrophoneWrapper*>(userData);
    if (!l_olli_data.handle_pcm) {
        printf("The handle is not opened!\n");
    }
    while (wrapper->isThreadReadRunning()) {
        std::memset(data, 0, sizeof(data));
        ret = snd_pcm_readi(l_olli_data.handle_pcm, (void *)data, KWS_FRAMES);
        if (ret < 0) {
            printf("Failed to read data to stream.\n");
        }
        if (wrapper->isStreaming()) {
            ssize_t returnCode = wrapper->m_writer->write((const void *)data, KWS_FRAMES);
            if (returnCode <= 0) {
                printf("Failed to write to stream.\n");
            }
        }
    }
}

/// String to identify log entries originating from this file.
static const std::string TAG("PortAudioMicrophoneWrapper");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::unique_ptr<PortAudioMicrophoneWrapper> PortAudioMicrophoneWrapper::create(
    std::shared_ptr<AudioInputStream> stream) {
    if (!stream) {
        ACSDK_CRITICAL(LX("Invalid stream passed to PortAudioMicrophoneWrapper"));
        return nullptr;
    }
    std::unique_ptr<PortAudioMicrophoneWrapper> portAudioMicrophoneWrapper(new PortAudioMicrophoneWrapper(stream));
    if (!portAudioMicrophoneWrapper->initialize()) {
        ACSDK_CRITICAL(LX("Failed to initialize PortAudioMicrophoneWrapper"));
        return nullptr;
    }
    return portAudioMicrophoneWrapper;
}

PortAudioMicrophoneWrapper::PortAudioMicrophoneWrapper(std::shared_ptr<AudioInputStream> stream) :
        m_audioInputStream{stream},
        m_paStream{nullptr},
        m_isStreaming{false},
        m_isThreadReadRunning{false} {
}

PortAudioMicrophoneWrapper::~PortAudioMicrophoneWrapper() {
#ifdef OLLI_IMPL
    m_isStreaming = false;
    m_isThreadReadRunning = false;
    if (l_olli_data.handle_pcm) {
        snd_pcm_close(l_olli_data.handle_pcm);
    }
    l_olli_data.handle_pcm = NULL;
#else
    Pa_StopStream(m_paStream);
    Pa_CloseStream(m_paStream);
    Pa_Terminate();
#endif
}

bool PortAudioMicrophoneWrapper::initialize() {
    m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        ACSDK_CRITICAL(LX("Failed to create stream writer"));
        return false;
    }
#ifndef OLLI_IMPL
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        ACSDK_CRITICAL(LX("Failed to initialize PortAudio").d("errorCode", err));
        return false;
    }

    PaTime suggestedLatency = -1;
    bool latencyInConfig = getConfigSuggestedLatency(suggestedLatency);

    if (!latencyInConfig) {
        err = Pa_OpenDefaultStream(
            &m_paStream,
            NUM_INPUT_CHANNELS,
            NUM_OUTPUT_CHANNELS,
            paInt16,
            SAMPLE_RATE,
            PREFERRED_SAMPLES_PER_CALLBACK,
            PortAudioCallback,
            this);
    } else {
        ACSDK_INFO(
            LX("PortAudio suggestedLatency has been configured to ").d("Seconds", std::to_string(suggestedLatency)));
        if (suggestedLatency < 0) {
            ACSDK_CRITICAL(LX("Failed to configure PortAudio invalid suggestedLatency"));
            return false;
        }

        PaStreamParameters inputParameters;
        std::memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.device = Pa_GetDefaultInputDevice();
        inputParameters.channelCount = NUM_INPUT_CHANNELS;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = suggestedLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        err = Pa_OpenStream(
            &m_paStream,
            &inputParameters,
            nullptr,
            SAMPLE_RATE,
            PREFERRED_SAMPLES_PER_CALLBACK,
            paNoFlag,
            PortAudioCallback,
            this);
    }

    if (err != paNoError) {
        ACSDK_CRITICAL(LX("Failed to open PortAudio default stream").d("errorCode", err));
        return false;
    }
#else
    if (!l_olli_data.handle_pcm) {
        int ret = open_alsa_device(&l_olli_data.handle_pcm, AEC_CLIENT_DEVNAME3, SND_PCM_STREAM_CAPTURE,
                                   SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED,
                                   AEC_CHANNELS, AEC_SAMPLE_RATE, AEC_PERIOD_SIZE, AEC_PERIODS);
        if (ret < 0) {
            printf("OLLI PCM Openning Fail\n");
            return false;
        } else {
            m_isThreadReadRunning = true;
            m_readInputThread = std::thread(&PortAudioMicrophoneWrapper::OLLIStartThreadrReadData, this, this);
        }
    } else {
        printf("OLLI PCM Openning\n");
    }
#endif
    return true;
}

bool PortAudioMicrophoneWrapper::startStreamingMicrophoneData() {
    ACSDK_INFO(LX(__func__));
    std::lock_guard<std::mutex> lock{m_mutex};
#ifndef OLLI_IMPL
    PaError err = Pa_StartStream(m_paStream);
    if (err != paNoError) {
        ACSDK_CRITICAL(LX("Failed to start PortAudio stream"));
        return false;
    }
#endif
    m_isStreaming = true;
    return true;
}

bool PortAudioMicrophoneWrapper::stopStreamingMicrophoneData() {
    ACSDK_INFO(LX(__func__));
    std::lock_guard<std::mutex> lock{m_mutex};
#ifndef OLLI_IMPL
    PaError err = Pa_StopStream(m_paStream);
    if (err != paNoError) {
        ACSDK_CRITICAL(LX("Failed to stop PortAudio stream"));
        return false;
    }
#endif
    m_isStreaming = false;
    return true;
}

bool PortAudioMicrophoneWrapper::isStreaming() {
    return m_isStreaming;
}

bool PortAudioMicrophoneWrapper::isThreadReadRunning() {
    return m_isThreadReadRunning;
}

int PortAudioMicrophoneWrapper::PortAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long numSamples,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    PortAudioMicrophoneWrapper* wrapper = static_cast<PortAudioMicrophoneWrapper*>(userData);
    ssize_t returnCode = wrapper->m_writer->write(inputBuffer, numSamples);
    if (returnCode <= 0) {
        ACSDK_CRITICAL(LX("Failed to write to stream."));
        return paAbort;
    }
    return paContinue;
}

bool PortAudioMicrophoneWrapper::getConfigSuggestedLatency(PaTime& suggestedLatency) {
    bool latencyInConfig = false;
    auto config = avsCommon::utils::configuration::ConfigurationNode::getRoot()[SAMPLE_APP_CONFIG_ROOT_KEY]
                                                                               [PORTAUDIO_CONFIG_ROOT_KEY];
    if (config) {
        latencyInConfig = config.getValue(
            PORTAUDIO_CONFIG_SUGGESTED_LATENCY_KEY,
            &suggestedLatency,
            suggestedLatency,
            &rapidjson::Value::IsDouble,
            &rapidjson::Value::GetDouble);
    }

    return latencyInConfig;
}

}  // namespace sampleApp
}  // namespace alexaClientSDK
