#include "network/NetworkManager.h"
#include "tech/Logger.h"
#include <ArduinoJson.h>

NetworkManager::NetworkManager() 
    : _server(Config::Network::HTTP_PORT), 
      _ws(Config::Web::WS_ENDPOINT),
      _wifiManager(_preferences),
      _webRouteManager(_preferences) 
{
    // Инициализируем коллбэки пустыми функциями по умолчанию.
    // Это гарантирует, что они всегда будут вызываемыми, и избавляет от проверок на null.
    onControlCommand = [](AsyncWebSocketClient*, int, int){};
    onClientDisconnect = [](AsyncWebSocketClient*){};
    onClientConnect = [](AsyncWebSocketClient*){};
}

void NetworkManager::begin() {
    // Подписываемся на события от WiFi-менеджера, чтобы запустить сервер,
    // когда сеть будет готова.
    _wifiManager.onStaConnected = [this]() { _onNetworkReady("STA"); };
    _wifiManager.onApModeStarted = [this]() { _onNetworkReady("AP"); };

    // Запускаем WiFi-менеджер в неблокирующем режиме
    _wifiManager.begin();
}

void NetworkManager::loop() {
    _wifiManager.loop(); // Поддерживаем работу конечного автомата WiFi
    _ws.cleanupClients();
}

void NetworkManager::broadcastStatus(const char* status) {
    StaticJsonDocument<64> doc; // Можно уменьшить размер, т.к. value короткое
    doc["type"] = "status";
    doc["value"] = status;
    char buffer[64];
    size_t len = serializeJson(doc, buffer);
    _ws.textAll(buffer, len);
}

void NetworkManager::_setupServer() {
    // Перенаправляем события от WebSocketHandler к публичным коллбэкам NetworkManager
    _wsHandler.onControlCommand = [this](AsyncWebSocketClient* client, int throttle, int steer) { onControlCommand(client, throttle, steer); };
    _wsHandler.onClientDisconnect = [this](AsyncWebSocketClient* client) { onClientDisconnect(client); };
    _wsHandler.onClientConnect = [this](AsyncWebSocketClient *client) { onClientConnect(client); };

    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        _wsHandler.handleEvent(server, client, type, arg, data, len);
    });
    _server.addHandler(&_ws);

    // Делегируем регистрацию HTTP роутов специализированному менеджеру
    _webRouteManager.registerRoutes(_server);
}

void NetworkManager::_onNetworkReady(const char* modeName) {
    _setupServer();
    _server.begin();
    Logger::info("HTTP и WebSocket сервер запущен в режиме %s.", modeName);
}