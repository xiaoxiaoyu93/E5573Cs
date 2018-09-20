/*
 * Driver for AR8035 PHY
 *
 * Author: jiangdihui <jiangdihui@huawei.com>
 *
 */

#include <linux/phy.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "mbb_net.h"
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
#include <linux/wakelock.h>
#include <linux/netdevice.h>
#include "../ethernet/stmicro/stmmac/stmmac.h"
#include <linux/sched.h>
#endif

#define AR8035_INER_LINK_OK             (1 << 10)
#define AR8035_SPEED1000                (1 << 15)
#define AR8035_SPEED100                 (1 << 14)
#define AR8035_FULLDPLX                 (1 << 13)
#define AR8035_LINKSTAT                 (1 << 10)
#define AR8035_HIB_CTRL                 (1 << 15)

#define AR8035_PHYSR                    0x11
#define AR8035_INER                     0x12
#define AR8035_INSR                     0x13
#define AR8035_DEBUG_ADDR               0x1D
#define AR8035_DEBUG_DATA               0x1E
#define AR8035_DEBUG_HIB                0x0B

#ifdef BSP_CONFIG_BOARD_E5_E5770s
#define AR8035_CONTROL                  0x00
#define AR8035_DEBUG_TXCLKDLY_CTRL      0x05
#define AR8035_DEBUG_EXTLOOPBACK        0x11
#define AR8035_TXCLK_DLY                (1 << 8)
#define AR8035_EXT_LPBK                 (1 << 0)
#endif


#define AR8035_PHY_ID                   0x004dd072   
#define AR8035_PHYID_MASK               0x00ffffff

#ifndef BSP_CONFIG_BOARD_E5_E5770s
#define GPIO_PHY_RESET                  (GPIO_1_8)
#endif
#define PHY_RESET_MTIME  100
#define PHY_RESET_DELAY  10
#ifdef BSP_CONFIG_BOARD_E5_E5770s
#define POWERON_DELAY       (1000)    //phyоƬ�ϵ����ʱ
#define PHY_LPBK_TEST_DELAY (2400)    //����MMI��λ�Ի�������ʱ
#endif
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
#define LAN_TRAFFIC_OFF_TIMEOUT  (600)  //������lan��ʱ��������ⳬʱʱ��
#define LAN_NO_USE_TIMEOUT       (180)  //����������״̬�仯�������������ʱ
#define PHY_POWER_ON             (1)
#define PHY_POWER_OFF            (0)
#define ETH_ROUTE_WAN            (1)
#define ETH_ROUTE_LAN            (0)
#define ETH_ROUTE_UNKNOWN        (0xf)
struct wake_lock wl_eth;   /* eth wakelock */
bool wifi_power_status = true; //��ʶwifi�Ƿ��µ�
#endif

MODULE_DESCRIPTION("Driver for AR8035 PHY");
MODULE_AUTHOR("jiangdihui <jiangdihui@huawei.com>");
MODULE_LICENSE("GPL");

static struct phy_device *s_phydev = NULL;

#ifdef  BSP_CONFIG_BOARD_E5_E5770s
/*****************************************************************************
��������   ar8035_phy_loopback_test
��������:  ar8035оƬphy external loopback����,��MMI���Ե���
��������� ��
����ֵ��   ����ͨ������0��ʧ�ܷ���-1
*****************************************************************************/
int ar8035_phy_loopback_test()
{
    int result = -1;
    int err = 0;
    int reg = 0;
    struct phy_device *phydev = s_phydev;

    //Write debug register 0xB[15]=0 to disable hibernate
    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_HIB);
    if (err < 0)
    {
        printk("ETH %s (line %d) : phy_write error %d\n", __func__, __LINE__, err);
        return err;
    }
    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg &= (~AR8035_HIB_CTRL);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        printk("ETH %s (line %d) : phy_write error %d reg = %d\n",
            __func__, __LINE__, err, reg);
        return err;
    }
    //Write debug register 0x11[0]=1 to enable external loopback
    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_EXTLOOPBACK);
    if (err < 0)
    {
        printk("ETH %s (line %d) : phy_write error %d\n", __func__, __LINE__, err);
        return err;
    }
    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg |= (AR8035_EXT_LPBK);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        printk("ETH %s (line %d) : phy_write error %d reg = %d\n",
            __func__, __LINE__, err, reg);
        return err;
    }
    //Select speed, writer register 0x0=0xA100 to set 100M external loopback
    err = phy_write(phydev, AR8035_CONTROL, 0xa100);
    if (err < 0)
    {
        printk("ETH %s (line %d) : phy_write error %d\n", __func__, __LINE__, err);
        return err;
    }

    //read test result
    msleep(PHY_LPBK_TEST_DELAY);

    reg = phy_read(phydev, AR8035_PHYSR);
    if (reg & AR8035_LINKSTAT)
    {
        printk("ETH %s (line %d) : link up success, loopback test ok!\n",
            __func__, __LINE__);
        return 0;
    }
    else
    {
        printk("ETH %s (line %d) : link up fail, loopback test failed!\n",
            __func__, __LINE__);
        return -1;
    }
}
EXPORT_SYMBOL(ar8035_phy_loopback_test);
#endif
/*****************************************************************************
��������   mbb_get_phy_device
��������:  �ṩ������ģ���ȡphy�豸
��������� ��
����ֵ��   phy�豸
*****************************************************************************/
struct phy_device *mbb_get_phy_device(void)
{
    return s_phydev;
}

