#include "FreeRTOS.h"
#include "common_api.h"
#include "audio_task.h"
#include "bsp_custom.h"
#include "audio_extern.h"
#include "luat_rtos.h"
#include "luat_audio_play_ec618.h"
#include "luat_i2s_ec618.h"
#include "ivTTSSDKID_all.h"
#include "ivTTS.h"
#include "luat_gpio.h"
#include "luat_debug.h"

//AIR780E+TM8211开发板配置
#define CODEC_PWR_PIN HAL_GPIO_12
#define CODEC_PWR_PIN_ALT_FUN	4
#define PA_PWR_PIN HAL_GPIO_25
#define PA_PWR_PIN_ALT_FUN	0
#define LED2_PIN	HAL_GPIO_24
#define LED2_PIN_ALT_FUN	0
#define LED3_PIN	HAL_GPIO_23
#define LED3_PIN_ALT_FUN	0
#define LED4_PIN	HAL_GPIO_27
#define LED4_PIN_ALT_FUN	0
#define CHARGE_EN_PIN	HAL_GPIO_2
#define CHARGE_EN_PIN_ALT_FUN	0



extern const unsigned char audiopoweron[];
static luat_rtos_semaphore_t audio_semaphore_handle;
static luat_rtos_task_handle audio_task_handle;


luat_rtos_queue_t audioQueueHandle;
static uint8_t audio_sleep_handler = 0xff;
static HANDLE g_s_delay_timer;

void audio_data_cb(uint8_t *data, uint32_t len, uint8_t bits, uint8_t channels)
{
    // HAL_I2sSrcAdjustVolumn(data, len, 4);
    LUAT_DEBUG_PRINT("cloud_speaker_audio_task %x,%d,%d,%d,%d", data, len, bits, channels);
}
void app_pa_on(uint32_t arg)
{
    luat_gpio_set(PA_PWR_PIN, 1);
}
void audio_event_cb(uint32_t event, void *param)
{
    LUAT_DEBUG_PRINT("cloud_speaker_audio_task %d", event);
    switch (event)
    {
    case MULTIMEDIA_CB_AUDIO_DECODE_START:
        luat_gpio_set(CODEC_PWR_PIN, 1);
		luat_audio_play_write_blank_raw(0, 6, 1);
        break;
    case MULTIMEDIA_CB_AUDIO_OUTPUT_START:
        luat_rtos_timer_start(g_s_delay_timer, 200, 0, app_pa_on, NULL);
        break;
    case MULTIMEDIA_CB_TTS_INIT:
        break;
    case LUAT_MULTIMEDIA_CB_TTS_DONE:
		if (!luat_audio_play_get_last_error(0))
		{
			luat_audio_play_write_blank_raw(0, 1, 0);
		}
		break;
    case MULTIMEDIA_CB_AUDIO_DONE:
        luat_rtos_timer_stop(g_s_delay_timer);
		LUAT_DEBUG_PRINT("audio play done, result=%d!", luat_audio_play_get_last_error(0));
		luat_gpio_set(PA_PWR_PIN, 0);
		luat_gpio_set(CODEC_PWR_PIN, 0);
        luat_rtos_semaphore_release(audio_semaphore_handle);
        break;
    }
}

void audio_task(void *param)
{
    audioQueueData audioQueueRecv = {0};
    uint32_t result = 0;
    while (1)
    {
        if (luat_rtos_queue_recv(audioQueueHandle, &audioQueueRecv, NULL, portMAX_DELAY) == 0)
        {
            LUAT_DEBUG_PRINT("cloud_speaker_audio_task this is play priority %d", audioQueueRecv.priority);
            LUAT_DEBUG_PRINT("cloud_speaker_audio_task this is play playType %d", audioQueueRecv.playType);
            if (audioQueueRecv.priority == MONEY_PLAY)
            {
                
                if (audioQueueRecv.playType == TTS_PLAY)
                {
                    luat_audio_play_tts_text(0, audioQueueRecv.message.tts.data, audioQueueRecv.message.tts.len);
                }
                else if (audioQueueRecv.playType == FILE_PLAY)
                {
                    luat_audio_play_multi_files(0, audioQueueRecv.message.file.info, audioQueueRecv.message.file.count);
                }
            }
            else if (audioQueueRecv.priority == PAD_PLAY)
            {
            }
            luat_rtos_semaphore_take(audio_semaphore_handle, LUAT_WAIT_FOREVER);
            LUAT_DEBUG_PRINT("cloud_speaker_audio_task this is play wait result %d", result);
            if (audioQueueRecv.playType == TTS_PLAY) {
                LUAT_DEBUG_PRINT("cloud_speaker_audio_task free tts data");
                free(audioQueueRecv.message.tts.data);
            }
            else if(audioQueueRecv.playType == FILE_PLAY)
            {
                free(audioQueueRecv.message.file.info);
                LUAT_DEBUG_PRINT("cloud_speaker_audio_task free file data");
            }
        }
    }
    luat_rtos_task_delete(audio_task_handle);
}

void audio_task_init(void)
{
    luat_rtos_timer_create(&g_s_delay_timer);
    luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
	luat_rtos_task_handle task_handle;


	gpio_cfg.pull = LUAT_GPIO_DEFAULT;
	gpio_cfg.pin = PA_PWR_PIN;
	luat_gpio_open(&gpio_cfg);
	gpio_cfg.pin = CODEC_PWR_PIN;
	luat_gpio_open(&gpio_cfg);
	gpio_cfg.alt_fun = CODEC_PWR_PIN_ALT_FUN;
	luat_gpio_open(&gpio_cfg);

    
    luat_audio_play_global_init(audio_event_cb, audio_data_cb, luat_audio_play_file_default_fun, luat_audio_play_tts_default_fun, NULL);
    ivCStrA sdk_id = AISOUND_SDK_USERID_16K;
    luat_audio_play_tts_set_resource(ivtts_16k, sdk_id, NULL);
    luat_i2s_base_setup(0, I2S_MODE_MSB, I2S_FRAME_SIZE_16_16);
    luat_rtos_semaphore_create(&audio_semaphore_handle, 1);

    luat_rtos_queue_create(&audioQueueHandle, 100, sizeof(audioQueueData));
    audioQueueData powerOn = {0};
    powerOn.playType = TTS_PLAY;
    powerOn.priority = MONEY_PLAY;
    char str[] = "正在开机"; 
    powerOn.message.tts.data = malloc(sizeof(str));
    memcpy(powerOn.message.tts.data, str, sizeof(str));
    powerOn.message.tts.len = sizeof(str);
    if (-1 == luat_rtos_queue_send(audioQueueHandle, &powerOn, NULL, 0))
    {
        LUAT_DEBUG_PRINT("cloud_speaker_audio_task start send audio fail");
    }
    int result = luat_rtos_task_create(&audio_task_handle, 2048, 20, "mqtt", audio_task, NULL, NULL);
    LUAT_DEBUG_PRINT("cloud_speaker_audio_task create task result %d", result);
}
