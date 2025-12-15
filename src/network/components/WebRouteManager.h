#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <functional>

class WebRouteManager {
public:
    WebRouteManager(Preferences& preferences);
    void registerRoutes(AsyncWebServer& server);

private:
    void _handleWifiSave(AsyncWebServerRequest *request);

    Preferences& _preferences;
};