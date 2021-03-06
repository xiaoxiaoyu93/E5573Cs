/*
 * Copyright 2010 HUAWEI Tech. Co., Ltd.
 */


#ifndef __BQ27510_BATTERY_H
 #define __BQ27510_BATTERY_H

#include <linux/power_supply.h>
//#include <linux/mux.h>

//#include <dev_bq27510.h>
#include <linux/gpio.h>
#include <product_config.h>

#define BQ27510_NAME    "bq27510-battery"

#define DRIVER_VERSION          "1.0.0"

#define BQ27510_REG_TEMP    (0x06)
#define BQ27510_REG_VOLT    (0x08)
#define BQ27510_REG_FLAGS   (0x0a)
/*Time to Empty*/
#define BQ27510_REG_TTE     (0x16)
/*Time to Full*/
#define BQ27510_REG_TTF     (0x18)
#if  defined(BSP_CONFIG_BOARD_E5771H_937)
/* State-of-Charge */
#define BQ27510_REG_SOC     (0x20)
#else
/* State-of-Charge */
#define BQ27510_REG_SOC     (0x2C)
#endif
/*Average Current*/
#define BQ27510_REG_AI      (0x14)
/*Remainning Capacity*/
#define BQ27510_REG_RM      (0x10)
 /*Full Charge Capacity*/
#define BQ27510_REG_FCC     (0x12)
/*Standby Current*/
#define BQ27510_REG_SI      (0x1a)
/*Control*/
#define BQ27510_REG_CTRL    (0x00)
/*Control Status*/
#define BQ27510_REG_CTRS    (0x0000)
/*Data Flash Class*/
#define BQ27510_REG_DFCLS      (0x3e)
#define BQ27510_REG_CLASS_ID     (82)
#define BQ27510_REG_QMAX       (0x42)
#define BQ27510_REG_QMAX1      (0x43)
#define BQ27510_REG_FLASH             (0x40)
#define BQ27510_REG_FIRMWARE_ID      (0x0008)
#define BQ27510_REG_FIRMWARE_VERSION (0x0039)

/* both words and bytes are LSB*/
/* Full-charged bit */
#define BQ27510_FLAG_FC               (1<<9)
#define BQ27510_FLAG_DET              (1<<3)
#define BQ27510_OCV_GD                (1<<5)
/* Over-Temperature-Charge bit */
#define BQ27510_FLAG_OTC              (1<<15)
/* Over-Temperature-Discharge bit */
#define BQ27510_FLAG_OTD              (1<<14)
/* State-of-Charge-Threshold 1 bit */
#define BQ27510_FLAG_SOC1             (1<<2)
/* State-of-Charge-Threshold Final bit */
#define BQ27510_FLAG_SOCF             (1<<1)
#define BQ27510_FLAG_LOCK       (BQ27510_FLAG_SOC1 | BQ27510_FLAG_SOCF)
/* Discharging detected bit */
#define BQ27510_FLAG_DSG              (1<<0)

#define CONST_NUM_10                   (10)
#define CONST_NUM_2730               (2730)

//#define ADCSENSOR_PIN                "gpio_170"
#define BATT_INT_PIN                 CHG_BATT_LOW_INT//根据原理图适配
#define BATT_ID_HKADC_ID             CHG_BATT_ID_CHAN//根据原理图适配 通道ID从0 开始

/* added for Firmware upgrade begine */
#define BSP_UPGRADE_FIRMWARE_BQFS_CMD       "upgradebqfs"
#define BSP_UPGRADE_FIRMWARE_DFFS_CMD       "upgradedffs"
/*the whole firmware*/
#define BSP_UPGRADE_FIRMWARE_BQFS_NAME      "/system/etc/coulometer/bq27510_pro.bqfs"
/*gas gauge data info*/
#define BSP_UPGRADE_FIRMWARE_DFFS_NAME      "/system/etc/coulometer/bq27510_pro.dffs"

#define BSP_ROM_MODE_I2C_ADDR               (0x0B)
#define BSP_NORMAL_MODE_I2C_ADDR            (0x55)
#define BSP_FIRMWARE_FILE_SIZE              (400*1024)
#define BSP_I2C_MAX_TRANSFER_LEN            (128)
#define BSP_MAX_ASC_PER_LINE                (400)
#define BSP_ENTER_ROM_MODE_CMD              (0x00)
#define BSP_ENTER_ROM_MODE_DATA             (0x0F00)
#define BSP_FIRMWARE_DOWNLOAD_MODE          (0xDDDDDDDD)
#define BSP_NORMAL_MODE                     (0x00)

