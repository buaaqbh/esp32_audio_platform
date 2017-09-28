/* Console  — BT commands

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"
#include "freertos/FreeRTOS.h"

#include "esp_bt_device.h"
#include "bt_speaker.h"

static struct {
    struct arg_str *name;
    struct arg_end *end;
} setname_args;

static struct {
    struct arg_str *cmd;
    struct arg_end *end;
} a2dp_args;

static int bt_set_device_name(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &setname_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, setname_args.end, argv[0]);
        return 1;
    }

    ESP_LOGI(__func__, "Set BT Device Name to '%s'", setname_args.name->sval[0]);

    /* set up device name */
    esp_bt_dev_set_device_name(setname_args.name->sval[0]);

    return 0;
}

static int bt_a2dp_control(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &a2dp_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, a2dp_args.end, argv[0]);
        return 1;
    }

    if (strncmp(a2dp_args.cmd->sval[0], "start", 5) == 0) {
        ESP_LOGI(__func__, "Start BT A2DP Sink function.");
        bt_speaker_start();
    }
    else if (strncmp(a2dp_args.cmd->sval[0], "stop", 4) == 0) {
        ESP_LOGI(__func__, "Stop BT A2DP Sink function.");
        bt_speaker_stop();
    }
    else if (strncmp(a2dp_args.cmd->sval[0], "connect", 7) == 0) {
        ESP_LOGI(__func__, "BT A2DP Re-Connect to remembered host.");
        bt_speaker_reconnect();
    }
    else if (strncmp(a2dp_args.cmd->sval[0], "disconnect", 10) == 0) {
        ESP_LOGI(__func__, "BT A2DP Disconnect from host.");
        bt_speaker_disconnect();
    }
    else {
        ESP_LOGI(__func__, "Unknow BT A2DP Sink function.");
    }

    return 0;
}

static void register_bt_setname()
{
    setname_args.name = arg_str1(NULL, NULL, "<name>", "device name of BT");
    setname_args.end = arg_end(2);

    const esp_console_cmd_t bt_setname_cmd = {
            .command = "bt-setname",
            .help = "set bt device name",
            .hint = NULL,
            .func = &bt_set_device_name,
            .argtable = &setname_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&bt_setname_cmd) );
}

static void register_bt_a2dp()
{
    a2dp_args.cmd = arg_str1(NULL, NULL, "<command>", "bt a2dp start/stop/connect/disconnect");
    a2dp_args.end = arg_end(2);

    const esp_console_cmd_t bt_setname_cmd = {
            .command = "bt-a2dp",
            .help = "bt a2dp start/stop/connect/disconnect",
            .hint = NULL,
            .func = &bt_a2dp_control,
            .argtable = &a2dp_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&bt_setname_cmd) );
}


void register_bt(void)
{
    register_bt_setname();
    register_bt_a2dp();
}
