/*
 * Copyright (c) 2022 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LUAT_PM_H
#define LUAT_PM_H

#include "luat_base.h"
/**
 * @defgroup luatos_pm 电源管理接口
 * @{
 */

/* ------------------------------------------------ sleep begin----------------------------------------------- */
#define LUAT_PM_SLEEP_MODE_NONE 0	 ///< 系统处于活跃状态，未采取任何的降低功耗状态
#define LUAT_PM_SLEEP_MODE_IDLE 1	 ///< 空闲模式，该模式在系统空闲时停止 CPU 和部分时钟，任意事件或中断均可以唤醒
#define LUAT_PM_SLEEP_MODE_LIGHT 2	 ///< 轻度睡眠模式，CPU 停止，多数时钟和外设停止
#define LUAT_PM_SLEEP_MODE_DEEP 3	 ///< 深度睡眠模式，CPU 停止，仅少数低功耗外设工作，可被特殊中断唤醒
#define LUAT_PM_SLEEP_MODE_STANDBY 4 ///< 待机模式，CPU 停止，设备上下文丢失(可保存至特殊外设)，唤醒后通常复位
//#define LUAT_PM_SLEEP_MODE_SHUTDOWN	5	///<关断模式，比 Standby 模式功耗更低， 上下文通常不可恢复， 唤醒后复位

/**
 * @brief 设置最深休眠模式标记
 * 
 * @param mode 最深休眠模式
 * @param vote_tag 休眠标记
 * @return int =0成功，其他失败
 */
int luat_pm_set_sleep_mode(int mode, const char *vote_tag);

/**
 * @brief 获取休眠标记对应的休眠模式
 * 
 * @param vote_tag 休眠标记
 * @return int = -1 失败，其他成功
 */
int luat_pm_get_sleep_mode(const char *vote_tag);

typedef void (*luat_pm_sleep_callback_t)(int mode);
/**
 * @brief 注册休眠前回调函数
 * 
 * @param callback_fun 休眠前用户回调
 * @return int =0成功，其他失败
 */
int luat_pm_sleep_register_pre_handler(luat_pm_sleep_callback_t callback_fun);

/**
 * @brief 解注册休眠前回调函数
 * 
 * @return int =0成功，其他失败
 */
int luat_pm_sleep_deregister_pre_handler(void);

/**
 * @brief 注册唤醒后回调函数
 * 
 * @param callback_fun 唤醒后用户回调
 * @return int =0成功，其他失败
 */
int luat_pm_sleep_register_post_handler(luat_pm_sleep_callback_t callback_fun);


/**
 * @brief 解注册唤醒后回调函数
 * 
 * @param callback_fun 唤醒后用户回调
 * @return int =0成功，其他失败
 */
int luat_pm_sleep_deregister_post_handler(void);
/*------------------------------------------------ sleep   end----------------------------------------------- */


/* ----------------------------------------------- wkaeup begin---------------------------------------------- */
/**
 * @brief wakeupPad
 */
typedef enum LUAT_PM_WAKEUP_PAD
{
	LUAT_PMWAKEUP_PAD_0 = 0, /**<wakeupid_0*/
	LUAT_PMWAKEUP_PAD_1, /**<wakeupid_1*/
	LUAT_PMWAKEUP_PAD_2,/**<wakeupid_2*/
	LUAT_PMWAKEUP_PAD_3,/**<wakeupid_3*/
	LUAT_PMWAKEUP_PAD_4,/**<wakeupid_4*/
	LUAT_PMWAKEUP_PAD_5,/**<wakeupid_5*/
	LUAT_PMWAKEUP_LPUART,/**<LPUART 唤醒*/
	LUAT_PMWAKEUP_LPUSB,/**<LPUSB唤醒*/
	LUAT_PMWAKEUP_PWRKEY,/**<PWRKEY唤醒*/
	LUAT_PMWAKEUP_CHARGE,/**<CHARGE唤醒*/
	LUAT_PMWAKEUP_PAD_MAX
}LUAT_PM_WAKEUP_PAD_E;


/**
 * @brief wakeupPad配置参数
 */
typedef struct luat_pm_wakeup_pad_cfg
{
	uint8_t pos_edge_enable;
    uint8_t neg_edge_enable;
	uint8_t pull_up_enable;
    uint8_t pull_down_enable;
}luat_pm_wakeup_pad_cfg_t;
/**
 * @brief 定义wakeupPad中断回调函数类型
 */
typedef void (*luat_pm_wakeup_pad_isr_callback_t)(LUAT_PM_WAKEUP_PAD_E num);

/**
 * @brief 设置wakeupPad中断回调函数
 * 
 * @param callback_fun wakeupPad中断回调函数
 * @return int =0成功，其他失败
 */
int luat_pm_wakeup_pad_set_callback(luat_pm_wakeup_pad_isr_callback_t callback_fun);


