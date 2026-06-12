#include "sdkconfig.h"
#include "debug_input.h"

#if CONFIG_DEBUG_INPUT_ENABLED

#include <string.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lwip/sockets.h"

#define TAG "DBGINPUT"

/* Each command from the PC client is 5 bytes:
 *   cmd, x_lo, x_hi, y_lo, y_hi */
typedef struct __attribute__((packed)) {
    uint8_t  cmd;
    uint16_t x;
    uint16_t y;
} input_cmd_t;

#define CMD_TAP     'T'
#define CMD_DOWN    'D'
#define CMD_MOVE    'M'
#define CMD_UP      'U'

#define TAP_HOLD_MS 120

static lv_indev_t       *s_indev;
static portMUX_TYPE       s_lock = portMUX_INITIALIZER_UNLOCKED;
static lv_point_t         s_point;
static lv_indev_state_t   s_state = LV_INDEV_STATE_RELEASED;

static void indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    portENTER_CRITICAL(&s_lock);
    data->point = s_point;
    data->state = s_state;
    portEXIT_CRITICAL(&s_lock);
}

static void set_input(uint16_t x, uint16_t y, lv_indev_state_t state)
{
    portENTER_CRITICAL(&s_lock);
    s_point.x = x;
    s_point.y = y;
    s_state   = state;
    portEXIT_CRITICAL(&s_lock);
}

static esp_err_t recv_all(int sock, void *buf, size_t len)
{
    uint8_t *p = (uint8_t *)buf;
    while (len > 0) {
        int n = recv(sock, p, len, 0);
        if (n <= 0) return ESP_FAIL;
        p   += (size_t)n;
        len -= (size_t)n;
    }
    return ESP_OK;
}

static void handle_client(int sock)
{
    input_cmd_t cmd;
    while (recv_all(sock, &cmd, sizeof(cmd)) == ESP_OK) {
        uint16_t x = cmd.x;
        uint16_t y = cmd.y;

        switch (cmd.cmd) {
        case CMD_TAP:
            set_input(x, y, LV_INDEV_STATE_PRESSED);
            vTaskDelay(pdMS_TO_TICKS(TAP_HOLD_MS));
            set_input(x, y, LV_INDEV_STATE_RELEASED);
            break;
        case CMD_DOWN:
            set_input(x, y, LV_INDEV_STATE_PRESSED);
            break;
        case CMD_MOVE:
            set_input(x, y, LV_INDEV_STATE_PRESSED);
            break;
        case CMD_UP:
            set_input(x, y, LV_INDEV_STATE_RELEASED);
            break;
        default:
            ESP_LOGW(TAG, "unknown command 0x%02x", cmd.cmd);
            return;
        }

        uint8_t ack = 'K';
        send(sock, &ack, 1, 0);
    }
}

static void input_task(void *arg)
{
    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "socket() failed: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port        = htons((uint16_t)CONFIG_DEBUG_INPUT_PORT),
    };

    if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        ESP_LOGE(TAG, "bind() failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, 1) != 0) {
        ESP_LOGE(TAG, "listen() failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "synthetic input server listening on port %d", CONFIG_DEBUG_INPUT_PORT);

    for (;;) {
        struct sockaddr_in src_addr;
        socklen_t addr_len = sizeof(src_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&src_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGW(TAG, "accept() failed: errno %d", errno);
            continue;
        }
        handle_client(sock);
        /* Release any pressed point left over by a disconnected client. */
        set_input(s_point.x, s_point.y, LV_INDEV_STATE_RELEASED);
        close(sock);
    }
}

esp_err_t debug_input_start(void)
{
    s_indev = lv_indev_create();
    if (!s_indev) {
        ESP_LOGE(TAG, "lv_indev_create failed");
        return ESP_ERR_NO_MEM;
    }
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, indev_read_cb);

    if (xTaskCreate(input_task, "debug_input", 4096, NULL, 3, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

#else /* !CONFIG_DEBUG_INPUT_ENABLED */

esp_err_t debug_input_start(void)
{
    return ESP_OK;
}

#endif
