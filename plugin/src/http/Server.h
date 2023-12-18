#pragma once

#include <juce_core/juce_core.h>

#include "../utils/AsyncFlacEncoder.h"
#include "../AudioSender.h"

#include "oatpp/network/Server.hpp"
#include <iostream>
#include "AppComponent.h"
#include "StaticFileController.h"
#include "WSEndpointController.h"
#include <atomic>

class Server : public juce::Thread, public AudioSender
{
    std::unique_ptr<AsyncFlacEncoder> _encoder;
    std::unique_ptr<oatpp::network::Server> _server;
    std::atomic<bool> _isServerRunning = false;

public:
    Server();
    ~Server();
    void run() override;

    void setAudioFormat(int numChannel, int bitPerSample, int sampleRate, int blockSize) override;
    void send(juce::AudioBuffer<float>) override;
};
