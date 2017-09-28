/*
 * a2dp.h
 *
 *  Created on: 07.05.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_BT_SPEAKER_H_
#define _INCLUDE_BT_SPEAKER_H_

#include "audio_renderer.h"

void bt_speaker_init(renderer_config_t *renderer_config);

void bt_speaker_start(void);

void bt_speaker_stop(void);

void bt_speaker_reconnect(void);

void bt_speaker_disconnect(void);

#endif /* _INCLUDE_BT_SPEAKER_H_ */
