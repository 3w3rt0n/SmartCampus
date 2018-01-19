/*
 * Main.c
 *
 *  Created on: 20 de nov de 2017
 *      Author: Ewerton L. de Sousa
 */

/* Projeto de automação de sala de aula


*/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
//FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
//ESP
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
//DRIVER
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
//ADC
#include "driver/adc.h"
//Touch_PAD
#include "driver/touch_pad.h"
//OUTROS
#include "soc/rmt_reg.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "RMT.h"			//Infrevermelho
#include "DHT22.h"			//Sensor DHT22
#include "WIFI.h"
#include "mqtt.h"

//Pino conectado ao DHT22
#define pinDHT 19

//Pino conectado ao LDR
#define pinLDR ADC1_GPIO34_CHANNEL //Pino 34
//#define pinLDR ADC1_GPIO39_CHANNEL //Pino 39

//Pino dos botões
#define btnLigar 21
#define btnAumentar 22
#define btnDiminuir 23

//Pinos Leds
#define pinLedRMT 25
#define pinLedDHT 26
#define pinLedTEMP 27


//Variaveis globais
float TEMPERATURA = 0;
float HUMIDADE = 0;
float LUMINOSIDADE = 0;
float SOM = 0;

//INFRARED EMISSOR
#define CANAL 0
#define NUM_BITS 34
#define pinLedIR 15

//Items do IR
rmt_item32_t* IR_Ligar;
rmt_item32_t* IR_Aumentar;
rmt_item32_t* IR_Diminuir;

/*
 *
 *
 *
 * TESTE
 *
 *
 */

// wifi settings
#define WIFI_SSID "@#$_="
#define WIFI_PASS "3w3rt0n!@#"

// Event group
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

// Wifi event handler
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {

    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

	case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;

	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;

	default:
        break;
    }

	return ESP_OK;
}


