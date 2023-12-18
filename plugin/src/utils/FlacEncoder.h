#pragma once

#include <FLAC++/encoder.h>
#include <functional>
#include <vector>

class FlacEncoder: public FLAC::Encoder::Stream {
public:
    FlacEncoder(): FLAC::Encoder::Stream() {}

    void set_write_callback(std::function<::FLAC__StreamEncoderWriteStatus(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)> callback) {
        _write_callback = callback;
    }

    void set_header_callback(std::function<void(const std::vector<std::vector<FLAC__byte>>)> callback) {
        _header_callback = callback;
    }
    
protected:
    std::function<::FLAC__StreamEncoderWriteStatus(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)> _write_callback;
    std::function<void(const std::vector<std::vector<FLAC__byte>>)> _header_callback;

    ::FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame) override {
        if(!_write_callback) return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
        if(_is_metadata_end)
            return _write_callback(buffer, bytes, samples, current_frame);
        
        if(bytes == 4 && memcmp(buffer, "fLaC", 4) == 0) {
            std::vector<FLAC__byte> data(bytes);
            memcpy(&data[0], buffer, bytes);
            _headers.push_back(data);
        } else {
            std::vector<FLAC__byte> data(bytes);
            memcpy(&data[0], buffer, bytes);
            _headers.push_back(data);

            _is_metadata_end = buffer[0] & 0b10000000;
            if(_is_metadata_end)
                _header_callback(_headers);
        }
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }
private:
    bool _is_metadata_end = false;
    std::vector<std::vector<FLAC__byte>> _headers;
};