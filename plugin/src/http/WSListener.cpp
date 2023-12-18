#include "WSListener.h"

oatpp::async::CoroutineStarter WSListener::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onPing");
  return socket->sendPongAsync(message);
}

oatpp::async::CoroutineStarter WSListener::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onPong");
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WSListener::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onClose code=%d", code);
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WSListener::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
  return nullptr; // do nothing
}

// ------------------------------

WSInstanceListener::WSInstanceListener() {
}

void WSInstanceListener::onAfterCreate_NonBlocking(const std::shared_ptr<WSListener::AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {
  socket->setListener(std::make_shared<WSListener>());

  for(auto& header : _flacHeaders) {
    sendBinaryMessage(socket, header.data(), header.size());
    juce::Thread::sleep(10);
  }
  sockets.push_back(socket);
}

void WSInstanceListener::onBeforeDestroy_NonBlocking(const std::shared_ptr<WSListener::AsyncWebSocket>& socket) {
    auto target = std::find(sockets.begin(), sockets.end(), socket);
    if(target != sockets.end()) {
        sockets.erase(target);
    }
}

void WSInstanceListener::sendBinaryMessage(const std::shared_ptr<WSListener::AsyncWebSocket>& socket, const FLAC__byte data[], size_t size) {
    class TestCoroutine: public oatpp::async::Coroutine<TestCoroutine> {
        std::shared_ptr<WSListener::AsyncWebSocket> socket;
        const void* data;
        size_t size;

        public:

        TestCoroutine(std::shared_ptr<WSListener::AsyncWebSocket> _socket, const void* _data, size_t _size) {
            socket = _socket;
            data = new char[_size];
            size = _size;
            memcpy((void*)data, _data, _size);
        }

        ~TestCoroutine() {
            delete[] data;
        }

        Action act() override {
            return socket->sendOneFrameBinaryAsync(oatpp::String((char*)data, size)).next(finish());
        }
    };
    executor->execute<TestCoroutine>(socket, data, size);
}

void WSInstanceListener::sendBroadcastBinaryMessage(const FLAC__byte buffer[], size_t bytes) {
    for(auto &socket : sockets) {
        sendBinaryMessage(socket, buffer, bytes);
    }
}

void WSInstanceListener::onFlacHeader(std::vector<std::vector<FLAC__byte>> _headers) {
    _flacHeaders = _headers;
}

void WSInstanceListener::onFlacFrame(const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame) {
    sendBroadcastBinaryMessage(buffer, bytes);
}