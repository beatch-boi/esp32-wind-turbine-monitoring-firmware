#ifndef WIND_TURBINE_HTTP_SERVER_H_
#define WIND_TURBINE_HTTP_SERVER_H_

#include <esp_http_server.h>

void StartHttpServer(void);

void SendWsMessage(const char *message);

#endif  // WIND_TURBINE_HTTP_SERVER_H_