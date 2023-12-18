#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>

#include "./FlacEncoder.h"
#include "./AudioRingBuffer.h"



class AsyncFlacEncoder : public juce::Thread {
    FlacEncoder _encoder;

    int _numChannel;
    int _bitPerSample;
    int _sampleRate;
    int _blockSize;

    AudioRingBuffer<float> _buffer;
    juce::AudioBuffer<float> _tmp_buffer;
    std::unique_ptr<FLAC__int32[]> _interleaved_tmp_buffer;

    using HeaderCallback = std::function<void(const std::vector<std::vector<FLAC__byte>>)>;
    using FrameCallback = std::function<void(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)> ;

    HeaderCallback _header_callback;
    FrameCallback _frame_callback;

public:
    AsyncFlacEncoder(
        int numChannel,
        int bitPerSample,
        int sampleRate, 
        int blockSize, 
        HeaderCallback header_callback,
        FrameCallback frame_callback);

    ~AsyncFlacEncoder();

    void process(juce::AudioBuffer<float>& buffer);

    void run() override;
};