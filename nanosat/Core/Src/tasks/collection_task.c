#include "adc.h"
#include "tim.h"
#include "main.h"
#include <string.h>

#include "sync_globals.h"

#include "tasks/collector_task.h"
#include "tasks/event_task.h"
#include "tasks/flash_task.h"


#include "rgb_led.h"
#include "mod_led.h"
#include "buzzer.h"
#include "dht.h"
#include "adc_part.h"
#include "DateTime.h"
#include "sensor_data.h"

#define LIGHT_MAX_VALUE 255
#define TRUE 1
#define FALSE 0


static float get_voltage(uint16_t adc_value);
static uint8_t map_to_percentage(uint16_t adc_value, uint16_t max_value);
static uint8_t is_in_range(CollectorSetting* collector_setting, SensorData* sensor);
static uint8_t read_all_sensors(DHT* dht, ADC_Part* pot, ADC_Part* light, CelsiusAndHumidity* ch, uint16_t* pot_val, uint16_t* light_val);
static void init_components(DHT* dht, ADC_Part* pot, ADC_Part* light, Led* red_led, Led* blue_led);
static void process_sensor_data(SensorData* data, const CelsiusAndHumidity* ch, uint16_t pot_val, uint16_t light_val, DateTime* datetime);
static void handle_event_transition(SensorData* data, uint8_t prev_mode);

static void update_setting(UpdateSetting const * setting, CollectorSetting* limits);

static Led red_led;
static Led blue_led;
volatile uint8_t stop_buzz = 0;

void collection_task(void* context)
{
    CollectorSetting sensor_limits = *((CollectorSetting*) context);

    DHT dht;
    ADC_Part potentiometer, light_sensor;
    CelsiusAndHumidity ch;
    DateTime datetime;
    SensorData current_data;
    uint16_t pot_val, light_val;
    uint32_t delay;

    init_components(&dht, &potentiometer, &light_sensor, &red_led, &blue_led);

    RGB_LED_Set_Color(COLOR_GREEN);


    while (1) {

    	for(uint8_t i = 0; i< osMessageQueueGetCount(g_sensor_limits_change); i++)
    	{
    		UpdateSetting updated_setting;
    		osStatus_t status = osMessageQueueGet(g_sensor_limits_change, &updated_setting, 0, 10);
       		if(status == osOK){
       			update_setting(&updated_setting, &sensor_limits);
			}
    	}



        RTC_ReadDateTime(&datetime);

        if (read_all_sensors(&dht, &potentiometer, &light_sensor, &ch, &pot_val, &light_val)) {
            process_sensor_data(&current_data, &ch, pot_val, light_val, &datetime);

            current_data.mode = is_in_range(&sensor_limits, &current_data) == TRUE ? OK_MODE : ERROR_MODE;

            if(current_data.mode == ERROR_MODE){
                if(current_data.volage < sensor_limits.safe_voltage){
                	current_data.mode = SAFE_MODE;
                	RGB_LED_Set_Color(COLOR_YELLOW);
                }
                else{
                	RGB_LED_Set_Color(COLOR_RED);
                }
                if(stop_buzz  == 0)
                {
                	buzzer_start();
                }
            }
            else{
            	buzzer_end();
            	stop_buzz = 0;
				RGB_LED_Set_Color(COLOR_GREEN);

            }

            if (g_latest_sensor_data.mode != UNINTILIZED_MODE && g_latest_sensor_data.mode != current_data.mode) {
                handle_event_transition(&current_data, g_latest_sensor_data.mode);
            }



            g_latest_sensor_data = current_data;


            //send to logger
    		osMessageQueuePut(g_sensor_queue, &g_latest_sensor_data, 0, 0);


            if(current_data.mode == SAFE_MODE){
            	 delay = (uint32_t) ( sensor_limits.delay) * 1000 * 2;

            }
            else{
            	delay = (uint32_t) ( sensor_limits.delay) * 1000 ;
            }

            osDelay(delay);
        }
    }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == SW1_Pin) {
		buzzer_end();
		stop_buzz = 1;
	}
}

