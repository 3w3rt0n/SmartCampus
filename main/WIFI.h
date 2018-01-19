/*
 * WIFI.h
 *
 *  Created on: 20 de nov de 2017
 *      Author: Ewerton L. de Sousa
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

esp_err_t event_handler(void *ctx, system_event_t *event);
void initialise_wifi(void);


#endif /* MAIN_WIFI_H_ */
