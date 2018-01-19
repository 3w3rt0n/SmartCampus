/*
 * RMT.c
 *
 *  Created on: 20 de nov de 2017
 *      Author: Ewerton L. de Sousa
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"


uint16_t TV_Ligar[] = {
// ON, OFF (in 1,25's of microseconds)
 4550, 4500,//0
  545, 1690,//1
  545, 1690,//2
  545, 1690,//3
  545, 570,	//4
  545, 570,	//5
  545, 570,	//6
  545, 570,	//7
  545, 570,	//8
  545, 1690,//9
  545, 1690,//10
  545, 1690,//11
  545, 570,	//12
  545, 570,	//13
  545, 570,	//14
  545, 570,	//15
  545, 570,	//16
  545, 570,	//17
  545, 1690,//18*
  545, 570,	//19
  545, 570,	//20
  545, 570,	//21
  545, 570,	//22
  545, 570,	//23
  545, 570,	//24
  545, 1690,//25
  545, 570,	//26*
  545, 1690,//27
  545, 1690,//28
  545, 1690,//29
  545, 1690,//30
  545, 1690,//31
  545, 1690,//32
  545, 1690,//33
  545, 1690	//34
};

uint16_t TV_CANAL_MAIS[] = {
// ON, OFF (in 1,25's of microseconds)
 4550, 4500,//0
  545, 1690,//1
  545, 1690,//2
  545, 1690,//3
  545, 570,	//4
  545, 570,	//5
  545, 570,	//6
  545, 570,	//7
  545, 570,	//8
  545, 1690,//9
  545, 1690,//10
  545, 1690,//11
  545, 570,	//12
  545, 570,	//13
  545, 570,	//14
  545, 570,	//15
  545, 570,	//16
  545, 570,	//17
  545, 1690,//18*
  545, 570,	//19
  545, 570,	//20
  545, 1690,//21
  545, 570,	//22
  545, 570,	//23
  545, 570,	//24
  545, 1690,//25
  545, 570,	//26*
  545, 1690,//27
  545, 1690,//28
  545, 570,//29
  545, 1690,//30
  545, 1690,//31
  545, 1690,//32
  545, 1690,//33
  545, 1690	//34
};

uint16_t TV_CANAL_MENOS[] = {
// ON, OFF (in 1,25's of microseconds)
 4550, 4500,//0 3600
  545, 1690,//1 690 1400
  545, 1690,//2
  545, 1690,//3
  545, 570,	//4
  545, 570,	//5
  545, 570,	//6
  545, 570,	//7
  545, 570,	//8
  545, 1690,//9
  545, 1690,//10
  545, 1690,//11
  545, 570,	//12
  545, 570,	//13
  545, 570,	//14
  545, 570,	//15
  545, 570,	//16
  545, 570,	//17
  545, 570,//18*
  545, 570,	//19
  545, 570,	//20
  545, 1690,//21
  545, 570,	//22
  545, 570,	//23
  545, 570,	//24
  545, 1690,//25
  545, 1690,//26*
  545, 1690,//27
  545, 1690,//28
  545, 570,//29
  545, 1690,//30
  545, 1690,//31
  545, 1690,//32
  545, 1690,//33
  545, 1690	//34
};

void RMT_TV_Ligar(rmt_item32_t* item){

	rmt_item32_t* item_temp = item;
	printf("       [HIGH]   [LOW] :%p\n", item);
	for(int i = 0; i < 68; i=i+2){
		 item_temp->level0 = 1;
		 item_temp->duration0 = TV_Ligar[i];
		 printf( "[%2d] - %4dus | ", i, item_temp->duration0);
		 item_temp->level1 = 0;
		 item_temp->duration1 = TV_Ligar[i+1];
		 printf( "%4dus \n", item_temp->duration1);
		 item_temp++;
	}
}

void RMT_TV_CANAL_MAIS(rmt_item32_t* item){

	rmt_item32_t* item_temp = item;
	printf("       [HIGH]   [LOW] :%p\n", item);
	for(int i = 0; i < 68; i=i+2){
		 item_temp->level0 = 1;
		 item_temp->duration0 = TV_CANAL_MAIS[i];
		 printf( "[%2d] - %4dus | ", i, item_temp->duration0);
		 item_temp->level1 = 0;
		 item_temp->duration1 = TV_CANAL_MAIS[i+1];
		 printf( "%4dus \n", item_temp->duration1);
		 item_temp++;
	}
}

void RMT_TV_CANAL_MENOS(rmt_item32_t* item){

	rmt_item32_t* item_temp = item;
	printf("       [HIGH]   [LOW] :%p\n", item);
	for(int i = 0; i < 68; i=i+2){
		 item_temp->level0 = 1;
		 item_temp->duration0 = TV_CANAL_MENOS[i];
		 printf( "[%2d] - %4dus | ", i, item_temp->duration0);
		 item_temp->level1 = 0;
		 item_temp->duration1 = TV_CANAL_MENOS[i+1];
		 printf( "%4dus \n", item_temp->duration1);
		 item_temp++;
	}
}

/*
 * @brief RMT transmitter initialization
 */
void INIT_RMT_TX(int CANAL, int PINO)
{
    rmt_config_t rmt_tx;
    rmt_tx.rmt_mode = RMT_MODE_TX;
    rmt_tx.channel = CANAL;
    rmt_tx.gpio_num = PINO;
    rmt_tx.mem_block_num = 1; //1 block = 62 entradas de 32bits
    rmt_tx.clk_div = 80;   // divisor = 100 => 80.000.000 / 100 = 800k => 1.25 msec
    rmt_tx.tx_config.loop_en = 0; //false
    rmt_tx.tx_config.carrier_duty_percent = 33;
    rmt_tx.tx_config.carrier_freq_hz = 37900; //38000
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

void imprimir_ITEM(rmt_item32_t* item, int NUM_BITS){

	rmt_item32_t* item_temp = item;
	printf("       [HIGH]   [LOW] :%p\n", item);
	for(int i=0; i<NUM_BITS; i++){
		printf( "[%2d] - %4dus | ", i, item_temp->duration0);
		printf( "%4dus \n", item_temp->duration1);
		item_temp++;
	}
}

void RMT_SEND(rmt_item32_t* item, int NUM_BITS, int CANAL){
	rmt_write_items(CANAL, item, NUM_BITS, true);
	imprimir_ITEM(item, 33);
	rmt_wait_tx_done(CANAL, portMAX_DELAY);
}



