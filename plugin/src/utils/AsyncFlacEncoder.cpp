#include "AsyncFlacEncoder.h"

AsyncFlacEncoder::AsyncFlacEncoder(int numChannel, int bitPerSample, int sampleRate, int blockSize, HeaderCallback header_callback, FrameCallback frame_callback)
    : juce::Thread("AsyncFlacEncoder")
    , _numChannel(numChannel)
    , _bitPerSample(bitPerSample)
    , _sampleRate(sampleRate)
    , _blockSize(blockSize)
    , _encoder()
    , _buffer(numChannel, blockSize * 4)
    , _tmp_buffer(numChannel, blockSize)
    , _interleaved_tmp_buffer(new FLAC__int32[numChannel * blockSize + 1])
    , _header_callback(header_callback)
    , _frame_callback(frame_callback)
{
    _encoder.set_verify(false);
    _encoder.set_compression_level(5);
    _encoder.set_channels(numChannel);
    _encoder.set_bits_per_sample(bitPerSample);
    _encoder.set_sample_rate(sampleRate);
    _encoder.set_header_callback([&](std::vector<std::vector<FLAC__byte>> headers) {
        if(_header_callback)
            _header_callback(headers);
    });
    _encoder.set_write_callback([&](const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame) -> ::FLAC__StreamEncoderWriteStatus {
        if(_frame_callback)
            _frame_callback(buffer, bytes, samples, current_frame);
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    });
    _encoder.init();

    startThread();
}

AsyncFlacEncoder::~AsyncFlacEncoder() {
    stopThread(1000);
    _encoder.finish();
}

void AsyncFlacEncoder::process(juce::AudioBuffer<float>& buffer) {
    _buffer.write(buffer, _blockSize, true);
}

void AsyncFlacEncoder::run() {
    while(!threadShouldExit()) {
        _buffer.read(_tmp_buffer, _tmp_buffer.getNumSamples(), true);
        for(auto channel = 0; channel < _numChannel; channel++) {
            for(auto sample = 0; sample < _blockSize; sample++) {
                _interleaved_tmp_buffer[sample * _numChannel + channel] = (FLAC__int32)(std::clamp(_tmp_buffer.getSample(channel, sample) , -1.0f, 1.0f) * (INT32_MAX >> (32 - _bitPerSample)));
            }
        }
        _encoder.process_interleaved(_interleaved_tmp_buffer.get(), _blockSize);
    }
}