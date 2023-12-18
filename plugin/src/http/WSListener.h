#pragma once
#include <juce_core/juce_core.h>
#include <FLAC/ordinals.h>

#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/core/macro/component.hpp"

class WSListener : public oatpp::websocket::AsyncWebSocket::Listener {
    static constexpr const char* TAG = "Server_WSListener";
public:
    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) override;
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
};

class WSInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
private:
    std::vector<std::shared_ptr<WSListener::AsyncWebSocket>> sockets;
    std::vector<std::vector<FLAC__byte>> _flacHeaders;

    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);

    void sendBinaryMessage(const std::shared_ptr<WSListener::AsyncWebSocket>& socket, const FLAC__byte buffer[], size_t bytes);
    void sendBroadcastBinaryMessage(const FLAC__byte buffer[], size_t bytes);
public:
    WSInstanceListener();

    void onAfterCreate_NonBlocking(const std::shared_ptr<WSListener::AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) override;
    void onBeforeDestroy_NonBlocking(const std::shared_ptr<WSListener::AsyncWebSocket>& socket) override;

    void onFlacHeader(std::vector<std::vector<FLAC__byte>>);
    void onFlacFrame(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame);
};
