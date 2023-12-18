#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

// Audioを送信するためのインターフェース
class AudioSender {
public:
    virtual ~AudioSender() {}

    /// 送信するオーディオフォーマットを設定する
    virtual void setAudioFormat(int numChannel, int bitPerSample, int sampleRate, int blockSize) = 0;

    /// オーディオデータを送信する
    virtual void send(juce::AudioBuffer<float>) = 0;
};