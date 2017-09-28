// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "../bt_speaker/bt_app_av.h"
#include "../bt_speaker/bt_app_core.h"
#include "bt_speaker.h"

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
    BT_APP_EVT_STACK_DOWN,
};

bt_av_state_t bt_state;

/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param) {
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    switch (event) {
        case BT_APP_EVT_STACK_UP: {
            /* set discoverable and connectable mode, wait to be connected */
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
            break;
        }
        case BT_APP_EVT_STACK_DOWN: {
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_NONE);
            break;
        }
        default:
            ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
            break;
    }
}

void bt_speaker_init(renderer_config_t *renderer_config)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
        return;
    }

    /* init renderer */
    renderer_init(renderer_config);

    memset(&bt_state, 0, sizeof(struct bt_av_state));
    bt_state.audio_state = ESP_A2D_AUDIO_STATE_STOPPED;

    /* set up device name */
    esp_bt_dev_set_device_name("ESP32_AUDIO");

    /* initialize A2DP sink */
    esp_a2d_register_callback(&bt_app_a2d_cb);
    esp_a2d_register_data_callback(bt_app_a2d_data_cb);
    esp_a2d_sink_init();

    /* initialize AVRCP controller */
    esp_avrc_ct_init();
    esp_avrc_ct_register_callback(bt_app_rc_ct_cb);

    bt_app_task_start_up();
}

void bt_speaker_start(void)
{
    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
}

void bt_speaker_stop(void)
{
    bt_speaker_disconnect();
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_DOWN, NULL, 0, NULL);
}

void bt_speaker_reconnect(void)
{
    uint8_t *bda = bt_state.host;

    if (bt_state.connection_state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        if ((bda[0] == 0) && (bda[1] == 0) && (bda[2] == 0) && (bda[3] == 0) && (bda[4] == 0) && (bda[5] == 0)) {
            ESP_LOGI(BT_AV_TAG, "%s, no host remembered", __func__);
        }
        else {
            ESP_LOGI(BT_AV_TAG, "%s, re-connected to host: [%02x:%02x:%02x:%02x:%02x:%02x]",
                     __func__, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
            esp_a2d_sink_connect(bt_state.host);
        }
    }
    else if (bt_state.connection_state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        ESP_LOGI(BT_AV_TAG, "%s, already connected to host: [%02x:%02x:%02x:%02x:%02x:%02x]",
                 __func__, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    }
}

void bt_speaker_disconnect(void)
{
    uint8_t *bda = bt_state.host;

    if (bt_state.connection_state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        ESP_LOGI(BT_AV_TAG, "%s, disconnect from host: [%02x:%02x:%02x:%02x:%02x:%02x]",
                 __func__, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        esp_a2d_sink_disconnect(bt_state.host);
    }
}