// MQTT connected callback
void mqtt_connected_callback(mqtt_client *client, mqtt_event_data_t *event_data)
{
	printf(" connected!\n");

	// send data every 5 seconds
	while(1) {


		char temp_string[10];
		char hum_string[10];
		char lum_string[10];
		char som_string[10];
		sprintf(temp_string, "%.1f", TEMPERATURA);
		sprintf(hum_string, "%.0f", HUMIDADE);
		sprintf(lum_string, "%.1f", LUMINOSIDADE);
		sprintf(som_string, "%.0f", SOM);
		printf("sending %s°C - %s%% - %sL - %sD\r\n", temp_string, hum_string, lum_string, som_string);
		mqtt_publish(client, "3w3rt0n/feeds/temp", temp_string, strlen(temp_string), 0, 0);
		mqtt_publish(client, "3w3rt0n/feeds/hum", hum_string, strlen(hum_string), 0, 0);
		mqtt_publish(client, "3w3rt0n/feeds/lum", lum_string, strlen(lum_string), 0, 0);
		mqtt_publish(client, "3w3rt0n/feeds/som", som_string, strlen(som_string), 0, 0);
		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}

// MQTT client configuration
mqtt_settings settings = {
	.host = "io.adafruit.com",
	.port = 1883,
	.client_id = "esp32",
	.username = "3w3rt0n",
	.password = "2368d35b571e42f3b9765c0dceb1bee9",
	.clean_session = 0,
	.keepalive = 120,
	.connected_cb = mqtt_connected_callback
};




/*
 *
 *
 * TESTE FIM
 *
 */


/*
 * @brief Tarefa responsável pela leitura da temperatura e umidade do ar, no sensor DHT22
 *
 *  	 __________
 *      |          |	VCC: 3.3 ~ 6V
 *      |   DHT    |
 *      |   22     |
 *      |__________|
 *      |  |   |   |
 *     VCC NC DATA GND
 *
 */
void DHT_task(void *pvParameter)
{
	//DHT
	setDHTgpio(pinDHT);
	printf( "Starting DHT Task\n\n");

	while(1) {

		printf("=== Reading DHT ===\n" );
		gpio_set_level(pinLedDHT, 1);
		int ret = readDHT();

		errorHandler(ret);

		TEMPERATURA = getTemperature();
		HUMIDADE = getHumidity();

		printf( "Umid: %.1f %% \n", HUMIDADE);
		printf( "Temp: %.1f *C \n", TEMPERATURA);

		//sprintf(val, "%.2f", getTemperature());
		vTaskDelay( 1000 / portTICK_RATE_MS );
		// -- wait at least 2 sec before reading again ------------
		// The interval of whole process must be beyond 2 seconds !!
		gpio_set_level(pinLedDHT, 0);
		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
}

/*
 * @brief Tarefa para ler o ADC (LDR)
 *
 */
void ADC_task(void *pvParameter){

	//ADC - LDR
	adc1_config_channel_atten(pinLDR, ADC_ATTEN_11db );

	while(1){
		LUMINOSIDADE = adc1_get_raw(pinLDR);
		printf( "LDR: %.1f \n", LUMINOSIDADE );
		vTaskDelay( 2000 / portTICK_RATE_MS );
	}

	vTaskDelete(NULL);
}

/*
 *
 * @Brief Button
 *
 */
void BUTTON_task(void *pvParameter)
{

	while(1) {

		if(gpio_get_level(btnLigar)){
			//btnLigar
			RMT_SEND(IR_Ligar, NUM_BITS, CANAL);
			gpio_set_level(pinLedRMT, 1);
			printf( "RMT: Ligar  \n");
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}else if(gpio_get_level(btnDiminuir)){
			//btnDiminuir
			gpio_set_level(pinLedRMT, 1);
			RMT_SEND(IR_Diminuir, NUM_BITS, CANAL);
			printf( "RMT: Diminuir  \n");
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}else if(gpio_get_level(btnAumentar)){
			//btnAUMENTAR
			gpio_set_level(pinLedRMT, 1);
			RMT_SEND(IR_Aumentar, NUM_BITS, CANAL);
			printf( "RMT: Aumentar  \n");
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}else{
			gpio_set_level(pinLedRMT, 0);
		}

		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}



void app_main()
{

	//*****************************
	// Configuração das GPIO's
	//-----------------------------
	gpio_set_direction(pinLedRMT, GPIO_MODE_OUTPUT);
	gpio_set_direction(pinLedDHT, GPIO_MODE_OUTPUT);
	gpio_set_direction(pinLedTEMP, GPIO_MODE_OUTPUT);
	gpio_set_direction(btnLigar, GPIO_MODE_INPUT);
	gpio_set_direction(btnAumentar, GPIO_MODE_INPUT);
	gpio_set_direction(btnDiminuir, GPIO_MODE_INPUT);
	gpio_set_pull_mode(btnLigar, GPIO_PULLDOWN_ENABLE);
	gpio_set_pull_mode(btnAumentar, GPIO_PULLDOWN_ENABLE);
	gpio_set_pull_mode(btnDiminuir, GPIO_PULLDOWN_ENABLE);
	gpio_set_level(pinLedRMT, 0);
	gpio_set_level(pinLedDHT, 0);
	gpio_set_level(pinLedTEMP, 0);
	//*****************************
	// FIM Configuração das GPIO's
	//-----------------------------

	//********************************
	// Inicialização dos vetores do IR
	//--------------------------------
	size_t size = (sizeof(rmt_item32_t) * 120 * 100);
	IR_Ligar = (rmt_item32_t*) malloc(size);
	memset((void*) IR_Ligar, 0, size);
	IR_Aumentar = (rmt_item32_t*) malloc(size);
	memset((void*) IR_Aumentar, 0, size);
	IR_Diminuir = (rmt_item32_t*) malloc(size);
	memset((void*) IR_Diminuir, 0, size);
	// Inicialização do RMT
	INIT_RMT_TX(CANAL, pinLedIR);
	RMT_TV_Ligar(IR_Ligar);
	RMT_TV_CANAL_MAIS(IR_Aumentar);
	RMT_TV_CANAL_MENOS(IR_Diminuir);
	//********************************
	// FIM inicialização IR
	//--------------------------------


	//************************************************************************************
	// 					FreeRTOS
	//************************************************************************************

	//Novo - 16-01-2018
	//**************************************
	// NVS - Necessario para o DHT22 e Wifi
	//**************************************
	// Initialize NVS.
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK( err );
	//######################################
	// Fim NVS
	//######################################

	//*****************************
	// BUTTON
	//*****************************
	xTaskCreate( BUTTON_task, "BUTTON_task", 2048, NULL, 5, NULL );
	//#############################
	// Fim
	//#############################


	//*****************************
	// ADC
	//*****************************
	xTaskCreate( ADC_task, "ADC_task", 2048, NULL, 5, NULL );
	//#############################
	// Fim
	//#############################


	//*****************************
	// DHT22
	//*****************************
	xTaskCreate( DHT_task, "DHT_task", 2048, NULL, 5, NULL );
	//#############################
	// Fim DHT22
	//#############################

	// Novo 16-01-2018
	//*****************************
	// WiFi
	//*****************************
	//initialise_wifi();
	//##############################
	// Fim
	//##############################

	// Novo 16-01-2018
	//*****************************
	// MQTT
	//*****************************
	wifi_event_group = xEventGroupCreate();
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	wifi_config_t wifi_config = {
		.sta = {
			.ssid = WIFI_SSID,
			.password = WIFI_PASS,
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	printf("waiting for wifi network...");
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	printf(" connected!\n");

	// start the MQTT client
	printf("Connecting to the MQTT server... ");
	mqtt_start(&settings);
	//##############################
	// Fim
	//##############################

}

