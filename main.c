#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "bsp/board.h"
#include "lwip/apps/mqtt.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

 
#define WIFI_SSID "Thais 2.4"
#define WIFI_PASSWORD "ferracini97"
#define MQTT_SERVER "54.146.113.169" //"broker.emqx.io" available at https://www.emqx.com/en/mqtt/public-mqtt5-broker

//#define PUBLISH_STR_NAME "dennys/button"
#define SUBS_STR_NAME "dennys/motor"

#define EN_A 18 //PWM pin to control motor's speed
#define A1 16   //Control Spin direction of the motor with the H bridge
#define A2 17   //Control Spin direction of the motor with the H bridge

/*
A1  A2  EN_A   Motor State
X   X   0      Steady
1   0   1      ClockWise
1   0   PWM    Speed control ClockWise
0   1   1      Counter-ClockWise
0   1   PWM    Speed control Counter-ClockWise
    
*/

void gpio_app_init(void);
long speed_range_cnv(long x, long in_min, long in_max, long out_min, long out_max);

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_request_cb(void *arg, err_t err);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

struct mqtt_connect_client_info_t mqtt_client_info=
{
  "<RA>/pico_w", /*client id*/
  NULL, /* user */
  NULL, /* pass */
  0,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};

uint16_t motorSpeed =0xFFFF;  //Control the Duty Cycle of the EN_A PWM. Might vary from 0 to 65535 (0xFFFF). 


int main()
{
  stdio_init_all();
  
  gpio_app_init();
  
  
  printf("Iniciando...\n");

  if (cyw43_arch_init()) 
  {
    printf("falha ao iniciar chip wifi\n");
    return 1;
  }

  cyw43_arch_enable_sta_mode();
  printf("Conectando ao %s\n", WIFI_SSID);

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
  {
    printf("Falha ao conectar\n");
    return 1;
  }
  printf("Conectado!!\n");
  
  ip_addr_t addr;
  if(!ip4addr_aton(MQTT_SERVER, &addr)){
    printf("ip error\n");
    return 1;
  }

  printf("Conectando ao MQTT\n");

  mqtt_client_t* cliente_mqtt = mqtt_client_new();

  mqtt_set_inpub_callback(cliente_mqtt, &mqtt_incoming_publish_cb, &mqtt_incoming_data_cb, NULL);

  err_t erro = mqtt_client_connect(cliente_mqtt, &addr, 1883, &mqtt_connection_cb, NULL, &mqtt_client_info); 
  
  if (erro != ERR_OK) 
  {
    printf("Erro de conexão\n");
    return 1;
  }

  printf("Conectado ao MQTT!\n");
  
  
  while(true) 
  {
    sleep_ms(10);
  }

}

/** @brief Initialize PICO_W GPIO according to needs of this application
 * @param void
 * @retval void
 */
void gpio_app_init(void)
{
    gpio_set_function(EN_A, GPIO_FUNC_PWM);
    int fatia_pwm = pwm_gpio_to_slice_num(EN_A);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(fatia_pwm, &config, true);
   
    gpio_init(A1);
    gpio_init(A2);
 
    gpio_set_dir(A1, GPIO_OUT);
    gpio_set_dir(A2, GPIO_OUT);
 
    gpio_put(A1, true);
    gpio_put(A2, false);
    pwm_set_gpio_level(EN_A, motorSpeed);
}

/** @brief Convert one value from one scale to another scale of values.
 * @param x= input value, in_min= Minimum Input value, in_max= Maximum Input value
 * 	   out_min= Minimum Output value, in_max= Maximum Output value
 * @retval long value from out_min to out_max
 */
long speed_range_cnv(long x, long in_min, long in_max, long out_min, long out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("data: %s\n",data);   
    char mqttMotorCtrl[30];
    memset(mqttMotorCtrl, "\0", 30);
    strncpy(mqttMotorCtrl, data, len);
    
    uint8_t motorSpeedPer100=0;

    //ClockWise
    if( (strncmp(mqttMotorCtrl, "h", 1) == 0) || (strncmp(mqttMotorCtrl, "H", 1) == 0) ) {
      pwm_set_gpio_level(EN_A, 0);
      sleep_ms(20);
      gpio_put(A1, true);
      gpio_put(A2, false);
      pwm_set_gpio_level(EN_A, motorSpeed);
    } 
    //Counter-ClockWise 
    else if ( (strncmp(mqttMotorCtrl, "a", 1) == 0) || (strncmp(mqttMotorCtrl, "A", 1) == 0) ) {
      pwm_set_gpio_level(EN_A, 0);
      sleep_ms(20);
      gpio_put(A1, false);
      gpio_put(A2, true);
      pwm_set_gpio_level(EN_A, motorSpeed);
    } 
    //Speed Control 
    else {
        motorSpeedPer100 = atoi(mqttMotorCtrl);
        
        if( motorSpeedPer100 >=0 && motorSpeedPer100 <=100 )
        {
          motorSpeed = speed_range_cnv(motorSpeedPer100, 0, 100, 0, 65535);
          pwm_set_gpio_level(EN_A, motorSpeed);
        }       
    }
}
  
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
  printf("topic %s\n", topic);
}
 
static void mqtt_request_cb(void *arg, err_t err) { 
  printf(("MQTT client request cb: err %d\n", (int)err));
}
 
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  printf(("Conectado ao Brokker MQTT %d\n",  (int)status));
  if (status == MQTT_CONNECT_ACCEPTED) {
    err_t erro = mqtt_sub_unsub(client, SUBS_STR_NAME, 0, &mqtt_request_cb, NULL, 1);

    if (erro == ERR_OK) 
    {
      printf("Inscrito com Sucesso!\n");
    } else {
      printf("Falha ao Inscrever!\n");
    }
  } else {
    printf("Conexão rejeitada!\n");
  }
}
 