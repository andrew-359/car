#include "network/components/WebSocketHandler.h"
#include <ArduinoJson.h>
#include "Config.h"
#include "tech/Logger.h"

WebSocketHandler::WebSocketHandler() {

    // Заполняем нашу таблицу поиска
    _eventHandlers[WS_EVT_CONNECT] = [this](AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
        if (onClientConnect) {
            onClientConnect(client);
        }
    };

    _eventHandlers[WS_EVT_DISCONNECT] = [this](AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
        if (onClientDisconnect) {
            onClientDisconnect(client);
        }
    };

    _eventHandlers[WS_EVT_DATA] = [this](AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            StaticJsonDocument<Config::WebSocket::JSON_DOC_SIZE> doc;
            DeserializationError error = deserializeJson(doc, data, len);

            if (error) {
                Logger::warn("Ошибка парсинга JSON от клиента #%u: %s", client->id(), error.c_str());
                return;
            }

            // Проверяем наличие и тип ключей
            if (doc[Config::WebSocket::KEY_THROTTLE].is<int>() && doc[Config::WebSocket::KEY_STEER].is<int>()) {
                int throttle = doc[Config::WebSocket::KEY_THROTTLE] | 0;
                int steer = doc[Config::WebSocket::KEY_STEER] | 0;
                if (onControlCommand) {
                    onControlCommand(client, throttle, steer);
                }
            } else {
                Logger::warn("Некорректный формат команды от клиента #%u", client->id());
            }
        }
    };

    // Можно добавить обработчики для WS_EVT_PONG, WS_EVT_ERROR и т.д.
}

void WebSocketHandler::handleEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    // Ищем обработчик в нашей таблице
    auto it = _eventHandlers.find(type);
    if (it != _eventHandlers.end()) {
        // Если нашли, вызываем его
        it->second(client, arg, data, len);
    } else {
        // Опционально: обработка неизвестных типов событий
    }
}