

/******************************************************************************
1 头文件包含
******************************************************************************/
#include "v_id.h"
#include "RnicProcMsg.h"
#include "v_typdef.h"
#include "RnicLog.h"
#include "RnicCtx.h"
#include "RnicInit.h"
#if (FEATURE_ON == MBB_WPG_COMMON)
#include  <linux/wakelock.h>
#endif

#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif

/*****************************************************************************
协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define    THIS_FILE_ID        PS_FILE_ID_RNIC_INITS_C

/*****************************************************************************
2 全局变量定义
*****************************************************************************/
#if (FEATURE_ON == MBB_WPG_COMMON)
struct wake_lock rnic_wakelock;
#endif

/*****************************************************************************
3 函数实现
*****************************************************************************/

VOS_INT RNIC_CCpuResetCallback(
    DRV_RESET_CALLCBFUN_MOMENT          enParam,
    VOS_INT                             iUserData
)
{
    RNIC_CCPU_RESET_IND_STRU           *pstMsg = VOS_NULL_PTR;

    /* 复位前 */
    if (DRV_RESET_CALLCBFUN_RESET_BEFORE == enParam)
    {
        /* 构造消息 */
        pstMsg = (RNIC_CCPU_RESET_IND_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC,
                                                                    sizeof(RNIC_CCPU_RESET_IND_STRU));
        if (VOS_NULL_PTR == pstMsg)
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_CCPU_RESET_IND_STRU: Malloc Msg Failed!\r\n");
            return VOS_ERROR;
        }

        /* 填写消息头 */
        pstMsg->ulReceiverPid               = ACPU_PID_RNIC;
        pstMsg->enMsgId                     = ID_RNIC_CCPU_RESET_START_IND;

        /* 发消息 */
        if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstMsg))
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_CCpuResetCallback: Send Msg Failed!\r\n");
            return VOS_ERROR;
        }

        /* 等待回复信号量初始为锁状态，等待消息处理完后信号量解锁。 */
        if (VOS_OK != VOS_SmP(RNIC_GetResetSem(), RNIC_RESET_TIMEOUT_LEN))
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_CCpuResetCallback: Lock Binary SEM Failed!\r\n");
            RNIC_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

            return VOS_ERROR;
        }

        return VOS_OK;
    }
    /* 复位后 */
    else if (DRV_RESET_CALLCBFUN_RESET_AFTER == enParam)
    {
        /* 记录复位的次数 */
        RNIC_DBG_SAVE_CCPU_RESET_SUCCESS_NUM(1);

        return VOS_OK;
    }
    else
    {
        return VOS_ERROR;
    }
}


VOS_UINT32 RNIC_PidInit (enum VOS_INIT_PHASE_DEFINE enPhase)
{
    RNIC_CTX_STRU                      *pstRnicCtx;

    pstRnicCtx                          = VOS_NULL_PTR;

    switch(enPhase)
    {
        case VOS_IP_LOAD_CONFIG:

            /* 初始化RNIC上下文信息 */
            pstRnicCtx = RNIC_GetRnicCtxAddr();
            RNIC_InitCtx(pstRnicCtx);
#if (FEATURE_ON == MBB_WPG_COMMON)
            wake_lock_init(&rnic_wakelock, WAKE_LOCK_SUSPEND, "rnic_wakeup");
#endif

            /* 给低软注册回调函数，用于C核单独复位的处理 */
            DRV_CCORERESET_REGCBFUNC(NAS_RNIC_FUNC_PROC_NAME,
                                     RNIC_CCpuResetCallback,
                                     0,
                                     ACPU_RESET_PRIORITY_RNIC);

            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
            break;
        default:
            break;
    }

    return VOS_OK;
}




VOS_UINT32 RNIC_FidInit (enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32                          ulRslt;

    switch(enPhase)
    {
        case VOS_IP_LOAD_CONFIG:

            /* 网卡模块注册PID */
            ulRslt = VOS_RegisterPIDInfo(ACPU_PID_RNIC,
                                (Init_Fun_Type)RNIC_PidInit,
                                (Msg_Fun_Type)RNIC_ProcMsg);
            if( VOS_OK != ulRslt )
            {
                RNIC_ERROR_LOG1(ACPU_PID_RNIC, "reg UEPS_PID_RNIC VOS_RegisterPIDInfo FAIL! ulRslt: %d\n", ulRslt);

                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskPrio(ACPU_FID_RNIC, VOS_PRIORITY_P6);

            if( VOS_OK != ulRslt )
            {
                return VOS_ERR;
            }

            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
        case VOS_IP_BUTT:
            break;

        default:
            break;
    }

    return VOS_OK;
}    /* RNIC_FidInit */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