static void update_setting(UpdateSetting const * setting,CollectorSetting* limits )
{
    switch (setting->update_attribute) {
        case UPDATE_HUMIDITY_LIMIT:
        	memcpy(&limits->min_humidity, setting->buffer, sizeof(limits->min_humidity));
        	printf("new humidity: %u\r\n", limits->min_humidity);
            break;

        case UPDATE_VOLTAGE_LIMIT:

        	memcpy(&limits->safe_voltage, setting->buffer, sizeof(limits->safe_voltage));
        	printf("new voltage: %f \r\n", limits->safe_voltage);
            break;

        case UPDATE_LIGHT_LIMIT:
        	memcpy(&limits->min_light, setting->buffer, sizeof(limits->min_light));
        	printf("new light: %u \r\n", limits->min_light);
            break;

        case UPDATE_MIN_TEMP_LIMIT:
        	memcpy(&limits->min_temp, setting->buffer, sizeof(limits->min_temp));
        	printf("new min temp: %u \r\n", limits->min_temp);
            break;
        case UPDATE_MAX_TEMP_LIMIT:
        	memcpy(&limits->max_temp, setting->buffer, sizeof(limits->max_temp));
        	printf("new max_temp: %u \r\n", limits->max_temp);
            break;

        default:
            // Handle unexpected value
            break;
    }
}


static void init_components(DHT* dht, ADC_Part* pot, ADC_Part* light, Led* red_led, Led* blue_led)
{
    init_DHT(dht, DHT_GPIO_Port, DHT_Pin, &htim6);
    init_Buzzer(&htim3, TIM_CHANNEL_1);
    ADC_init(pot, &hadc1);
    ADC_init(light, &hadc2);

    led_create(blue_led, LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    led_create(red_led, LED_RED_GPIO_Port, LED_RED_Pin);
    RGB_LED_init(RGB_RED_GPIO_Port, RGB_RED_Pin, RGB_GREEN_GPIO_Port, RGB_GREEN_Pin, RGB_BLUE_GPIO_Port, RGB_BLUE_Pin);
}

static uint8_t read_all_sensors(DHT* dht, ADC_Part* pot, ADC_Part* light, CelsiusAndHumidity* ch, uint16_t* pot_val, uint16_t* light_val)
{
    uint8_t success = TRUE;

    if (read_DHT(dht, ch) != DHT_OK) {
        //printf("DHT fail\r\n");
        success = FALSE;
    }

    if (ADC_read_data(pot, pot_val) != HAL_OK) {
        printf("Pot error\r\n");
        success = FALSE;
    }

    if (ADC_read_data(light, light_val) != HAL_OK) {
        printf("Light error\r\n");
        success = FALSE;
    }

    return success;
}

static void process_sensor_data(SensorData* data, const CelsiusAndHumidity* ch, uint16_t pot_val, uint16_t light_val, DateTime* datetime)
{
    data->timestamp = datetime_to_timestamp(datetime);
    data->volage = get_voltage(pot_val);
    data->light = map_to_percentage(light_val, LIGHT_MAX_VALUE);
    data->humid = ch->humidity_integral;
    data->temp = ch->tempature_integral;
}


static void handle_event_transition(SensorData* data, uint8_t prev_mode)
{
    EventData event_data;
    event_data.timestamp = data->timestamp;

    if (prev_mode == OK_MODE && data->mode == ERROR_MODE) {
        event_data.event = EVENT_OK_TO_ERROR;

    } else if (prev_mode == OK_MODE && data->mode == SAFE_MODE){
        event_data.event = EVENT_OK_TO_SAFE;

    } else if (prev_mode == SAFE_MODE && data->mode == ERROR_MODE) {
        event_data.event = EVENT_SAFE_TO_ERROR;

    } else if (prev_mode == SAFE_MODE && data->mode == OK_MODE) {
        event_data.event = EVENT_SAFE_TO_OK;
    }


    osMessageQueuePut(g_event_queue, &event_data, 0, 0);
}

static uint8_t is_in_range(CollectorSetting* cs, SensorData* sensor)
{
    uint8_t in_range = TRUE;

    if (sensor->humid < cs->min_humidity) {
        printf("Humidity %u is below minimum %u \r\n", sensor->humid, cs->min_humidity);
        in_range = FALSE;
    }

    if (sensor->temp < cs->min_temp || sensor->temp > cs->max_temp) {
        printf("Temperature %u is out of range (%u - %u) \r\n", sensor->temp, cs->min_temp, cs->max_temp);
        in_range = FALSE;
    }

    if (sensor->light < cs->min_light) {
        printf("Light %u is below minimum %u\r\n", sensor->light, cs->min_light);
        in_range = FALSE;
    }

    if (sensor->volage < cs->safe_voltage) {
        printf("Voltage %.2f is below safe minimum %.2f\r\n", sensor->volage, cs->safe_voltage);
        in_range = FALSE;
    }

    return in_range;
}

static uint8_t map_to_percentage(uint16_t adc_value, uint16_t max_value)
{
    return (adc_value * 100) / max_value;
}

static float get_voltage(uint16_t adc_value)
{
    return ((float)adc_value * 3.3f) / 4095.0f;
}