/**
 * @brief 配置wakeupPad中断参数，和GPIO20-22输入配置冲突，不可以和GPIO的API同时使用，建议使用GPIO的API
 * 
 * @param enable 中断使能
 * @param source_id wakeupPad
 * @param cfg wakeupPad配置参数
 * @return int =0成功，其他失败
 */
int luat_pm_wakeup_pad_set(uint8_t enable, LUAT_PM_WAKEUP_PAD_E source_id, luat_pm_wakeup_pad_cfg_t *cfg);
/* ------------------------------------------------ wakeup end----------------------------------------------- */


/* ---------------------------------------- power on/off/reboot begin---------------------------------------- */
/**
 * @brief 开机原因
 */
typedef enum LUAT_PM_POWERON_REASON
{
	LUAT_PM_POWERON_REASON_KEY,			 /**<按键开机*/
	LUAT_PM_POWERON_REASON_CHARGER,		 /**<充电开机*/
	LUAT_PM_POWERON_REASON_ALARM,		 /**<闹钟开机*/
	LUAT_PM_POWERON_REASON_REBOOT,		 /**<软件重启开机*/
	LUAT_PM_POWERON_REASON_EXCEPTION,	 /**<异常重启*/
	LUAT_PM_POWERON_REASON_RESET,		 /**<reset 键重启*/
	LUAT_PM_POWERON_REASON_WDT,			 /**<看门狗重启*/
	LUAT_PM_POWERON_REASON_UNKOWN = 0xFF /**<未知原因*/
} LUAT_PM_POWERON_REASON_E;
/**
 * @brief 获取开机原因
 * @param NULL
 * @return @see LUAT_PM_POWERON_REASON_E
 */
int luat_pm_get_poweron_reason(void);
/**
 * @brief 设置设备关机
 */
int luat_pm_poweroff(void);
/**
 * @brief 设备重启
 */
int luat_pm_reboot(void);
/* ----------------------------------------- power on/off/reboot end----------------------------------------- */


/* --------------------------------------------- vbat/vbus begin--------------------------------------------- */
/**
 * @brief 获取充电器状态
 * @param status 充电器状态
 */
int luat_pm_get_vbus_status(uint8_t *status);
/**
 * @brief 电源管理事件变化参数
*/
typedef enum LUAT_PM_EVENT
{
	LUAT_PM_BATTERY_VOLT_EVENT = 0,/**<电池电量发生变化*/
    LUAT_PM_VBUS_STATUS_EVENT/**<VBUS状态发生变化*/
}LUAT_PM_EVENT_E;
/**
 * @brief 定义PM_event事件处理函数
 */
typedef void (*luat_pm_event_callback_t)(LUAT_PM_EVENT_E event, uint32_t param);
/**
 * @brief 定义PM_event事件处理函数回调函数
 * @param luat_pm_event_callback_t 指针函数
 */
int luat_pm_event_register_handler(luat_pm_event_callback_t callback_fun);
/* ---------------------------------------------- vbat/vbus end---------------------------------------------- */


/* ------------------------------------------------ timer begin----------------------------------------------- */
/**
 * @brief 深度睡眠模式下的软件定时器句柄
 */
typedef void * luat_pm_deep_sleep_mode_timer_t;
/**
 * @brief 定义定时时间到后的回调函数类型
 */
typedef LUAT_RT_RET_TYPE (*luat_pm_deep_sleep_mode_timer_callback_t)(LUAT_RT_CB_PARAM);
/**
 * @brief 创建深度睡眠模式下的软件定时器
 * 
 * @param timer_handle 返回定时器句柄
 * @return int =0成功，其他失败
 */
int luat_pm_deep_sleep_mode_timer_create(luat_pm_deep_sleep_mode_timer_t *timer_handle);

/**
 * @brief 删除深度睡眠模式下的软件定时器
 * 
 * @param timer_handle 定时器句柄
 * @return int =0成功，其他失败
 */
int luat_pm_deep_sleep_mode_timer_delete(luat_pm_deep_sleep_mode_timer_t timer_handle);

/**
 * @brief 启动深度睡眠模式下的软件定时器
 * 
 * @param timer_handle 定时器句柄
 * @param timeout 超时时间，单位ms，没有特殊值
 * @param repeat 0不重复，其他重复
 * @param callback_fun 定时时间到后的回调函数
 * @param user_param 回调函数时的最后一个输入参数
 * @return int =0成功，其他失败
 */
int luat_pm_deep_sleep_mode_timer_start(luat_pm_deep_sleep_mode_timer_t timer_handle, uint32_t timeout, uint8_t repeat, luat_pm_deep_sleep_mode_timer_callback_t callback_fun, void *user_param);

/**
 * @brief 停止深度睡眠模式下的软件定时器
 * 
 * @param timer_handle 定时器句柄
 * @return int =0成功，其他失败
 */
int luat_pm_deep_sleep_mode_timer_stop(luat_pm_deep_sleep_mode_timer_t timer_handle);
/*------------------------------------------------ timer   end----------------------------------------------- */
/**@}*/

#endif
