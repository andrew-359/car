#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Ticker.h>
#include <functional>
#include "Config.h"
#include "network/components/WebSocketHandler.h"
#include "network/components/WiFiConnectionManager.h"
#include "network/components/WebRouteManager.h"

class NetworkManager {
public:
    NetworkManager();
    void begin();
    void loop();

    // --- Публичные коллбэки для событий ---
    std::function<void(AsyncWebSocketClient* client, int throttle, int steer)> onControlCommand;
    std::function<void(AsyncWebSocketClient* client)> onClientDisconnect;
    std::function<void(AsyncWebSocketClient* client)> onClientConnect;

    // Метод для трансляции сообщений всем клиентам
    void broadcastStatus(const String& status);

private:
    AsyncWebServer _server;
    AsyncWebSocket _ws;
    Preferences _preferences;

    // Менеджеры для конкретных задач
    WiFiConnectionManager _wifiManager;
    WebRouteManager _webRouteManager;
    WebSocketHandler _wsHandler;

    void _setupServer();
    void _onNetworkReady(const char* modeName);
};