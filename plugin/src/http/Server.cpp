#include "Server.h"

Server::Server() : juce::Thread("Server")
{

    startThread();
}

Server::~Server()
{
    _isServerRunning = false;
    stopThread(1000);
}

void Server::run()
{
    oatpp::base::Environment::init();

    /* Register Components in scope of run() method */
    AppComponent components;

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create MyController and add all of its endpoints to router */
    router->addController(std::make_shared<WSEndpointController>());
    router->addController(std::make_shared<StaticFileController>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    _server = std::make_unique<oatpp::network::Server>(connectionProvider, connectionHandler);

    /* Priny info about server port */
    OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());
    _isServerRunning = true;
    /* Run server */
    _server->run();
    OATPP_LOGI("MyApp", "Server closed");
    oatpp::base::Environment::destroy();
}

void Server::setAudioFormat(int numChannel, int bitPerSample, int sampleRate, int blockSize)
{
    OATPP_COMPONENT(std::shared_ptr<WSInstanceListener>, wsInstanceListener);

    auto header_callback = [&](const std::vector<std::vector<FLAC__byte>> headers)
    {
        if (_isServerRunning)
            wsInstanceListener->onFlacHeader(headers);
    };
    auto frame_callback = [&](const FLAC__byte buffer[], size_t bytes, uint32_t samples, uint32_t current_frame)
    {
        if (_isServerRunning)
            wsInstanceListener->onFlacFrame(buffer, bytes, samples, current_frame);
    };

    // オーディオフォーマットの変更に応じてEncoderを再生成する
    _encoder = std::make_unique<AsyncFlacEncoder>(numChannel, bitPerSample, sampleRate, blockSize, header_callback, frame_callback);
}

void Server::send(juce::AudioBuffer<float> buffer)
{
    if (!_encoder || !_isServerRunning)
        return;
    _encoder->process(buffer);
}
