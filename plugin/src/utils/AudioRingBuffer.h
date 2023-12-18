#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <condition_variable>
#include <mutex>

template<typename T>
/// オーディオデータを格納するリングバッファ
class AudioRingBuffer
{
    std::mutex _mtx;

    // バッファの要素数に変更があった場合の通知
    std::condition_variable _cv;

    size_t _buffer_size;
    size_t _channel_size;
    juce::AudioBuffer<float> _buffer;

    size_t _read_pos = 0;
    size_t _write_pos = 0;

    size_t readableSize()
    {
        if(_read_pos <= _write_pos)
            return _write_pos - _read_pos;
        else
            return (_buffer_size - _read_pos) + _write_pos;
    }

    size_t writableSize() { return (_buffer_size - 1) - readableSize(); }

    void copy(juce::AudioBuffer<T>& dst, size_t dst_pos, const juce::AudioBuffer<T>& src, size_t src_pos, size_t size) {
        for(int ch = 0; ch < _channel_size; ch++) {
            dst.copyFrom(ch, dst_pos, src, ch, src_pos, size);
        }
    }

   public:
    AudioRingBuffer(size_t channel, size_t capacity) : _buffer_size(capacity + 1), _channel_size(channel), _buffer(channel, capacity + 1) {}
    ~AudioRingBuffer() {}

    /// バッファに書き込む
    /// waitをtrueに指定するとバッファがいっぱいで書き込めない場合も
    /// すべて書き込めるようになるまで待機する（待機時はスレッドがsleepするのでcpu時間を消費しない）
    size_t write(const juce::AudioBuffer<T>& data, size_t size, bool wait)
    {
        std::unique_lock<std::mutex> lk(_mtx);

        if(wait)
        {
            _cv.wait(lk, [&] {  // 書き込める領域があることを待機
                return writableSize() >= size;
            });
        }
        else
        {
            if(writableSize() < size)
                size =
                    writableSize();  // 待機しない場合は書き込めるだけ書き込む
        }

        // 一度に書き込める場合
        if(size <= _buffer_size - _write_pos)
        {
            copy(_buffer, _write_pos, data, 0, size);
            
            _write_pos += size;

            if(_write_pos == _buffer_size) _write_pos = 0;
        }
        else
        {  // 一度に書き込めない場合、前後半に分けて書き込む
            size_t firstHalf = _buffer_size - _write_pos;
            size_t latterHalf = size - firstHalf;

            copy(_buffer, _write_pos, data, 0, firstHalf);
            _write_pos = 0;
            copy(_buffer, _write_pos, data, firstHalf, latterHalf);
            _write_pos = latterHalf;
        }

        // 他スレッドが待っていたら起こす
        _cv.notify_all();

        return size;
    }

    /// バッファから読み取る
    /// waitをtrueに設定すると、現時点で読み取れる十分な要素がない場合
    /// 指定されたサイズ分を読み取れるようになるまで待機する（待機時はスレッドがsleepするのでcpu時間を消費しない）
    size_t read(juce::AudioBuffer<T>& data, size_t size, bool wait)
    {
        std::unique_lock<std::mutex> lk(_mtx);

        if(wait)
        {  // 指定されたサイズ分読み取れるまで待機
            _cv.wait(lk, [&] { return readableSize() >= size; });
        }
        else
        {  // 待機しない場合は現在読み取れる分だけ読み取る
            if(readableSize() < size) size = readableSize();
        }

        if(size <= _buffer_size - _read_pos)
        {  // 一度に読み取れる場合
            copy(data, 0, _buffer, _read_pos, size);
            _read_pos += size;

            if(_read_pos == _buffer_size) _read_pos = 0;
        }
        else
        {  // 一度に読み取れない場合、前後半に分割して読み取る
            size_t firstHalf = _buffer_size - _read_pos;
            size_t latterHalf = size - firstHalf;

            copy(data, 0, _buffer, _read_pos, firstHalf);
            _read_pos = 0;
            copy(data, firstHalf, _buffer, _read_pos, latterHalf);
            _read_pos = latterHalf;
        }

        // 他スレッドが待っていたら起こす
        _cv.notify_all();

        return size;
    }
};