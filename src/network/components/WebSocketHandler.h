#pragma once

#include <ESPAsyncWebServer.h>
#include <functional>
#include <map>

class WebSocketHandler {
public:
    WebSocketHandler();

    // Основной метод, который будет вызываться из onEvent
    void handleEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    // --- Публичные коллбэки для событий ---
    std::function<void(AsyncWebSocketClient* client, int throttle, int steer)> onControlCommand;
    std::function<void(AsyncWebSocketClient* client)> onClientDisconnect;
    std::function<void(AsyncWebSocketClient* client)> onClientConnect;

private:
    using WsEventHandler = std::function<void(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)>;
    std::map<AwsEventType, WsEventHandler> _eventHandlers;
};