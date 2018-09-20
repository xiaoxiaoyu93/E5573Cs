

#ifndef __LINUXINTERFACE_H_
#define __LINUXINTERFACE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/******************************************************************************
  1 ����ͷ�ļ�����
******************************************************************************/
#if (VOS_OS_VER == VOS_LINUX)
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/init.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
#include <linux/if.h>
#else
#include <linux/if.h>
#endif
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <asm-generic/errno-base.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/netlink.h>
#else
#include "LinuxStub.h"
#endif

/******************************************************************************
  2 �궨��
******************************************************************************/
#ifdef __UT_CENTER__
#define ACPU_FID_RNIC    (0)
#endif

/******************************************************************************
  3 ö�ٶ���
******************************************************************************/


/******************************************************************************
  4 ȫ�ֱ�������
******************************************************************************/


/******************************************************************************
  5 ��Ϣͷ����
******************************************************************************/


/******************************************************************************
  6 ��Ϣ����
******************************************************************************/


/******************************************************************************
  7 STRUCT����
******************************************************************************/
#pragma pack(4)



#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


/******************************************************************************
  8 UNION����
******************************************************************************/


/******************************************************************************
  9 OTHERS����
******************************************************************************/




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* LINUXInterface.h */