/*****************************************************************************
��������   ar8035_phy_hwreset
��������:  phy�豸reset���Ÿ�λ
��������� ��
����ֵ��   ��
*****************************************************************************/
int ar8035_phy_hwreset(void* priv)
{
    gpio_request(GPIO_PHY_RESET, "ar8035");
    #ifdef  BSP_CONFIG_BOARD_E5_E5770s
    //E5770��Ŀ����phyоƬ�ܽ�RST����Ч����λʱӦ����0����1
    gpio_direction_output(GPIO_PHY_RESET, 0);
    mdelay(PHY_RESET_MTIME); 
    gpio_direction_output(GPIO_PHY_RESET, 1);
    #else
    gpio_direction_output(GPIO_PHY_RESET, 1);
    mdelay(PHY_RESET_MTIME);
    gpio_direction_output(GPIO_PHY_RESET, 0);
    #endif
    mdelay(PHY_RESET_DELAY); 
   
    LAN_DEBUG("AR8035 start phy reset PIN:%d\r\n", GPIO_PHY_RESET);

    return 0;
}
  
/*****************************************************************************
��������   ar8035_config_init
��������:  phy�豸��ʼ��
��������� phydev
����ֵ��   NET_RET_OK(0)Ϊok
*****************************************************************************/
static int ar8035_config_init(struct phy_device *phydev)
{
    int reg, err;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    LAN_DEBUG("AR8035 phy driver configinit\r\n");
 #ifndef  BSP_CONFIG_BOARD_E5_E5770s    //E5770��Ŀ��hibernate����
    /*�ر�hibernate�� ����GMAC dma��ʼ����ʧ��*/
    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_HIB);
    if (err < 0)
    {
        return err;
    }

    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg &= (~AR8035_HIB_CTRL);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        return err;
    }
#endif
 
#if 0
    /*����phy�Զ�Э��*/
    reg = phy_read(phydev, MII_BMCR);
    reg |= (BMCR_ANENABLE | BMCR_ANRESTART);
    err = phy_write(phydev, MII_BMCR, reg);
    if (err < 0)
    {
        return err;
    }
    
    reg = phy_read(phydev, AR8035_INER);
    reg = reg | AR8035_INER_LINK_OK;
    err = phy_write(phydev, AR8035_INER, reg);

    if (err < 0)
    {
        return err;
    }    
#endif

    phydev->autoneg = AUTONEG_ENABLE;
#ifdef BSP_CONFIG_BOARD_E5_E5770s
    //E5770��Ŀ��Ҫ����phy�Ĵ�������tx_clk_dly
    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_TXCLKDLY_CTRL);
    if (err < 0)
    {
        return err;
    }
    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg |= (AR8035_TXCLK_DLY);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        return err;
    }
