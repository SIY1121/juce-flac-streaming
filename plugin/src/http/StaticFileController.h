#pragma once

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include <fstream>
#include <filesystem>

#include <juce_core/juce_core.h>

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class StaticFileController : public oatpp::web::server::api::ApiController
{
private:
    typedef StaticFileController __ControllerType;

    std::map<const char *, const char *> _extMime = {
        {"html", "text/html"},
        {"js", "text/javascript"},
        {"css", "text/css"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"gif", "image/gif"},
        {"ico", "image/vnd.microsoft.icon"},
        {"svg", "image/svg+xml"},
        {"json", "application/json"},
        {"wasm", "application/wasm"}};

public:
    StaticFileController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    ENDPOINT_ASYNC("GET", "*", F){

        ENDPOINT_ASYNC_INIT(F)

            Action act() override{
                auto filePath = request->getPathTail();
    if (filePath == "")
    {
        filePath = "index.html";
    }

    auto staticRoot = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("static").getFullPathName();

#ifdef _WIN32
    auto sep = '\\';
#else
    auto sep = "/";
#endif

    std::string path = staticRoot.toStdString() + sep + std::string(filePath);

    if (!std::filesystem::exists(std::filesystem::path(path)))
    {
        return _return(controller->createResponse(Status::CODE_404, "Not Found"));
    }

    std::ifstream ifs(path, std::ios::binary);
    auto data = std::string(std::istreambuf_iterator<char>(ifs),
                            std::istreambuf_iterator<char>());

    auto mimeType = "";

    auto mimeTypeResult = std::find_if(controller->_extMime.begin(), controller->_extMime.end(), [&](auto &extMime)
                                       {
            auto suffix = "." + std::string(extMime.first);
            if (path.size() < suffix.size()) return false;
            return std::equal(std::rbegin(suffix), std::rend(suffix), std::rbegin(path)); });

    mimeType = mimeTypeResult->second;

    auto response = controller->createResponse(Status::CODE_200, oatpp::String(data.c_str(), data.size()));
    response->putHeader(oatpp::web::protocol::http::Header::CONTENT_TYPE, mimeType);
    return _return(response);
}
}
;
}
;

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end