#define DISABLE_ACCESS_TIME                 (2000)
/* this number is equal to -20 degree*/
#define BATTERY_TEMPERATURE_COLD            (-20)
/* added for Firmware upgrade end */
#define BAT_VOL_3200    (3200)
#define BAT_VOL_3500    (3500)
#define BAT_VOL_3550    (3550)
#define BAT_VOL_3600    (3600)
#define BAT_VOL_3700    (3700)
#define BAT_VOL_3800    (3800)
#define BAT_VOL_3850    (3850)
#define BAT_VOL_3900    (3900)
#define BAT_VOL_3950    (3950)
#define BAT_VOL_4000    (4000)
#define BAT_VOL_4200    (4200)
#define BAT_VOL_4250    (4250)

//#define BQ27510_DEBUG_FLAG
#if defined(BQ27510_DEBUG_FLAG)
#define BQ27510_DBG(format,arg...)     do { printk(KERN_ALERT format, ## arg);  } while (0)
#define BQ27510_ERR(format,arg...)     do { printk(KERN_ERR format, ## arg);  } while (0)
#define BQ27510_FAT(format,arg...)     do { printk(KERN_CRIT format, ## arg); } while (0)
#else
#define BQ27510_DBG(format,arg...)     do { (void)(format); } while (0)
#define BQ27510_ERR(format,arg...)     do { (void)(format); } while (0)
#define BQ27510_FAT(format,arg...)     do { (void)(format); } while (0)
#endif


 struct bq27510_device_info {
    struct device  *dev;
    int             id;
    struct power_supply    bat;
    struct i2c_client      *client;
    struct delayed_work     notifier_work;
    unsigned long           timeout_jiffies;
    struct iomux_pin        *pin;
    struct hisi_coul_ops *   ops;
};

/*external functions*/
extern int bq27510_battery_temperature(struct bq27510_device_info *di);
extern int bq27510_battery_voltage(struct bq27510_device_info *di);
extern short bq27510_battery_current(struct bq27510_device_info *di);
extern int bq27510_battery_tte(struct bq27510_device_info *di);
extern int bq27510_battery_ttf(struct bq27510_device_info *di);
extern int is_bq27510_battery_full(struct bq27510_device_info *di);
extern int is_bq27510_battery_exist(struct bq27510_device_info *di);
extern int bq27510_battery_status(struct bq27510_device_info *di);
extern int bq27510_battery_health(struct bq27510_device_info *di);
extern int bq27510_battery_capacity(struct bq27510_device_info *di);
extern int bq27510_battery_capacity_level(struct bq27510_device_info *di);
extern int bq27510_battery_technology(struct bq27510_device_info *di);
extern int bq27510_battery_rm(struct bq27510_device_info *di);
extern int bq27510_battery_fcc(struct bq27510_device_info *di);
extern int is_bq27510_battery_reach_threshold(struct bq27510_device_info *di);
extern int bq27510_battery_check_firmware_version(struct bq27510_device_info *di);
extern const char* bq27510_battery_get_firmware_version(struct bq27510_device_info *di);

//用于库仑计信息的log打印输出
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
extern int hisi_bq27510_battery_capacity();//库仑计读取的电量
extern int hisi_bq27510_battery_voltage();//库仑计读取的电压
extern int hisi_bq27510_battery_temperature();//库仑计读取的温度
extern short hisi_bq27510_battery_current();//库仑计读取的电流
extern int hisi_bq27510_battery_fcc();//库仑计读取的满电电量
extern int hisi_bq27510_battery_rm();//库仑计读取的剩余电量
extern int hisi_is_bq27510_battery_full();//库仑计读取的电池是否充满
extern int hisi_bq27510_battery_qmax();
#endif
/*  BAT ID 的识别电池ID类型，E5771的待确认 */
typedef enum 
{
    BAT_NO_PRESENT_STATUS = 0,
    BAT_ID_10K_STATUS,
    BAT_ID_22K_STATUS,
    BAT_ID_39K_STATUS,
    BAT_ID_68K_STATUS,
    BAT_ID_110K_STATUS,
    BAT_ID_200K_STATUS,
    BAT_ID_470K_STATUS,
    BAT_ID_2M_STATUS,
}battery_id_type;

/*  BAT ID 的识别电压范围，E5771的待确认 */
#define BAT_ID_VOLT_0   (0)
#define BAT_ID_VOLT_1   (600)
#define BAT_ID_VOLT_2   (1000)
#define BAT_ID_VOLT_3   (1400)
#define BAT_ID_VOLT_4   (1800)
#define BAT_ID_VOLT_5   (2200)
#define BAT_ID_VOLT_6   (2600)
#define BAT_ID_VOLT_7   (3000)
#define BAT_ID_VOLT_8   (3300)

#define BATTERY_LOW_WARNING      0xA1
#define BATTERY_LOW_SHUTDOWN     0xA2


extern int bq27510_get_gpadc_conversion(int channel_no);
extern int get_battery_id(void);
extern bool bq27510_get_gasgauge_normal_capacity(unsigned int* design_capacity);
extern bool bq27510_get_gasgauge_param_temperature(unsigned int* param_5,unsigned int* param_10);

#endif