#endif
    s_phydev = phydev;

    /*����ģ��sysfs�ڵ��ʼ��*/
    hw_net_sysfs_init();

    return NET_RET_OK;
}
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
void eth_wake_lock_timeout();
int phy_power_state = PHY_POWER_OFF;  //��ʶphy�ϵ�״̬��1�е�  0�޵�
extern void mbb_eth_state_report(int new_state);
extern void set_eth_lan();
extern int stmmac_restore(struct net_device *dev);
/*****************************************************************************
��������   eth_power_down
��������:  ����GPIO���0ʹphyоƬ�µ�
���������
����ֵ��1 �µ�ɹ�    0 ��ǰ���µ磬�����ظ�����
*****************************************************************************/
int eth_power_down()
{
    //phy power�����µ�״̬ʱ�������ظ����µ�
    if(PHY_POWER_OFF == phy_power_state)
    {
        return 0;
    }
    phy_power_state = PHY_POWER_OFF;
    set_eth_lan();
    gpio_direction_output(GPIO_PHY_RESET, 0);
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 0);
    LAN_DEBUG("ar8035 power down\n");
    return 1;
}
EXPORT_SYMBOL(eth_power_down);
/*****************************************************************************
��������   eth_power_on
��������:  ����GPIO���1ʹphyоƬ�ϵ磬�ϵ����и�λ����
���������
����ֵ��1�ϵ�ɹ�    0��ǰ���ϵ磬�����ظ�����
*****************************************************************************/
int eth_power_on()
{
    eth_wake_lock_timeout(); //ϵͳ�Ӵ���״̬�»��ѣ�eth�ϵ���3���ӳ�ʱ��
    if (PHY_POWER_ON == phy_power_state)
    {
        LAN_DEBUG("phy power is enabled\n");
        return 0;
    }
    else
    {
        phy_power_state = PHY_POWER_ON;
    }
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 1);
    LAN_DEBUG("ar8035 power on\n");
    wifi_power_status = true;
    gpio_direction_output(GPIO_PHY_RESET, 0);
    mdelay(PHY_RESET_MTIME);
    gpio_direction_output(GPIO_PHY_RESET, 1);
    mdelay(PHY_RESET_DELAY);
    ar8035_config_init(s_phydev);
    //phy�ϵ��ʼ���󣬵���stmmac_restore
    struct net_device *dev = __dev_get_by_name(&init_net, "eth0");
    stmmac_restore(dev);
    return 1;
}
EXPORT_SYMBOL(eth_power_on);
/*****************************************************************************
��������   eth_check_wan
��������:  �������ʶ��Ϊlan�ڻ�wan��
���������
����ֵ��   ���ʶ��Ϊwan�ڷ���1��ʶ��Ϊlan�ڷ���0
*****************************************************************************/
extern int eth_check_wan();

/*****************************************************************************
��������   check_lan_stream
��������:  ���lan������
���������
����ֵ��   �������������1������������0
*****************************************************************************/
extern struct net init_net;
extern struct net_device *__dev_get_by_name(struct net *net, const char *name);

int check_lan_stream()
{
    static int eth_rx_packets = 0;
    static int eth_tx_packets = 0;
    struct net_device *dev = __dev_get_by_name(&init_net, "eth0");
    if( (dev->stats.rx_packets != eth_rx_packets)
            && (dev->stats.tx_packets != eth_tx_packets) )
    {
        eth_rx_packets = dev->stats.rx_packets;
        eth_tx_packets = dev->stats.tx_packets;
        return 1;
    }
    else
    {
        return 0;
    }
}

