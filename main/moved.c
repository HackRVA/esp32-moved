/* PSMoved Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "nvs_flash.h"

#include "lwip/sockets.h"

#include "psmove_protocol.h"



//Header file and defines for led control mostly ripped from the application example from esp-idf docs
#include "driver/ledc.h"


#define LEDC_SELECTED_TIMER_BIT_DEPTH LEDC_TIMER_13_BIT

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       CONFIG_GPIO_R
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0  //R
#define LEDC_HS_CH1_GPIO       CONFIG_GPIO_G
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1  //G
#define LEDC_HS_CH2_GPIO       CONFIG_GPIO_B
#define LEDC_HS_CH2_CHANNEL    LEDC_CHANNEL_2  //B

#define LEDC_CH_NUM       (3)



/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID            CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS            CONFIG_WIFI_PASSWORD


static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const static int CONNECTED_BIT = BIT0;

const static char *TAG = "MOVED";

static void initilize_leds(void) {
   
  ledc_channel_config_t ledc_ch[LEDC_CH_NUM] = {
    {
	.channel    = LEDC_HS_CH0_CHANNEL,
	.gpio_num   = LEDC_HS_CH0_GPIO,
	.speed_mode = LEDC_HS_MODE,
	.timer_sel  = LEDC_HS_TIMER,
	.duty       = 0, //Start OFF
	.intr_type  = LEDC_INTR_DISABLE //Disable Fade
    },
    {
	.channel    = LEDC_HS_CH1_CHANNEL,
	.gpio_num   = LEDC_HS_CH1_GPIO,
	.speed_mode = LEDC_HS_MODE,
	.timer_sel  = LEDC_HS_TIMER,
	.duty       = 0, //Start OFF
	.intr_type  = LEDC_INTR_DISABLE //Disable Fade
    },
    {
	.channel    = LEDC_HS_CH2_CHANNEL,
	.gpio_num   = LEDC_HS_CH2_GPIO,
	.speed_mode = LEDC_HS_MODE,
	.timer_sel  = LEDC_HS_TIMER,
	.duty       = 0, //Start OFF
	.intr_type  = LEDC_INTR_DISABLE //Disable Fade
    }
  };
  
  
  ledc_timer_config_t ledc_timer = {
      .bit_num = LEDC_TIMER_13_BIT, //set timer counter bit number
      .freq_hz = 5000,              //set frequency of pwm
      .speed_mode = LEDC_HS_MODE,   //timer mode,
      .timer_num = LEDC_HS_TIMER    //timer index
  };
  //configure timer0 for high speed channels
  ledc_timer_config(&ledc_timer);
  
  //configure each LED channel
  int i;
  for(i = 0; i < LEDC_CH_NUM; ++i) {
    ledc_channel_config(&ledc_ch[i]);
  }
}

static void setRGB(unsigned char r,
		   unsigned char g,
		   unsigned char b) {
  //bitshift our 8 bit number to fit the counter duty cycle
  
  uint32_t r_int = r << (LEDC_SELECTED_TIMER_BIT_DEPTH - sizeof(r));
  uint32_t g_int = g << (LEDC_SELECTED_TIMER_BIT_DEPTH - sizeof(g));
  uint32_t b_int = b << (LEDC_SELECTED_TIMER_BIT_DEPTH - sizeof(b));
  
  //TODO: Scale the shifted bits to fit maximumm and minimum possible values
  
  ledc_set_duty(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL, r_int); 
  ledc_set_duty(LEDC_HS_MODE,LEDC_HS_CH1_CHANNEL, g_int); 
  ledc_set_duty(LEDC_HS_MODE,LEDC_HS_CH2_CHANNEL, b_int);
  
  ledc_update_duty(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL);
  ledc_update_duty(LEDC_HS_MODE,LEDC_HS_CH1_CHANNEL);
  ledc_update_duty(LEDC_HS_MODE,LEDC_HS_CH2_CHANNEL);
}

static int serialize_moved_report_packet(PSMove_Data_Input* packet) {
  //Currently let us fill this with dummy values
  memset(packet,0,sizeof(PSMove_Data_Input));
  
  return 0; //Success
}

static int parse_moved_client_update(const char* buf, size_t len, PSMove_Data_LEDs* dest) {
  if(len > sizeof(PSMove_Data_LEDs)) {
    ESP_LOGE(TAG, "Recieved update packet too large to populate internal buffer");
    return -1;
  }
  memset(dest,0,sizeof(PSMove_Data_Input));
  
  //Let us try to populate without care for endianess
  memcpy(dest, buf, len);
  
  return 0;
}

static void handle_moved_client_update(PSMove_Data_LEDs* leds) {
  //UPDATE RGB led values and set vibratio motor intensity
  if(leds == 0) {
    ESP_LOGE(TAG,"Null pointer setting led and motor state");
    return;
  }
  if(leds->type != PSMove_Req_SetLEDs) {
    ESP_LOGE(TAG,"Recieved unsuppored update packet from moved client!");
    return;
  }
  ESP_LOGI(TAG,"Changing R Led PWM duty cycle to %i/255",leds->r);
  ESP_LOGI(TAG,"Changing G Led PWM duty cycle to %i/255",leds->g);
  ESP_LOGI(TAG,"Changing B Led PWM duty cycle to %i/255",leds->b);
  ESP_LOGI(TAG,"Changing Vibration Motor PWM duty cycle to %i/255",leds->rumble2);
}


static void moved_thread(void *p)
{
    fd_set socketfds;
    int fd;
    struct sockaddr_in listen_addr;
    struct sockaddr_in remote_addr;
    socklen_t addrlen = sizeof(remote_addr);
    
    memset((char*)&listen_addr, 0, sizeof(listen_addr));
    
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(CONFIG_MOVED_PORT);
    
    unsigned char in_buffer[52];
    
    while(1) {
      xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,
			  false,true,portMAX_DELAY);
      ESP_LOGI(TAG, "Connected to AP: " CONFIG_WIFI_SSID);
      
      fd = socket(listen_addr.sin_family,SOCK_DGRAM,0);
      if(fd < 0) {
	ESP_LOGE(TAG, "Cannot create socket");
	continue;
      }
      int binding = bind(fd,(struct sockaddr *)&listen_addr,sizeof(listen_addr));
      if(binding < 0) {
	ESP_LOGE(TAG, "Cannot bind to port: %d", CONFIG_MOVED_PORT);
	continue;
      }
      while(1) {
	ESP_LOGI(TAG, "Waiting for udp packet");
	int recvlen = recvfrom(fd, in_buffer, 52, 0, (struct sockaddr *)&remote_addr, &addrlen);
	ESP_LOGI(TAG, "Got packet!");
      }
      
    }
    vTaskDelete(NULL);
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void wifi_conn_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    
    wifi_conn_init();
    
    initilize_leds();
    
    xTaskCreate(moved_thread, "moved", 2048, NULL, 5, NULL);
}
