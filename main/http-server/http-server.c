#include "http-server.h"

#include <esp_log.h>

httpd_handle_t server;

static esp_err_t uri_index_handler(httpd_req_t *req) {
  FILE *f = fopen("/fs/index.html", "r");
  if (f == NULL) {
    ESP_LOGE("uri_index_handler", "Failed to open file for reading");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *file_data = malloc(file_size + 1);
  fread(file_data, 1, file_size, f);
  fclose(f);
  httpd_resp_send(req, file_data, file_size);
  free(file_data);
  return ESP_OK;
}

static esp_err_t handle_ws_req(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    ESP_LOGI("handle_ws_req", "Handshake done, the new connection was opened");
    return ESP_OK;
  }
  return ESP_OK;
}

// Structure for asynchronous response arguments
struct async_resp_arg {
  httpd_handle_t hd; // HTTP server handle
  int fd;            // File descriptor for the client
};

void SendWsMessage(const char *message) {
  const size_t max_clients = 4;
  size_t clients = max_clients;
  int client_fds[max_clients];

  if (httpd_get_client_list(server, &clients, client_fds) == ESP_OK) {
    for (size_t i = 0; i < clients; ++i) {
      int sock = client_fds[i];
      if (httpd_ws_get_fd_info(server, sock) == HTTPD_WS_CLIENT_WEBSOCKET) {
        ESP_LOGI("send_ws_message", "Active client (fd=%d) -> sending message",
                 sock);
        struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
        assert(resp_arg != NULL);
        resp_arg->hd = server;
        resp_arg->fd = sock;

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.payload = (uint8_t *)message;
        ws_pkt.len = strlen(message);
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        if (httpd_ws_send_frame_async(resp_arg->hd, resp_arg->fd, &ws_pkt) != ESP_OK) {
          ESP_LOGE("send_ws_message", "Failed to send message!");
        }
        free(resp_arg);
      }
    }
  }
  else {
    ESP_LOGE("send_ws_message", "httpd_get_client_list failed!");
  }
}

void StartHttpServer(void) {
  httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(httpd_start(&server, &httpd_config));
  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = uri_index_handler,
    .user_ctx = NULL};
  httpd_uri_t ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = handle_ws_req,
    .user_ctx = NULL,
    .is_websocket = true};
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &index_uri));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &ws));
}