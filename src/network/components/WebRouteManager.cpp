#include "network/components/WebRouteManager.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "tech/Logger.h"
#include "utils/EventBus.h"

WebRouteManager::WebRouteManager(Preferences& preferences) : _preferences(preferences) {}

void WebRouteManager::registerRoutes(AsyncWebServer& server) {
    // Обработчик для сохранения настроек Wi-Fi
    server.on(Config::Web::ROUTE_WIFI_SAVE, HTTP_POST, [this](AsyncWebServerRequest *request) { _handleWifiSave(request); });

    // Обслуживание статических файлов (index.html, wifi.html, main.js, etc.) из LittleFS.
    // Эта строка должна идти ПОСЛЕ определения специфичных роутов (таких как /save-wifi).
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found"); });
}

void WebRouteManager::_handleWifiSave(AsyncWebServerRequest *request) {
    String ssid = request->arg("ssid");
    if (ssid.length() == 0) {
        request->send(400, "text/plain", "SSID не может быть пустым.");
        return;
    }

    String password = request->arg("password");
    Logger::info("Сохранение новых данных Wi-Fi: SSID=%s", ssid.c_str());
    _preferences.begin(Config::Network::PREFERENCES_NAMESPACE, false); // чтение-запись
    _preferences.putString("ssid", ssid);
    _preferences.putString("password", password);
    _preferences.end();

    request->send(LittleFS, "/save_success.html", "text/html");
    // Публикуем событие в шину. Нам не важно, кто его получит.
    GlobalEventBus.publishRestartRequired();
}