int eth_traffic_check()
{
    int route_state;
    route_state = eth_check_wan();

    if ( ETH_ROUTE_WAN == route_state )
    {
        wake_unlock(&wl_eth);
        return 0;
    }
    else if (ETH_ROUTE_LAN == route_state)
    {
        //��⵽eth��lan����wifi��δ�µ�ʱ��������ֹ����(��wifi��ʱ����ʱ����ýڵ����ͷ���)
        if(wifi_power_status)
        {
            wake_lock(&wl_eth);
        }
        if( 1 == check_lan_stream() )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}
/*****************************************************************************
��������   wifi_unlock_eth
��������:  �ͷ����ڳ��е�wakelock������wifiģ�������ʵ��ETH��WIFIͬ��LANʱһ���µ�
���������
����ֵ��
*****************************************************************************/
void wifi_unlock_eth()
{
    //ֻ����wan��lan��ʱ������wifi�����ͷ�wakelock��
    if ( ETH_ROUTE_LAN == eth_check_wan() )
    {
        wifi_power_status = false;  //wifi׼���µ�ʱ����ô˺������ͷ�������
        LAN_DEBUG("wifi power off timeout, eth as lan release eth_lowpower lock\n");
        wake_unlock(&wl_eth);
    }
}
void eth_wake_lock_timeout()
{
    LAN_DEBUG("set eth wakelock timeout 180\n");
    wake_lock_timeout(&wl_eth, LAN_NO_USE_TIMEOUT * HZ);
}
void eth_wake_unlock()
{
    wake_unlock(&wl_eth);
}
#endif
/*****************************************************************************
��������   ar8035_read_status
��������:  phy�豸��ȡ״̬
��������� phydev
����ֵ��   NET_RET_OK(0)Ϊ��ȡ״̬OK
*****************************************************************************/
int ar8035_read_status(struct phy_device *phydev)
{
    int reg;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    reg = phy_read(phydev, AR8035_PHYSR);    
    if (reg < 0)
    {
        return reg;
    }
    #if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    //phyоƬ�µ�󣬶�ȡ�ļĴ���ֵΪ0xffff����Ҫreturn��ֹ��link up
    if (reg == 0xffff)
    {
        return reg;
    }
    #endif
    
    /*phy�Ƿ�link*/
    if (reg & AR8035_LINKSTAT)
    {
        phydev->link = 1;
    }    
    else
    {
        phydev->link = 0;
    }    

    /*phy��������*/
    if ((reg & AR8035_SPEED100) && !(reg & AR8035_SPEED1000))
    {
        phydev->speed = SPEED_100;
    }
    else if (!(reg & AR8035_SPEED100) && (reg & AR8035_SPEED1000))
    {
        phydev->speed = SPEED_1000;
    }
    else
    {
        phydev->speed = SPEED_10;
    }
    if (reg & AR8035_FULLDPLX)
    {
        phydev->duplex = DUPLEX_FULL;
    }
    else
    {
        phydev->duplex = DUPLEX_HALF;
    }
    
    phydev->pause = 0;
    phydev->asym_pause = 0;

    //LAN_DEBUG("AR8035 phy read status link:%d, speed:%d, duplex:%d\r\n", phydev->link, phydev->speed, phydev->duplex);

    return NET_RET_OK;
}

#define REG_NUM_MAX   32
void SetPhyReg(unsigned int  regnum, unsigned short val)
{
    if (NULL == s_phydev)
    {
        return;
    }

    if (regnum >= REG_NUM_MAX || val > 0xFFFF)
    {       
        return;
    }

    LAN_DEBUG("set phy register %d value: 0x%x\r\n", regnum, val);

    phy_write(s_phydev, regnum, val);
}

int GetPhyReg(unsigned int regnum)
{
    int value; 
    if (NULL == s_phydev)
    {
        return -1;
    }

    if (regnum >= REG_NUM_MAX)
    {       
        return -1;
    }
    
    value = phy_read(s_phydev, regnum);
   
    LAN_DEBUG("get phy register %d value: 0x%x\r\n", regnum, value);

    return value;
}

EXPORT_SYMBOL(SetPhyReg);
EXPORT_SYMBOL(GetPhyReg);

static struct phy_driver ar8035_driver = {
    .phy_id         = AR8035_PHY_ID,
    .phy_id_mask    = AR8035_PHYID_MASK,      
    .name           = "AR8035 Gigabit Phy",
    .features       = PHY_GBIT_FEATURES,
    .flags          = PHY_HAS_INTERRUPT,
    .config_aneg    = genphy_config_aneg,
    .read_status    = ar8035_read_status,
    .config_init    = ar8035_config_init,
    .driver         = { .owner = THIS_MODULE,},
};

/*****************************************************************************
��������   ar8035_init
��������: ar8035 phy�豸ע��
��������� ��
����ֵ��   
*****************************************************************************/
static int __init ar8035_init(void)
{
    int ret;
    #if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    wake_lock_init(&wl_eth, WAKE_LOCK_SUSPEND, "eth_lowpower");
    eth_wake_lock_timeout();
    #endif

    #ifdef BSP_CONFIG_BOARD_E5_E5770s
    gpio_request(GPIO_PHY_LOWPOWEREN, "lowpower_en");
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 1);
    msleep(POWERON_DELAY);
    #endif

    #if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    phy_power_state = PHY_POWER_ON;
    #endif

    LAN_DEBUG("AR8035 phy driver register\r\n");

    ret = phy_driver_register(&ar8035_driver);

    return ret;
}

/*****************************************************************************
��������   ar8035_exit
��������: ar8035 phy�豸�˳�
��������� ��
����ֵ��   
*****************************************************************************/
static void __exit ar8035_exit(void)
{
    phy_driver_unregister(&ar8035_driver);

    hw_net_sysfs_uninit();
}

module_init(ar8035_init);
module_exit(ar8035_exit);

static struct mdio_device_id __maybe_unused ar8035_tbl[] = {
    { AR8035_PHY_ID, AR8035_PHYID_MASK },
    { }
};

MODULE_DEVICE_TABLE(mdio, ar8035_tbl);
