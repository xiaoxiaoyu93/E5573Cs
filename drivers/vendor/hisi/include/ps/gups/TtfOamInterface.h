

#ifndef __TTFOAMINTERFACE_H__
#define __TTFOAMINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TTFLink.h"

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
/* R99 Mailbox VTB 译码数据地址 */
extern VOS_UINT32  g_aulR99MailboxVtbAddr[];


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/



/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

#ifndef COMM_ITT
/* Supported by L2 */
extern VOS_VOID WTTF_MAC_R99MailBox_ReadFor3G0MsIsr(VOS_VOID);

extern VOS_VOID WTTF_INTResponseNotify(VOS_VOID);

extern VOS_VOID WTTF_MACDL_DpaDecodeIntNotify(VOS_VOID);

extern VOS_VOID WTTF_HsupaStatusIndINTResponseNotify(VOS_VOID);

extern MsgBlock *           TTF_TraceMsg(MsgBlock *pMsg);

extern VOS_UINT8            TTF_FilterMsg(MsgBlock *msg);

extern VOS_VOID             TTF_TraceModuleSet(VOS_INT32 lModuleNum,VOS_UINT32 *paulModuleId);

#endif

extern VOS_VOID             TTF_TraceSet(VOS_UINT32 ulOption);     /* L2消息跟踪类型设定接口 */


extern VOS_VOID WTTF_MACDL_BbpMasterVoteAllowSleep(VOS_VOID);

extern VOS_VOID WTTF_MACDL_BbpMasterDrxInit(VOS_VOID);

extern VOS_VOID WTTF_MAC_R99MailBox_ReadForIpcHalt(VOS_VOID);



#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of WasOmInterface.h */

