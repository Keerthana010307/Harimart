#pragma once

#include <drogon/HttpController.h>

class ChatbotController
    : public drogon::HttpController<ChatbotController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ChatbotController::chat,
        "/api/v1/chatbot",
        drogon::Post
    );

    METHOD_LIST_END

    void chat(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};
