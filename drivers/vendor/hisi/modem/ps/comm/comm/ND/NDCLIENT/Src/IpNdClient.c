
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

#ifndef __IPNDCLIENT_C__
#define __IPNDCLIENT_C__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include  "IpComm.h"
#include  "IpIpmGlobal.h"
#include  "IpNdClient.h"
#include  "ApsNdInterface.h"
#include  "CdsNdInterface.h"
#include  "AdsNdInterface.h"
#include  "TTFMem.h"
/*#include  "DrvInterface.h"*/

/*lint -e767*/
#define    THIS_FILE_ID        PS_FILE_ID_IPNDCLIENT_C
/*lint +e767*/

/*****************************************************************************
  #pragma pack(*)    设置字节对齐方式
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
IP_ND_MSG_STRU_UNION    g_stNdMsgBuffer;

VOS_UINT32              g_aulNdClientEntityCnt[MODEM_ID_BUTT] = {0};
IP_NDCLIENT_ENTITY_STRU g_stNdClientEntity[IP_MAX_NDCLIENT_ENTITY_NUM + 1];

IP_NDCLIENT_STATISTIC_INFO_STRU g_stNdClientStatInfo;


VOS_UINT32              g_ulNdRtrSolicitationInterval   = 4000; /* milliseconds */
VOS_UINT32              g_ulNdMaxRtrSolicitaion         = 20;

VOS_UINT32      g_ulNdClientMtu = 1500;

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
extern VOS_VOID IP_NdClient_ReleaseResendResourse
(
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity
);

VOS_VOID IP_StartFilterIpv6(VOS_UINT16 usModemId);
VOS_VOID IP_StopFilterIpv6(VOS_UINT16   usModemId);



VOS_VOID IP_NdClient_InitEntityNum(VOS_VOID)
{
    IP_MEM_SET(g_aulNdClientEntityCnt,0,sizeof(g_aulNdClientEntityCnt));
    return;
}



VOS_VOID IP_NdClient_AddEntityNum(VOS_UINT16 usModemId)
{
    if (usModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,"IP_NdClient_AddEntityNum: Input ModemId Error.",usModemId);
        return;
    }

    g_aulNdClientEntityCnt[usModemId] ++;

    if (1 == g_aulNdClientEntityCnt[usModemId])
    {
        IP_StartFilterIpv6(usModemId);
        IPND_INFO_LOG1(NDCLIENT_TASK_PID,"IP_NdClient_AddEntityNum: Start IPv6 Filter.",usModemId);
    }

    return;
}


VOS_VOID IP_NdClient_SubEntityNum(VOS_UINT16 usModemId)
{
    if (usModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,"IP_NdClient_SubEntityNum: Input ModemId Error.",usModemId);
        return;
    }

    if (0 == g_aulNdClientEntityCnt[usModemId])
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,"IP_NdClient_SubEntityNum: Ative ND Entity Already 0.",usModemId);
        return;
    }

    g_aulNdClientEntityCnt[usModemId] --;

    if (0 == g_aulNdClientEntityCnt[usModemId])
    {
        IP_StopFilterIpv6(usModemId);
        IPND_INFO_LOG1(NDCLIENT_TASK_PID,"IP_NdClient_SubEntityNum: Stop IPv6 Filter.",usModemId);
    }

    return;
}


VOS_VOID IP_NdClient_Init(VOS_VOID)
{
    VOS_UINT32  i;

    IP_MEM_SET(g_stNdClientEntity, IP_NULL, IP_MAX_NDCLIENT_ENTITY_NUM*sizeof(IP_NDCLIENT_ENTITY_STRU));
    for(i = 0; i < IP_MAX_NDCLIENT_ENTITY_NUM; i++)
    {
        g_stNdClientEntity[i].ucflag = 0;
        g_stNdClientEntity[i].enMsgType = IP_ICMPV6_TYPE_BUTT;
    }

    IP_MEM_SET(&g_stNdClientStatInfo, IP_NULL, sizeof(IP_NDCLIENT_STATISTIC_INFO_STRU));

    IP_NdClient_InitEntityNum();

    return;
}


VOS_UINT32  IP_NdClient_GetEntityIndex(VOS_UINT8 ucExEpsId)
{
    VOS_UINT32      ulCnt;

    for (ulCnt = 0; ulCnt < IP_MAX_NDCLIENT_ENTITY_NUM; ulCnt ++)
    {
        if((0 != g_stNdClientEntity[ulCnt].ucflag) && (ucExEpsId == g_stNdClientEntity[ulCnt].ucEpsbId))
        {
            return ulCnt;
        }
    }

    return IP_MAX_NDCLIENT_ENTITY_NUM;
}



IP_NDCLIENT_ENTITY_STRU* IP_NdClient_GetEntity
(
    VOS_UINT8 ucEpsbId
)
{
    VOS_UINT32 i = 0;

    /* 查询是否已存在相应Entity */
    do{
        if((0 != g_stNdClientEntity[i].ucflag) && (ucEpsbId == g_stNdClientEntity[i].ucEpsbId))
        {
            /*找到相应实体*/
            return &g_stNdClientEntity[i];
        }

    }while((++i) < IP_MAX_NDCLIENT_ENTITY_NUM);

    return IP_NULL_PTR;
}


IP_NDCLIENT_ENTITY_STRU* IP_NdClient_AllocEntity(VOS_UINT8  ucExRabId)
{
    VOS_UINT32 i = 0;

    /* 查询是否存在空闲Enity */
    do{
        if(0 == g_stNdClientEntity[i].ucflag)
        {
            /*找到空闲实体*/
            IP_MEM_SET(&g_stNdClientEntity[i], 0, sizeof(IP_NDCLIENT_ENTITY_STRU));

            g_stNdClientEntity[i].ucflag = 1;
            g_stNdClientEntity[i].ucEpsbId = ucExRabId;
            /*增加实体个数*/
            IP_NdClient_AddEntityNum(ND_GET_MODEMID_FROM_EXEPSID(ucExRabId));

            return &g_stNdClientEntity[i];
        }

    }while(++i < IP_MAX_NDCLIENT_ENTITY_NUM);

    return IP_NULL_PTR;

}


VOS_VOID IP_NdClient_RelEntity
(
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity
)
{
    IP_ASSERT(IP_NULL_PTR != pstNdClientEntity);
    IP_ASSERT(0 != pstNdClientEntity->ucflag);

    /*减小实体计数*/
    IP_NdClient_SubEntityNum(ND_GET_MODEMID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId));

    /* 释放重传资源 */
    IP_NdClient_ReleaseResendResourse(pstNdClientEntity);

    IP_MEM_SET(pstNdClientEntity, 0, sizeof(IP_NDCLIENT_ENTITY_STRU));
    pstNdClientEntity->ucflag = 0;
    pstNdClientEntity->enMsgType = IP_ICMPV6_TYPE_BUTT;

    return;
}




VOS_VOID IP_StartFilterIpv6(VOS_UINT16 usModemId)
{
    NDCLIENT_CDS_START_FILTER_REQ_STRU      *pstStartFilterReq = VOS_NULL_PTR;

    /*入参判断*/
    if (usModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,"IP_StartFilterIpv6: Input ModemId Error.",usModemId);
        return;
    }

    /*分配空间并检验分配是否成功*/
    pstStartFilterReq = (NDCLIENT_CDS_START_FILTER_REQ_STRU *)
                        PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_CDS_START_FILTER_REQ_STRU));

    if ( VOS_NULL_PTR == pstStartFilterReq )
    {
        /*打印异常信息*/
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_StartFilterIpv6: Alloc Msg fail!");
        return ;
    }

    /*填写消息头*/
    ND_IP_WRITE_MSG_HEAD(pstStartFilterReq,ID_NDCLIENT_CDS_START_FILTER_REQ,NDCLIENT_TASK_PID,UEPS_PID_CDS);
    pstStartFilterReq->enModemId = usModemId;

    /* 调用消息发送函数 */
    if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstStartFilterReq))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_StartFilterIpv6::send ID_NDCLIENT_CDS_START_FILTER_REQ error.\n");
    }

    IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient trigger CDS to start filter RA from DL IP!");

    return;
}


VOS_VOID IP_StopFilterIpv6(VOS_UINT16   usModemId)
{
    NDCLIENT_CDS_STOP_FILTER_REQ_STRU      *pstStopFilterReq = VOS_NULL_PTR;

    if (usModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,"IP_StopFilterIpv6: Input ModemId Error.",usModemId);
        return;
    }

    /*分配空间并检验分配是否成功*/
    pstStopFilterReq = (NDCLIENT_CDS_STOP_FILTER_REQ_STRU *)
                        PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_CDS_STOP_FILTER_REQ_STRU));

    if ( VOS_NULL_PTR == pstStopFilterReq )
    {
        /*打印异常信息*/
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_StopFilterIpv6: Alloc Msg fail!");
        return ;
    }

    pstStopFilterReq->enModemId = usModemId;

    /*填写消息头*/
    ND_IP_WRITE_MSG_HEAD(pstStopFilterReq,ID_NDCLIENT_CDS_STOP_FILTER_REQ,NDCLIENT_TASK_PID,UEPS_PID_CDS);

    IP_NDCLIENT_IncSendPackageNum();
    IPND_INFO_LOG(NDCLIENT_TASK_PID,"NdClient trigger CDS to stop filter RA from DL IP!");

    /* 调用消息发送函数 */
    if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstStopFilterReq))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_StopFilterIpv6::send ID_NDCLIENT_CDS_START_FILTER_REQ error.\n");
    }

    return;
}


VOS_UINT32  IP_NdClient_GetTimerLen
(
    NDCLIENT_TIMER_ENUM_UINT32     enTimerType
)
{
    VOS_UINT32                  ulTimerLen   = IP_NULL;

    /*根据定时器不同类型，定时器时长不同*/
    switch( enTimerType )
    {
        case IP_NDCLIENT_TIMER_RS:
            ulTimerLen = g_ulNdRtrSolicitationInterval;
            break;
        default :
            break;
    }

    return ulTimerLen;
}



VOS_VOID IP_NdClient_StopTimer
(
    IP_TIMER_STRU    *pstTimerInfo
)
{
    if (VOS_NULL_PTR != pstTimerInfo->hTm)
    {
        if(VOS_OK != PS_STOP_REL_TIMER(&(pstTimerInfo->hTm)))
        {
            IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_StopTimer: Error to stop RealTimer");
        }
        pstTimerInfo->hTm = VOS_NULL_PTR;
    }
}


VOS_VOID IP_NdClient_ReleaseResendResourse
(
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity
)
{
    /* 停止定时器 */
    IP_NdClient_StopTimer(&pstNdClientEntity->stTimerInfo);
    pstNdClientEntity->stTimerInfo.ucLoopTimes = 0;

    pstNdClientEntity->enMsgType = IP_ICMPV6_TYPE_BUTT;

    /* 释放重发缓存消息内存 */
    if(VOS_NULL_PTR != pstNdClientEntity->stIpSndNwMsg.pucSendMsg)
    {
        PS_MEM_FREE(NDCLIENT_TASK_PID,pstNdClientEntity->stIpSndNwMsg.pucSendMsg);
        pstNdClientEntity->stIpSndNwMsg.pucSendMsg = VOS_NULL_PTR;

        IP_NDCLIENT_Entity_IncMemFreeNum(pstNdClientEntity->ucEpsbId);
    }
}


IP_BOOL_ENUM_UINT8 IP_IsValidPrefix
(
    const IP_IPV6_PREFIX_STRU *pstPrefix
)
{
    VOS_UINT32                          ulTmp;

    if(pstPrefix->ulPreferredLifeTime > pstPrefix->ulValidLifeTime)
    {
        IPND_WARNING_LOG(NDCLIENT_TASK_PID, "IP_IsValidPrefix: preferred lifetime > valid lifetime!");
        return IP_FALSE;
    }

    IP_GetUint16Data(ulTmp, pstPrefix->aucPrefix);
    if(IP_IPV6_LINK_LOCAL_PREFIX == (ulTmp & IP_IPV6_LINK_LOCAL_MASK))
    {
        IPND_WARNING_LOG(NDCLIENT_TASK_PID, "IP_IsValidPrefix: link-local prefix!");
        return IP_FALSE;
    }

    return IP_TRUE;
}


IP_BOOL_ENUM_UINT8 IP_NdClient_RaIsForMe
(
    VOS_UINT8      ucEpsbId,
    const VOS_UINT8      *pucDstIp
)
{
    VOS_UINT8    aucLinkLocalAddr[IP_IPV6_ADDR_LEN] = {0};
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity = VOS_NULL_PTR;

    pstNdClientEntity = IP_NdClient_GetEntity(ucEpsbId);

    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_RaIsForMe: Not find valid Entity!");
        return IP_FALSE;
    }

    /* all-node multicast address */
    if(0 == IP_MEM_CMP(pucDstIp, g_aucNdAllNodesMulticaseAddr, IP_IPV6_ADDR_LEN))
    {
        IPND_INFO_LOG(NDCLIENT_TASK_PID, "IP_NdClient_RaIsForMe: to all-node multicast address");
        return IP_TRUE;
    }

    /* 构造link-local address */
    IP_BuildLinkLocalAddr(aucLinkLocalAddr, pstNdClientEntity->aucInterfaceId);
    /* 是否发给本机link-local address */
    if(0 == IP_MEM_CMP(pucDstIp, aucLinkLocalAddr, IP_IPV6_ADDR_LEN))
    {
        IPND_INFO_LOG(NDCLIENT_TASK_PID, "IP_NdClient_RaIsForMe: to my link-local address");
        return IP_TRUE;
    }

    IPND_WARNING_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_RaIsForMe: not match my address:", *(VOS_UINT16*)(VOS_VOID*)pucDstIp);
    return IP_FALSE;
}



IP_ERR_ENUM_UINT32 IP_NdClient_DecompositeRA
(
    IP_ND_MSG_RA_STRU      *pstRa,
    const VOS_UINT8        *pucRaMsg,
    VOS_UINT32              ulTypeOffset
)
{
    VOS_UINT32              ulPayLoad;
    VOS_INT32               remainLen;
    VOS_UINT32              ulOptType;
    VOS_UINT32              ulOptLen;
    const VOS_UINT8         *pucIpMsg = pucRaMsg;

    /* 获取PAYLOAD */
    IP_GetUint16Data(ulPayLoad, pucIpMsg + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);

    /* 获取RA cur hop limit */
    pucIpMsg += ulTypeOffset + IP_ICMPV6_HOP_LIMIT_OFFSET;
    pstRa->ulBitCurHopLimit = *pucIpMsg;

    pucIpMsg++;
    pstRa->ulBitM = (*pucIpMsg >> 7) & 0x1;
    pstRa->ulBitO = (*pucIpMsg >> 6) & 0x1;

    pucIpMsg++;
    /* 获取RouterLifeTime */
    IP_GetUint16Data(pstRa->ulBitRouterLifeTime, pucIpMsg);

    pucIpMsg += 2;
    /* 获取ReachableTimer */
    IP_GetUint32Data(pstRa->ulReachableTimer, pucIpMsg);

    pucIpMsg += 4;
    /* 获取ReTransTime */
    IP_GetUint32Data(pstRa->ulReTransTime, pucIpMsg);

    pucIpMsg += 4;
    /* 判断RA消息长度是否合法 >=16 */
    remainLen = (VOS_INT32)ulPayLoad + IP_IPV6_HEAD_LEN - (pucIpMsg - pucRaMsg);
    if(remainLen < 0)
    {
        IPND_ERROR_LOG2(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid IPV6 PayLoad::!", ulPayLoad, ulTypeOffset);
        return IP_FAIL;
    }

    while(remainLen > 0)
    {
        ulOptType = *pucIpMsg;
        ulOptLen = *(pucIpMsg + 1);

        if(0 == ulOptLen)
        {
            IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid ND options length 0!");
            return IP_FAIL;
        }

        switch(ulOptType)
        {
        case IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR:
            {
                if(1 == ulOptLen)
                {
                    if(0 == pstRa->ulBitOpSrcLinkLayerAddr)
                    {
                        IP_MEM_CPY(pstRa->aucSrcLinkLayerAddr, pucIpMsg+2, IP_MAC_ADDR_LEN);
                        pstRa->ulBitOpSrcLinkLayerAddr = 1;
                    }
                    else
                    {
                        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Redundant Source Link-Layer Addr!");
                    }
                }
                else
                {
                    IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid Source Link-Layer Addr Length:!", ulOptLen);
                }
            }
            break;
        case IP_ICMPV6_OPT_MTU:
            {
                if(1 == ulOptLen)
                {
                    if(0 == pstRa->ulBitOpMtu)
                    {
                        IP_GetUint32Data(pstRa->ulMtu, pucIpMsg+4);
                        pstRa->ulBitOpMtu = 1;
                    }
                    else
                    {
                        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Redundant MTU!");
                    }
                }
                else
                {
                    IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid MTU Length:!", ulOptLen);
                }
            }
            break;
        case IP_ICMPV6_OPT_PREFIX_INFO:
            {
                if(4 == ulOptLen)
                {
                    VOS_UINT32 ulPrefixLen = *(pucIpMsg + 2);

                    if((128 < ulPrefixLen) || (0 == ulPrefixLen))
                    {
                        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid Prefix Length!", ulPrefixLen);
                        break;  /* 非法前缀，丢弃 */
                    }

                    if(IP_MAX_PREFIX_NUM_IN_RA > pstRa->ulPrefixNum)
                    {
                        IP_IPV6_PREFIX_STRU *pstPrefix = &pstRa->astPrefixList[pstRa->ulPrefixNum];
                        pstPrefix->ulBitL = (*(pucIpMsg + 3) >> 7) & 0x1;
                        pstPrefix->ulBitA = (*(pucIpMsg + 3) >> 6) & 0x1;
                        pstPrefix->ulBitPrefixLen = ulPrefixLen;
                        IP_GetUint32Data(pstPrefix->ulValidLifeTime, pucIpMsg+4);
                        IP_GetUint32Data(pstPrefix->ulPreferredLifeTime, pucIpMsg+8);
                        IP_MEM_CPY(pstPrefix->aucPrefix, pucIpMsg+16, IP_IPV6_ADDR_LEN);

                        if(IP_TRUE == IP_IsValidPrefix(pstPrefix))
                        {
                            pstRa->ulPrefixNum++;
                        }
                    }
                    else
                    {
                        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: too much Prefix Info, ignored!");
                    }
                }
                else
                {
                    IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Invalid Prefix Option Length:!", ulOptLen);
                }
            }
            break;
        default:
            break;
        }

        remainLen -= (VOS_INT32)ulOptLen * 8;
        pucIpMsg += ulOptLen * 8;
    }

    if(0 != remainLen)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_DecompositeRA: Payload not match Package:!", remainLen);
        return IP_FAIL;
    }

    return IP_SUCC;
}


IP_ERR_ENUM_UINT32 IP_NdClient_UpdateWithRA
(
    VOS_UINT8                ucEpsbId,
    const IP_ND_MSG_RA_STRU       *pstRa,
    const VOS_UINT8               *pucSrcIp
)
{
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity = VOS_NULL_PTR;

    pstNdClientEntity = IP_NdClient_GetEntity(ucEpsbId);

    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_UpdateWithRA: Not find valid Entity!");
        return IP_FAIL;
    }

    /*暂时以覆盖方式处理*/
    IP_MEM_CPY(pstNdClientEntity->aucRouterIpAddr, pucSrcIp, IP_IPV6_ADDR_LEN);
    pstNdClientEntity->usRouterLifeTime = pstRa->ulBitRouterLifeTime;
    pstNdClientEntity->ulReachableTimer = pstRa->ulReachableTimer;
    pstNdClientEntity->ulReTransTime = pstRa->ulReTransTime;
    pstNdClientEntity->ulMtu         = pstRa->ulMtu;

    if(IP_ICMPV6_TYPE_RS == pstNdClientEntity->enMsgType)
    {
        /*收到响应，取消重发处理*/
        IPND_INFO_LOG(NDCLIENT_TASK_PID, "IP_NdClient_UpdateWithRA: RS received, Release resend resource!");
        IP_NdClient_ReleaseResendResourse(pstNdClientEntity);
    }

    return IP_SUCC;

}


IP_ERR_ENUM_UINT32 IP_NdClient_NotifyApsWithRA
(
    VOS_UINT8              ucEpsId,
    const IP_ND_MSG_RA_STRU      *pstRa
)
{
    NDCLIENT_APS_PARA_INFO_NOTIFY_STRU   *pstIpConfigParaInd = IP_NULL_PTR;
    NDCLIENT_APS_IPV6_INFO_STRU          *pstIpv6ConfigPara  = IP_NULL_PTR;
    VOS_UINT16                            usModemId;

    /*分配空间并检验分配是否成功*/
    pstIpConfigParaInd = (NDCLIENT_APS_PARA_INFO_NOTIFY_STRU *)
                          PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_APS_PARA_INFO_NOTIFY_STRU));

    if ( IP_NULL_PTR == pstIpConfigParaInd )
    {
        /*打印异常信息*/
        IPND_ERROR_LOG(NDCLIENT_TASK_PID,"IP_NdClient_NotifyApsWithRA: Alloc Msg fail!");
        return IP_FAIL;
    }

    usModemId = ND_GET_MODEMID_FROM_EXEPSID(ucEpsId);
    pstIpConfigParaInd->ucRabId = ND_GET_EPSID_FROM_EXEPSID(ucEpsId);

    pstIpv6ConfigPara = &pstIpConfigParaInd->stIpv6Info;

    /* 初始化消息体 */
    IP_MEM_SET(pstIpv6ConfigPara,
               IP_NULL,
               sizeof(NDCLIENT_APS_IPV6_INFO_STRU));

    pstIpv6ConfigPara->ulPrefixNum = pstRa->ulPrefixNum;
    IP_MEM_CPY(pstIpv6ConfigPara->astPrefixList,
                pstRa->astPrefixList,
                pstRa->ulPrefixNum * sizeof(IP_IPV6_PREFIX_STRU));
    pstIpv6ConfigPara->ulBitOpMtu = pstRa->ulBitOpMtu;
    pstIpv6ConfigPara->ulBitCurHopLimit = pstRa->ulBitCurHopLimit;
    pstIpv6ConfigPara->ulBitM = pstRa->ulBitM;
    pstIpv6ConfigPara->ulBitO = pstRa->ulBitO;
    pstIpv6ConfigPara->ulMtu = pstRa->ulMtu;

    /*填写消息头*/
    if (MODEM_ID_0 == usModemId)
    {
        ND_IP_WRITE_MSG_HEAD(pstIpConfigParaInd,ID_NDCLIENT_APS_PARA_INFO_NOTIFY,NDCLIENT_TASK_PID,I0_WUEPS_PID_TAF);
    }
    else
    {
        ND_IP_WRITE_MSG_HEAD(pstIpConfigParaInd,ID_NDCLIENT_APS_PARA_INFO_NOTIFY,NDCLIENT_TASK_PID,I1_WUEPS_PID_TAF);
    }


    /* 调用消息发送函数 */
    if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstIpConfigParaInd))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_NotifyApsWithRA::send ID_NDCLIENT_APS_PARA_INFO_NOTIFY error.");
        return IP_FAIL;
    }

    return IP_SUCC;

}


VOS_VOID IP_NdClient_ProcessRA
(
    VOS_UINT8              ucExRabId,
    const VOS_UINT8       *pucIpMsg,
    VOS_UINT32             ulTypeOffset
)
{
    IP_ND_MSG_RA_STRU         *pstRa = &g_stNdMsgBuffer.stRa;
    VOS_UINT8                  aucSrcIp[IP_IPV6_ADDR_LEN] = {0};
    VOS_UINT8                  aucDstIp[IP_IPV6_ADDR_LEN] = {0};

    IP_NDCLIENT_Entity_IncRevRaPackageNum(ucExRabId);

    IP_MEM_SET((VOS_VOID*)pstRa, 0, sizeof(IP_ND_MSG_RA_STRU));

    IP_MEM_CPY(aucSrcIp, pucIpMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_MEM_CPY(aucDstIp, pucIpMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    if(IP_TRUE != IP_NdClient_RaIsForMe(ucExRabId, aucDstIp))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: RA not for me!");
        IP_NDCLIENT_Entity_IncRevNotForMePackageNum(ucExRabId);
        return ;
    }

    /* 判断是否合法的RA消息 */
    if (IP_TRUE != IP_IsValidRAPacket(pucIpMsg, ulTypeOffset))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: Invalid IPV6 RA Msg:!");
        IP_NDCLIENT_Entity_IncRevInvalidPackageNum(ucExRabId);
        return;
    }

    /* RA解包 */
    if(IP_SUCC != IP_NdClient_DecompositeRA(pstRa, pucIpMsg, ulTypeOffset))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: Fail to decomposite RA!");
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        return;
    }

    /* 没有有效prefix，discard the RA */
    if(0 == pstRa->ulPrefixNum)
    {
        IPND_WARNING_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: Fail to get valid RA!");
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        return;
    }

    /* 保存RA信息 */
    if(IP_SUCC != IP_NdClient_UpdateWithRA(ucExRabId, pstRa, aucSrcIp))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: Fail to Update local state with RA!");
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        return ;
    }

    IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient get prefix to build global IPv6 address!");

    /* Send ID_ESM_IP_CONFIG_PARA_IND to ESM */
    if(IP_SUCC != IP_NdClient_NotifyApsWithRA(ucExRabId, pstRa))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessRA: Fail to Notify ESM with RA!");
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        return ;
    }

    IP_NDCLIENT_Entity_IncSendPackageNum(ucExRabId);

    IPND_INFO_LOG(NDCLIENT_TASK_PID,"NdClient Notify ESM with RA!");
}


TTF_MEM_ST *IP_NdClient_BuildEchoReply(const VOS_UINT8 *paucLinkLocalAddr, const VOS_UINT8 *pucIpMsg, VOS_UINT32 ulDataLen)
{
    TTF_MEM_ST                         *pstSdu = IP_NULL_PTR;
    VOS_UINT8                          *pucData;                      /*浮动指针*/
    VOS_UINT8                           aucDstIPAddr[IP_IPV6_ADDR_LEN] = {IP_NULL};

    if (ulDataLen <= IP_IPV6_HEAD_LEN)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_BuildEchoReply: ulDataLen is too short.", ulDataLen);
        return IP_NULL_PTR;
    }

    /* 为IP包申请内存 */
    pstSdu = TTF_MemBlkAlloc(NDCLIENT_TASK_PID,
                             IP_NDCLIENT_MEM_POOL_ID,
                             (VOS_UINT16)(ulDataLen + IP_PACKAGE_HEADER_RESERVE));
    if(VOS_NULL_PTR == pstSdu)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_BuildEchoReply: Fail to malloc memory for RS!");
        return IP_NULL_PTR;
    }

    IP_MEM_SET(pstSdu->pData, IP_NULL, (ulDataLen + IP_PACKAGE_HEADER_RESERVE));

    pstSdu->pData += IP_PACKAGE_HEADER_RESERVE;
    pstSdu->usUsed = (VOS_UINT16)ulDataLen;

    pucData = pstSdu->pData;

    /*指向ICMP首部，填写ICMP报文*/
    pucData  += IP_IPV6_HEAD_LEN;
    pucIpMsg += IP_IPV6_HEAD_LEN;
    IP_MEM_CPY(pucData, pucIpMsg,(ulDataLen-IP_IPV6_HEAD_LEN));

    /*ICMP type域*/
    *pucData = IP_ICMPV6_TYPE_ECHOREPLY;
    pucData++;

    /*ICMP code域*/
    *pucData = IP_IPV6_ND_VALID_CODE;
    pucData++;

    /*ICMP 校验和域*/
    *(VOS_UINT16 *)(VOS_VOID*)pucData = 0;

    /*再指向IPV6首部*/
    pucData = pstSdu->pData;
    pucIpMsg -= IP_IPV6_HEAD_LEN;

    /*填写目的IPV6地址*/
    IP_MEM_CPY(aucDstIPAddr, pucIpMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);

    /*填写IPV6头部*/
    IP_ND_FormIPv6HeaderMsg(paucLinkLocalAddr, aucDstIPAddr, (ulDataLen-IP_IPV6_HEAD_LEN), pucData, IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6报头校验和 */
    if (IP_SUCC != IP_BuildIcmpv6Checksum(pucData, IP_IPV6_HEAD_LEN))
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstSdu);
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_BuildEchoReply: Build ICMPv6 Checksum failed.");
        return IP_NULL_PTR;
    }

    return pstSdu;
}



VOS_VOID IP_NdClient_ProcessEchoReq(VOS_UINT8 ucRabId,VOS_UINT8 *pucSrcData, VOS_UINT32 ulDataLen)
{
    VOS_UINT8                          *pucIpMsg = pucSrcData;
    VOS_UINT8                           aucPktDstIpAddr[IP_IPV6_ADDR_LEN] = {IP_NULL};
    VOS_UINT8                           aucLinkLocalAddr[IP_IPV6_ADDR_LEN] = {0};
    IP_NDCLIENT_ENTITY_STRU            *pstNdClientEntity = IP_NULL_PTR;
    TTF_MEM_ST                         *pstIpPacket;
    NDCLIENT_CDS_DATA_REQ_STRU         *pstDataReq;

    IP_NDCLIENT_Entity_IncRevEchoReqPackageNum(ucRabId);

    pstNdClientEntity = IP_NdClient_GetEntity(ucRabId);

    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_ProcessEchoReq: Not find valid Entity!", ucRabId);
        return;
    }

    /*ECHO REQUEST报文中的目的IPV6地址*/
    IP_MEM_CPY(aucPktDstIpAddr, pucIpMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);

    /* 构造link-local address */
    IP_BuildLinkLocalAddr(aucLinkLocalAddr, pstNdClientEntity->aucInterfaceId);
    /* 是否发给本机link-local address */
    if(0 != IP_MEM_CMP(aucPktDstIpAddr, aucLinkLocalAddr, IP_IPV6_ADDR_LEN))
    {
        IP_NDCLIENT_Entity_IncRevNotForMePackageNum(pstNdClientEntity->ucEpsbId);
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessEchoReq: ECHO REQUEST is not for me!");
        return;
    }

    pstIpPacket = IP_NdClient_BuildEchoReply(aucLinkLocalAddr, pucIpMsg, ulDataLen);
    if (IP_NULL_PTR == pstIpPacket)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessEchoReq: IP_NdClient_BuildEchoReply fail!");
        return;
    }

    /*分配空间并检验分配是否成功*/
    pstDataReq = (NDCLIENT_CDS_DATA_REQ_STRU *)
                        PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_CDS_DATA_REQ_STRU));

    if ( VOS_NULL_PTR == pstDataReq )
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstIpPacket);
        /*打印异常信息*/
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessEchoReq: Alloc Msg fail!");
        return;
    }

    /*填写ModemId和RabId*/
    pstDataReq->enModemId   = ND_GET_MODEMID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
    pstDataReq->ucRabId     = ND_GET_EPSID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
    pstDataReq->pstIpPacket = pstIpPacket;

    /*填写消息头*/
    ND_IP_WRITE_MSG_HEAD(pstDataReq,ID_NDCLIENT_CDS_DATA_REQ,NDCLIENT_TASK_PID,UEPS_PID_CDS);

    /* 调用消息发送函数 */
    if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstDataReq))
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstIpPacket);
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcessEchoReq:send ECHO REPLY fail!");
    }
    else
    {
        IP_NDCLIENT_Entity_IncSendEchoReplyPackageNum(pstNdClientEntity->ucEpsbId);
        IP_NDCLIENT_Entity_IncSendPackageNum(pstNdClientEntity->ucEpsbId);
    }

    return;
}


TTF_MEM_ST *IP_NdClient_BuildTooBigICMPPkt(const VOS_UINT8 *paucLinkLocalAddr, const VOS_UINT8 *pucIpMsg, VOS_UINT32 ulDataLen)
{

    TTF_MEM_ST                         *pstSdu = IP_NULL_PTR;
    VOS_UINT8                          *pucData;                  /*浮动指针*/
    VOS_UINT8                           aucDstIPAddr[IP_IPV6_ADDR_LEN] = {IP_NULL};

    if  (ulDataLen <= (IP_IPV6_HEAD_LEN + IP_ICMPV6_HEAD_LEN))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_BuildTooBigICMPPkt: ulDataLen is too short.", ulDataLen);
        return IP_NULL_PTR;
    }

    /* 为IP包申请内存 */
    pstSdu = TTF_MemBlkAlloc(NDCLIENT_TASK_PID,
                             IP_NDCLIENT_MEM_POOL_ID,
                             (VOS_UINT16)(ulDataLen + IP_PACKAGE_HEADER_RESERVE));
    if(VOS_NULL_PTR == pstSdu)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_BuildTooBigICMPPkt: Fail to malloc memory!");
        return IP_NULL_PTR;
    }

    IP_MEM_SET(pstSdu->pData, IP_NULL, (ulDataLen + IP_PACKAGE_HEADER_RESERVE));

    pstSdu->pData += IP_PACKAGE_HEADER_RESERVE;
    pstSdu->usUsed = (VOS_UINT16)ulDataLen;

    pucData = pstSdu->pData; /*IP首部*/

    /*指向ICMP首部，填写ICMP报文*/
    pucData  += IP_IPV6_HEAD_LEN;

    /*ICMP type域*/
    *pucData = IP_ICMPV6_PACKET_TOO_BIG;
    pucData++;

    /*ICMP code域*/
    *pucData = IP_IPV6_ND_VALID_CODE;
    pucData++;

    /*ICMP 校验和域*/
    *(VOS_UINT16 *)(VOS_VOID*)pucData = 0;
    pucData += 2;

    /*MTU 域*/
    *(VOS_UINT32 *)(VOS_VOID*)pucData = VOS_HTONL(ulDataLen);
    pucData += 4;

    /*拷贝ICMP数据部分*/
    IP_MEM_CPY(pucData, pucIpMsg,((ulDataLen-IP_IPV6_HEAD_LEN)-IP_ICMPV6_HEAD_LEN));

    /*再指向IPV6首部*/
    pucData = pstSdu->pData;

    /*填写目的IPV6地址*/
    IP_MEM_CPY(aucDstIPAddr, pucIpMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);

    /*填写IPV6头部*/
    IP_ND_FormIPv6HeaderMsg(paucLinkLocalAddr, aucDstIPAddr, (ulDataLen-IP_IPV6_HEAD_LEN), pucData, IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6报头校验和 */
    if (IP_SUCC != IP_BuildIcmpv6Checksum(pucData, IP_IPV6_HEAD_LEN))
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstSdu);
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_BuildTooBigICMPPkt: Build ICMPv6 Checksum failed.");
        return IP_NULL_PTR;
    }

    return pstSdu;
}



VOS_VOID IP_NdClient_ProcTooBigPkt(VOS_UINT8 ucRabId, VOS_UINT8 *pucSrcData, VOS_UINT32 ulDataLen)
{
    VOS_UINT8                          *pucIpMsg = pucSrcData;
    VOS_UINT8                           aucLinkLocalAddr[IP_IPV6_ADDR_LEN] = {0};
    IP_NDCLIENT_ENTITY_STRU            *pstNdClientEntity = IP_NULL_PTR;
    TTF_MEM_ST                         *pstIpPacket;
    NDCLIENT_CDS_DATA_REQ_STRU         *pstDataReq;

    IP_NDCLIENT_Entity_IncRevTooBigPackageNum(ucRabId);

    pstNdClientEntity = IP_NdClient_GetEntity(ucRabId);

    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_ProcTooBigPkt: Not find valid Entity!", ucRabId);
        return;
    }

    /* 构造link-local address */
    IP_BuildLinkLocalAddr(aucLinkLocalAddr, pstNdClientEntity->aucInterfaceId);

    if (pstNdClientEntity->ulMtu > 0)
    {
        ulDataLen = PS_MIN(ulDataLen, pstNdClientEntity->ulMtu);                /*取NV中的MTU和RA指示的MTU两者中的最小值*/
    }

    pstIpPacket = IP_NdClient_BuildTooBigICMPPkt(aucLinkLocalAddr, pucIpMsg, ulDataLen);
    if (IP_NULL_PTR == pstIpPacket)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcTooBigPkt: IP_NdClient_BuildTooBigICMPPkt fail!");
        return;
    }

    /*分配空间并检验分配是否成功*/
    pstDataReq = (NDCLIENT_CDS_DATA_REQ_STRU *)
                        PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_CDS_DATA_REQ_STRU));

    if ( VOS_NULL_PTR == pstDataReq )
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstIpPacket);
        /*打印异常信息*/
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcTooBigPkt: Alloc Msg fail!");
        return;
    }

    /*填写ModemId和RabId*/
    pstDataReq->enModemId   = ND_GET_MODEMID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
    pstDataReq->ucRabId     = ND_GET_EPSID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
    pstDataReq->pstIpPacket = pstIpPacket;

    /*填写消息头*/
    ND_IP_WRITE_MSG_HEAD(pstDataReq,ID_NDCLIENT_CDS_DATA_REQ,NDCLIENT_TASK_PID,UEPS_PID_CDS);


    /* 调用消息发送函数 */
    if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstDataReq))
    {
        TTF_MemFree(NDCLIENT_TASK_PID, pstIpPacket);
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_ProcTooBigPkt::send TooBigPktReply error.");
    }
    else
    {
        IP_NDCLIENT_Entity_IncSendICMPV6TooBigPackageNum(pstNdClientEntity->ucEpsbId);
        IP_NDCLIENT_Entity_IncSendPackageNum(pstNdClientEntity->ucEpsbId);
    }

    return;
}


IP_ERR_ENUM_UINT32 IP_NdClient_SendIpToNw
(
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity
)
{
    TTF_MEM_ST                   *pstSdu;
    IP_TIMER_STRU                *pstTimerInfo;
    VOS_UINT32                    ulTimerLen = 0;
    NDCLIENT_CDS_DATA_REQ_STRU   *pstDataReq = VOS_NULL_PTR;

    if(IP_ICMPV6_TYPE_BUTT == pstNdClientEntity->enMsgType)
    {
        /* Timer停不掉，或者停的时候Timer消息已发出 */
        IPND_WARNING_LOG(NDCLIENT_TASK_PID, "IP_NdClient_SendIpToNw: Msg released!");
        return IP_FAIL;
    }

    pstTimerInfo = &pstNdClientEntity->stTimerInfo;

    /* 无论发送成功与否，当做完成一次重发 */
    pstTimerInfo->ucLoopTimes++;

    /* 发送缓存的IP包 */
    pstSdu = TTF_MemBlkAlloc(NDCLIENT_TASK_PID,
                             IP_NDCLIENT_MEM_POOL_ID,
                             (VOS_UINT16)(pstNdClientEntity->stIpSndNwMsg.ulSendMsgSize + IP_PACKAGE_HEADER_RESERVE));
    if(VOS_NULL_PTR == pstSdu)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_SendIpToNw: Fail to malloc memory for RS!");
    }
    else
    {
        IP_MEM_SET(pstSdu->pData, IP_NULL, (pstNdClientEntity->stIpSndNwMsg.ulSendMsgSize + IP_PACKAGE_HEADER_RESERVE));
        pstSdu->pData += IP_PACKAGE_HEADER_RESERVE;
        pstSdu->usUsed = (VOS_UINT16)pstNdClientEntity->stIpSndNwMsg.ulSendMsgSize;

        IP_MEM_CPY(pstSdu->pData, pstNdClientEntity->stIpSndNwMsg.pucSendMsg, pstNdClientEntity->stIpSndNwMsg.ulSendMsgSize);

        /*分配空间并检验分配是否成功*/
        pstDataReq = (NDCLIENT_CDS_DATA_REQ_STRU *)
                            PS_ALLOC_MSG_WITH_HEADER_LEN(NDCLIENT_TASK_PID, sizeof(NDCLIENT_CDS_DATA_REQ_STRU));

        if ( VOS_NULL_PTR == pstDataReq )
        {
            TTF_MemFree(NDCLIENT_TASK_PID, pstSdu);
            /*打印异常信息*/
            IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_SendIpToNw: Alloc Msg fail!");
            return IP_FAIL;
        }

        pstDataReq->enModemId   = ND_GET_MODEMID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
        pstDataReq->ucRabId     = ND_GET_EPSID_FROM_EXEPSID(pstNdClientEntity->ucEpsbId);
        pstDataReq->pstIpPacket = pstSdu;

        /*填写消息头*/
        ND_IP_WRITE_MSG_HEAD(pstDataReq,ID_NDCLIENT_CDS_DATA_REQ,NDCLIENT_TASK_PID,UEPS_PID_CDS);

        IP_NDCLIENT_IncSendPackageNum();

        /* 调用消息发送函数 */
        if (IP_SUCC != PS_SEND_MSG(NDCLIENT_TASK_PID, pstDataReq))
        {
            TTF_MemFree(NDCLIENT_TASK_PID, pstSdu);
            IPND_ERROR_LOG(NDCLIENT_TASK_PID,  "IP_NdClient_SendIpToNw::send RS Pkt fail!");
        }
        else
        {
            IP_NDCLIENT_Entity_IncSendRsPackageNum(pstNdClientEntity->ucEpsbId);
            IP_NDCLIENT_Entity_IncSendPackageNum(pstNdClientEntity->ucEpsbId);
        }

    }

    /* 清除Timer句柄
    pstTimerInfo->hTimer = IP_NULL_PTR;*/

    /* 重传处理 */
    ulTimerLen = IP_NdClient_GetTimerLen(pstTimerInfo->ulName);
    if (ulTimerLen == IP_NULL)
    {
        /*打印异常信息*/
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_SendIpToNw: ERROR to get TimerLen: ", pstTimerInfo->ulName);
        IP_NdClient_ReleaseResendResourse(pstNdClientEntity);
        return IP_FAIL;
    }

    if (VOS_OK != PS_START_REL_TIMER(&pstTimerInfo->hTm,NDCLIENT_TASK_PID,\
                                ulTimerLen,pstTimerInfo->ulName,\
                                pstNdClientEntity->ucEpsbId,VOS_RELTIMER_NOLOOP))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_SendIpToNw: Fail to Start Timer, halt to send msg! ");
        pstTimerInfo->hTm= IP_NULL_PTR;
        IP_NdClient_ReleaseResendResourse(pstNdClientEntity);
        return IP_FAIL;
    }

    IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient start RS Timer!");
    IP_NDCLIENT_Entity_IncTimerNum(pstNdClientEntity->ucEpsbId);
    return IP_SUCC;
}


VOS_VOID IP_BuildRsHeader
(
    VOS_UINT8    *pucIcmpv6
)
{
    VOS_UINT16  usInitChecksum = IP_ICMPV6_INIT_CHECKSUM;

    /* 设置ICMPV6消息中的TYPE字段 */
    *pucIcmpv6 = IP_ICMPV6_TYPE_RS;

    /* 设置ICMPV6消息中的CODE字段 */
    *(pucIcmpv6 + IP_ICMPV6_CODE_OFFSET) = IP_IPV6_ND_VALID_CODE;

    /* 设置ICMPV6消息中的checksum字段 */
    IP_SetUint16Data(pucIcmpv6 + IP_ICMPV6_CHECKSUM_OFFSET, usInitChecksum);
}


VOS_VOID IP_NDCLIENT_ProcNdDataInd
(
    VOS_UINT8       ucExRabId,
    VOS_UINT8      *pucIpPkt,
    VOS_UINT32      ulDataLen
)

{
    VOS_UINT32                          ulTypeOffset    = IP_NULL;
    IP_ICMPV6_TYPE_ENUM_UINT32          ulType          = IP_NULL;


    if (IP_SUCC != ND_CheckEpsIdValid(ucExRabId))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID,
                        "IP_NDCLIENT_ProcAdsDataInd: Invalid Para.ModemId,RabId,ExRabId:",
                        ucExRabId);
        return;
    }

    IP_NDCLIENT_Entity_IncRevPackageNum(ucExRabId);

    /* 判断是否合法的ND消息 */
    if (IP_TRUE != IP_IsValidNdMsg(pucIpPkt, ulDataLen, &ulTypeOffset))
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcAdsDataInd: Invalid IPV6 ND Msg:!");
        IP_NDCLIENT_Entity_IncRevInvalidPackageNum(ucExRabId);
        return ;
    }

    /* 取ICMPV6消息中的TYPE字段 */
    ulType = *(pucIpPkt + ulTypeOffset);

    switch(ulType)
    {
    case IP_ICMPV6_TYPE_RA:
        IP_NdClient_ProcessRA(ucExRabId,pucIpPkt,ulTypeOffset);
        break;

    case IP_ICMPV6_TYPE_ECHOREQUEST:
        IP_NdClient_ProcessEchoReq(ucExRabId,pucIpPkt,ulDataLen);
        break;

    default:
        IPND_WARNING_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcAdsDataInd: Ignored IPV6 ND Msg:!", ulType);
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        break;
    }

    return ;
}


VOS_VOID IP_NdClient_AddrRequest
(
    IP_NDCLIENT_ENTITY_STRU *pstNdClientEntity
)
{
    VOS_UINT8               *pucRs = IP_NULL_PTR;
    VOS_UINT16              usCheckSum;
    VOS_UINT32              ulUpperLength = 8;
    IPV6_PSEDUOHEADER_STRU  stPseduoHeader;

    IP_ASSERT(IP_NULL_PTR != pstNdClientEntity);
    IP_ASSERT(IP_NULL_PTR == pstNdClientEntity->stIpSndNwMsg.pucSendMsg);

    PS_MEM_SET(&stPseduoHeader, 0, sizeof(IPV6_PSEDUOHEADER_STRU));

    pucRs = PS_MEM_ALLOC(NDCLIENT_TASK_PID,IP_ICMPV6_RS_MAX_LENGTH);
    if(IP_NULL_PTR == pucRs)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NdClient_AddrRequest: Fail to alloc memory for RS");
        return;
    }

    IP_NDCLIENT_Entity_IncMemAllocNum(pstNdClientEntity->ucEpsbId);

    IP_MEM_SET(pucRs, IP_NULL, IP_ICMPV6_RS_MAX_LENGTH);

    /* 保存消息缓存 */
    pstNdClientEntity->enMsgType = IP_ICMPV6_TYPE_RS;
    pstNdClientEntity->stIpSndNwMsg.pucSendMsg = pucRs;
    pstNdClientEntity->stIpSndNwMsg.ulSendMsgSize = IP_IPV6_HEAD_LEN + ulUpperLength;

    /* 构造ICMPV6 Header */
    IP_BuildRsHeader(pucRs + IP_IPV6_HEAD_LEN);

    /* 构造IPv6 header for RS */
    IP_BuildIPv6Header(pucRs, &stPseduoHeader,
                       pstNdClientEntity->aucInterfaceId, ulUpperLength);

    usCheckSum = IPv6_Checksum(&stPseduoHeader, pucRs + IP_IPV6_HEAD_LEN, ulUpperLength);
    /* 设置ICMPV6消息中的checksum字段 */
    /*IP_SetUint16Data(pucRs + IP_IPV6_HEAD_LEN + IP_ICMPV6_CHECKSUM_OFFSET, usCheckSum);*/
    *(VOS_UINT16 *)(VOS_VOID*)(pucRs + IP_IPV6_HEAD_LEN + IP_ICMPV6_CHECKSUM_OFFSET) = usCheckSum;

    pstNdClientEntity->stTimerInfo.ucLoopTimes = 0;
    pstNdClientEntity->stTimerInfo.ulName = IP_NDCLIENT_TIMER_RS;
    if(IP_SUCC == IP_NdClient_SendIpToNw(pstNdClientEntity))
    {
        IPND_INFO_LOG1(NDCLIENT_TASK_PID, "IP_NdClient_AddrRequest: succeed to trigger RS procedure:", pstNdClientEntity->stTimerInfo.ucLoopTimes);
    }
}


VOS_VOID IP_NDCLIENT_ProcConfigReq
(
    VOS_VOID      *pRcvMsg
)
{
    APS_NDCLIENT_PDP_ACT_IND           *pstConfigParaReq    = IP_NULL_PTR;
    IP_NDCLIENT_ENTITY_STRU            *pstNdClientEntity   = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucExRabId;

    IP_ASSERT(VOS_NULL_PTR != pRcvMsg);

    pstConfigParaReq = (APS_NDCLIENT_PDP_ACT_IND*)pRcvMsg;
    enModemId = VOS_GetModemIDFromPid(pstConfigParaReq->ulSenderPid);
    if (enModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcConfigReq: Invalid ModemId:", enModemId);
        return ;
    }

    ucExRabId = ND_FORM_EXEPSID(enModemId, pstConfigParaReq->ucRabId);

    if (IP_SUCC != ND_CheckEpsIdValid(ucExRabId))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcConfigReq: Invalid EpsbId:", ucExRabId);
        return;
    }

    IPND_INFO_LOG1(NDCLIENT_TASK_PID, "NdClient receive config req to trigger RS to require prefix:", ucExRabId);
    IP_NDCLIENT_Entity_IncRevPackageNum(ucExRabId);

    /* 查询是否已存在相应Entity */
    pstNdClientEntity = IP_NdClient_GetEntity(ucExRabId);
    if(IP_NULL_PTR != pstNdClientEntity)
    {
        /* 释放相应Entity */
        IP_NdClient_RelEntity(pstNdClientEntity);
    }

    pstNdClientEntity = IP_NdClient_AllocEntity(ucExRabId);
    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcConfigReq: too many Entity!");
        IP_NDCLIENT_IncDiscPackageNum();
        return;
    }

    IP_MEM_CPY(pstNdClientEntity->aucInterfaceId, pstConfigParaReq->aucIpv6InterfacId, ND_IPV6_IF_LEN);

    /* 触发RS，申请全球唯一的IPv6地址 */
    IP_NdClient_AddrRequest(pstNdClientEntity);
}


VOS_VOID IP_NDCLIENT_ProcStartFilterCnf
(
    VOS_VOID       *pRcvMsg
)
{
    CDS_NDCLIENT_START_FILTER_CNF_STRU    *pstStartFilterCnf = VOS_NULL_PTR;

    pstStartFilterCnf = (CDS_NDCLIENT_START_FILTER_CNF_STRU*)pRcvMsg;
    IP_ASSERT(VOS_NULL_PTR != pstStartFilterCnf);

    IP_NDCLIENT_IncRevPackageNum();

    if(NDCLIENT_CDS_RESULT_SUCC != pstStartFilterCnf->enRslt)
    {
        IPND_WARNING_LOG1(NDCLIENT_TASK_PID,
                          "IP_NDCLIENT_ProcStartFilterCnf: Fail to Start ICMPv6 Filter! ModemId=",
                          pstStartFilterCnf->enModemId);
    }
    else
    {
        IPND_INFO_LOG1(NDCLIENT_TASK_PID,
                       "IP_NDCLIENT_ProcStartFilterCnf: Start ICMPv6 Filter Successfully! ModemId=",
                       pstStartFilterCnf->enModemId);
    }

    return;
}


VOS_VOID IP_NDCLIENT_ProcStopFilterCnf
(
    VOS_VOID        *pRcvMsg
)
{
    CDS_NDCLIENT_STOP_FILTER_CNF_STRU    *pstStopFilterCnf = VOS_NULL_PTR;

    pstStopFilterCnf = (CDS_NDCLIENT_STOP_FILTER_CNF_STRU*)pRcvMsg;
    IP_ASSERT(VOS_NULL_PTR != pstStopFilterCnf);

    IP_NDCLIENT_IncRevPackageNum();

    if(NDCLIENT_CDS_RESULT_SUCC != pstStopFilterCnf->enRslt)
    {
        IPND_WARNING_LOG1(NDCLIENT_TASK_PID,
                          "IP_NDCLIENT_ProcStopFilterCnf: Fail to Stop ICMPv6 Filter! ModemId=",
                          pstStopFilterCnf->enModemId);
    }
    else
    {
        IPND_INFO_LOG1(NDCLIENT_TASK_PID,
                      "IP_NDCLIENT_ProcStopFilterCnf: Stop ICMPv6 Filter Successfully!",
                      pstStopFilterCnf->enModemId);
    }
}


VOS_VOID IP_NDCLIENT_ProcCdsNdDataInd
(
    VOS_VOID        *pRcvMsg
)
{
    CDS_NDCLIENT_ND_DATA_IND_STRU  *pstNdDataInd;
    VOS_UINT8                       ucExRabId;

    pstNdDataInd = (CDS_NDCLIENT_ND_DATA_IND_STRU*)((VOS_VOID*)pRcvMsg);
    IP_ASSERT(VOS_NULL_PTR != pstNdDataInd);

    IP_NDCLIENT_IncRevPackageNum();

    pstNdDataInd = (CDS_NDCLIENT_ND_DATA_IND_STRU*)pRcvMsg;

    ucExRabId = ND_FORM_EXEPSID(pstNdDataInd->enModemId, pstNdDataInd->ucRabId);

    IP_NDCLIENT_ProcNdDataInd(ucExRabId,pstNdDataInd->aucData,pstNdDataInd->ulLen);

    return;
}


VOS_VOID IP_NDCLIENT_ProcAdsErrInd
(
    VOS_VOID      *pRcvMsg
)
{
    ADS_NDCLIENT_ERR_IND_STRU          *pstErrInd      = IP_NULL_PTR;
    ADS_NDCLIENT_ERR_TYPE_ENUM_UINT8    enErrType;
    VOS_UINT8                           ucExRabId;

    pstErrInd = (ADS_NDCLIENT_ERR_IND_STRU*)pRcvMsg;

    ucExRabId = ND_FORM_EXEPSID(pstErrInd->enModemId, pstErrInd->ucRabId);

    enErrType   = pstErrInd->enErrType;

    if (IP_SUCC != ND_CheckEpsIdValid(ucExRabId))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcAdsErrInd: Invalid EpsbId:", ucExRabId);
        return;
    }

    IP_NDCLIENT_Entity_IncRevPackageNum(ucExRabId);

    switch(enErrType)
    {
    case ADS_NDCLIENT_ERR_TYPE_MTU_OVERLONG:
        IP_NdClient_ProcTooBigPkt(ucExRabId, pstErrInd->aucData, g_ulNdClientMtu);
        break;
    default:
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcAdsErrInd: ERR PktType:", enErrType);
        break;
    }

    return;
}



VOS_VOID IP_NDCLIENT_ProcAdsDataInd
(
    VOS_VOID      *pRcvMsg
)
{

    VOS_UINT8                           ucExRabId;
    ADS_NDCLIENT_DATA_IND_STRU         *pstDataInd;

    if (VOS_NULL_PTR == pRcvMsg)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcAdsDataInd: Input NULL PTR");
        return ;
    }

    pstDataInd = (ADS_NDCLIENT_DATA_IND_STRU*)pRcvMsg;

    ucExRabId = ND_FORM_EXEPSID(pstDataInd->enModemId, pstDataInd->ucRabId);

    IP_NDCLIENT_ProcNdDataInd(ucExRabId,pstDataInd->aucData,pstDataInd->usLen);

    return ;
}


VOS_VOID IP_NDCLIENT_ProcEsmIpPdpReleaseInd
(
    VOS_VOID             *pRcvMsg
)
{
    APS_NDCLIENT_PDP_DEACT_IND          *pstPdpReleaseInd = IP_NULL_PTR;
    IP_NDCLIENT_ENTITY_STRU             *pstNdClientEntity = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucExRabId;

    IP_ASSERT(IP_NULL_PTR != pRcvMsg);

    pstPdpReleaseInd = (APS_NDCLIENT_PDP_DEACT_IND *)pRcvMsg;

    enModemId = VOS_GetModemIDFromPid(pstPdpReleaseInd->ulSenderPid);
    if (enModemId >= MODEM_ID_BUTT)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcEsmIpPdpReleaseInd: Invalid ModemId:", enModemId);
        return ;
    }

    ucExRabId = ND_FORM_EXEPSID(enModemId, pstPdpReleaseInd->ucRabId);

    if (IP_SUCC != ND_CheckEpsIdValid(ucExRabId))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcEsmIpPdpReleaseInd: Invalid EpsbId:", ucExRabId);
        return;
    }

    IP_NDCLIENT_Entity_IncRevPackageNum(ucExRabId);
    IPND_INFO_LOG2(NDCLIENT_TASK_PID, "NdClient receive PDN release.ModemId,RabId", enModemId,pstPdpReleaseInd->ucRabId);

    /* 查询是否已存在相应Entity */
    pstNdClientEntity = IP_NdClient_GetEntity(ucExRabId);
    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "IP_NDCLIENT_ProcEsmIpPdpReleaseInd: Fail to find the Entity:!", ucExRabId);
        IP_NDCLIENT_Entity_IncDiscPackageNum(ucExRabId);
        return;
    }

    /* 释放相应Entity */
    IP_NdClient_RelEntity(pstNdClientEntity);

    return;
}


VOS_VOID IP_NDCLIENT_ProcEsmIpRelInd
(
    VOS_VOID     *pRcvMsg
)
{
    VOS_UINT32              i = 0;

    (VOS_VOID)pRcvMsg;

    IP_NDCLIENT_IncRevPackageNum();
    IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient receive RELEASE ALL!");

    for(i = 0; i < IP_MAX_NDCLIENT_ENTITY_NUM; i++)
    {
        if(0 != g_stNdClientEntity[i].ucflag)
        {
            IP_NdClient_RelEntity(&g_stNdClientEntity[i]);
        }
    }

    /*实体计数全部*/
    IP_NdClient_InitEntityNum();

    return;
}


VOS_UINT32  APP_NdClient_Pid_InitFunc( enum VOS_INIT_PHASE_DEFINE ePhase)
{
    switch( ePhase )
    {
        case   VOS_IP_LOAD_CONFIG:

            /*ND CLIENT模块初始化*/
            IP_NdClient_Init();

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
    return 0;
}


VOS_VOID NdClient_ApsMsgProc( MsgBlock *pMsgBlock )
{
    VOS_UINT32      ulMsgId;

    ulMsgId = TTF_GET_MSG_NAME(pMsgBlock);

    switch (ulMsgId)
    {
        case ID_APS_NDCLIENT_PDP_ACT_IND :
            IP_NDCLIENT_ProcConfigReq(pMsgBlock);
            break;

        case ID_APS_NDCLIENT_PDP_DEACT_IND :
            IP_NDCLIENT_ProcEsmIpPdpReleaseInd(pMsgBlock);
            break;

        case ID_APS_NDCLIENT_REL_IND :
            IP_NDCLIENT_ProcEsmIpRelInd(pMsgBlock);
            break;

        default:
            IPND_WARNING_LOG(NDCLIENT_TASK_PID,"NdClient_ApsMsgProc:Error Msg!");
            break;
    }

    return;
}


VOS_VOID NdClient_CdsMsgProc( MsgBlock *pMsgBlock )
{
    VOS_UINT32      ulMsgId;

    ulMsgId = TTF_GET_MSG_NAME(pMsgBlock);

    switch (ulMsgId)
    {
        case ID_CDS_NDCLIENT_START_FILTER_CNF :
            IP_NDCLIENT_ProcStartFilterCnf(pMsgBlock);
            break;

        case ID_CDS_NDCLIENT_STOP_FILTER_CNF :
            IP_NDCLIENT_ProcStopFilterCnf(pMsgBlock);
            break;

        case ID_CDS_NDCLIENT_ND_DATA_IND:
            IP_NDCLIENT_ProcCdsNdDataInd(pMsgBlock);
            break;

        default:
            IPND_WARNING_LOG(NDCLIENT_TASK_PID,"NdClient_CdsMsgProc:Error Msg!");
            break;
    }

    return;
}


VOS_VOID NdClient_AdsMsgProc( MsgBlock *pMsgBlock )
{
    VOS_UINT32      ulMsgId;

    ulMsgId = TTF_GET_MSG_NAME(pMsgBlock);

    switch (ulMsgId)
    {
        case ID_ADS_NDCLIENT_DATA_IND :
            IP_NDCLIENT_ProcAdsDataInd(pMsgBlock);
            break;

        case ID_ADS_NDCLIENT_ERR_IND :
            /*移植V3R1项目中TTF_NDIS_Ipv6RouterFwdWWanInputPacket函数的功能*/
            IP_NDCLIENT_ProcAdsErrInd(pMsgBlock);
            break;

        default:
            IPND_WARNING_LOG(NDCLIENT_TASK_PID, "NdClient_CdsMsgProc:Error Msg!");
            break;
    }

    return;
}


VOS_VOID NdClient_ProcTmrMsg(const REL_TIMER_MSG *pRcvMsg)
{
    VOS_UINT32                          ulTmrId;
    IP_NDCLIENT_ENTITY_STRU            *pstNdClientEntity;
    VOS_UINT8                           ucExEpsbId;

    ulTmrId = IP_GetTimerName(pRcvMsg);
    ucExEpsbId = (VOS_UINT8)IP_GetTimerPara(pRcvMsg);

    if (IP_SUCC != ND_CheckEpsIdValid(ucExEpsbId))
    {
        IPND_ERROR_LOG1(NDCLIENT_TASK_PID, "NdClient_ProcTmrMsg: Invalid EpsbId:", ucExEpsbId);
        return;
    }

    IP_NDCLIENT_Entity_IncTimeoutNum(ucExEpsbId);

    pstNdClientEntity = IP_NdClient_GetEntity(ucExEpsbId);
    if(IP_NULL_PTR == pstNdClientEntity)
    {
        IPND_ERROR_LOG(NDCLIENT_TASK_PID, "NdClient_ProcTmrMsg: Fail to get specific Entity");
        return;
    }

    switch(ulTmrId)
    {
    case IP_NDCLIENT_TIMER_RS:
        IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient receive RS Timeout!");
        pstNdClientEntity->stTimerInfo.hTm = IP_NULL_PTR;
        if(pstNdClientEntity->stTimerInfo.ucLoopTimes < g_ulNdMaxRtrSolicitaion)
        {
            if(IP_SUCC == IP_NdClient_SendIpToNw(pstNdClientEntity))
            {
                IPND_INFO_LOG1(NDCLIENT_TASK_PID, "NdClient_ProcTmrMsg: succeed to trigger RS RESEND:", pstNdClientEntity->stTimerInfo.ucLoopTimes);
            }
        }
        else
        {
            IPND_WARNING_LOG(NDCLIENT_TASK_PID, "NdClient_ProcTmrMsg: Fail to get RA by RS");
            IP_NdClient_ReleaseResendResourse(pstNdClientEntity);
            IPND_INFO_LOG(NDCLIENT_TASK_PID, "NdClient has NONE prefix to build global IPv6 address!");
        }

        break;
    default:
        break;
    }

    return;
}


VOS_VOID APP_NdClient_PidMsgProc( MsgBlock* pMsgBlock )
{
    if (VOS_NULL_PTR == pMsgBlock)
    {
        vos_printf("Error:APP_Ndis_DLPidMsgProc Parameter pRcvMsg is NULL!");
        return ;
    }

    switch (pMsgBlock->ulSenderPid)
    {
        case DOPRA_PID_TIMER:
            /*lint -e826*/
            NdClient_ProcTmrMsg((REL_TIMER_MSG *)pMsgBlock);
            /*lint +e826*/
            break;
        case I0_WUEPS_PID_TAF:
        case I1_WUEPS_PID_TAF:
            NdClient_ApsMsgProc(pMsgBlock);
            break;
        case UEPS_PID_CDS:          /*ADS通过OSA消息发送DHCP和ND SERVER包给NDIS模块*/
            NdClient_CdsMsgProc(pMsgBlock);
            break;
        case ACPU_PID_ADS_DL:
            NdClient_AdsMsgProc(pMsgBlock);
            break;
        default:
            IPND_WARNING_LOG(NDCLIENT_TASK_PID, "Warning:APP_Ndis_ULPidMsgProc Recv not expected msg!");
            break;
    }

    return ;
}


VOS_UINT32 NDCLIENT_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32  ulResult = PS_FAIL;

    switch (enPhase)
    {
        case   VOS_IP_LOAD_CONFIG:

            ulResult = VOS_RegisterPIDInfo(NDCLIENT_TASK_PID,
                                           (Init_Fun_Type)APP_NdClient_Pid_InitFunc,
                                           (Msg_Fun_Type)APP_NdClient_PidMsgProc);
            if (VOS_OK != ulResult)
            {
                vos_printf("NDCLIENT_FidInit, NDCLIENT_TASK_PID Reg msg routine FAIL!\n");
                return VOS_ERR;
            }

            ulResult = VOS_RegisterMsgTaskPrio(NDCLIENT_TASK_FID, VOS_PRIORITY_M5);
            if( VOS_OK != ulResult )
            {
                vos_printf("NDCLIENT_FidInit, VOS_RegisterMsgTaskPrio FAIL!\n");
                return VOS_ERR;
            }

            break;
        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;
        default:
            break;
    }

    return PS_SUCC;
}


VOS_VOID IP_NDCLIENT_Stat(VOS_UINT32 ulEpsbId, VOS_UINT32 ulModemId)
{
    VOS_UINT32 ulIndex;
    VOS_UINT8  ucExEpsbId;

    if ((ulEpsbId < ND_MIN_EPSB_ID) || (ulEpsbId > ND_MAX_EPSB_ID))
    {
        vos_printf("输入的EPSBID值错误－EPSBID取值范围为5..15 \r\n");
        return;
    }

    if (ulModemId >= MODEM_ID_BUTT)
    {
        vos_printf("输入的MODEMID非法\r\n");
        return;
    }


    vos_printf("\n**************ND CLIENT 统计信息*****************");
    vos_printf("\n 收到数据包总数           : %d", g_stNdClientStatInfo.ulRevPackageNum);
    vos_printf("\n 收到RA总数               : %d", g_stNdClientStatInfo.ulRevRaPackageNum);
    vos_printf("\n 收到ECHO REQUEST总数     : %d", g_stNdClientStatInfo.ulRevEchoReqPackageNum);
    vos_printf("\n 收到超长包总数           : %d", g_stNdClientStatInfo.ulRevTooBigPackageNum);
    vos_printf("\n 发送数据包总数           : %d", g_stNdClientStatInfo.ulSendPackageNum);
    vos_printf("\n 发送RS总数               : %d", g_stNdClientStatInfo.ulSendRsPackageNum);
    vos_printf("\n 发送ECHO REPLY总数       : %d", g_stNdClientStatInfo.ulSendEchoReplyPackageNum);
    vos_printf("\n 发送超长包响应总数       : %d", g_stNdClientStatInfo.ulSendICMPV6TooBigPackageNum);
    vos_printf("\n 丢弃数据包总数           : %d", g_stNdClientStatInfo.ulDiscPackageNum);
    vos_printf("\n 收到非本机数据包总数     : %d", g_stNdClientStatInfo.ulRevNotForMePackageNum);
    vos_printf("\n 收到非法数据包总数       : %d", g_stNdClientStatInfo.ulRevInvalidPackageNum);

    vos_printf("\n 启动Timer次数            : %d", g_stNdClientStatInfo.ulTimerNum);
    vos_printf("\n Timer超时处理次数        : %d", g_stNdClientStatInfo.ulTimeoutNum);

    vos_printf("\n 动态分配内存次数         : %d", g_stNdClientStatInfo.ulMemAllocNum);
    vos_printf("\n 动态释放内存次数         : %d", g_stNdClientStatInfo.ulMemFreeNum);
    vos_printf("\n****************************\n");

    ucExEpsbId = ND_FORM_EXEPSID(ulModemId, ulEpsbId);
    ulIndex = IP_NDCLIENT_GET_ADDR_INFO_INDEX(ucExEpsbId);
    if (ulIndex >= IP_MAX_NDCLIENT_ENTITY_NUM)
    {
        vos_printf("\n EPSID %d对应的实体已释放",ulEpsbId);
        return;
    }

    vos_printf("\n**************ND CLIENT %d 统计信息*****************", ulEpsbId);
    vos_printf("\n 收到数据包总数           : %d", g_stNdClientEntity[ulIndex].stStats.ulRevPackageNum);
    vos_printf("\n 收到RA总数               : %d", g_stNdClientEntity[ulIndex].stStats.ulRevRaPackageNum);
    vos_printf("\n 收到ECHO REQUEST总数     : %d", g_stNdClientEntity[ulIndex].stStats.ulRevEchoReqPackageNum);
    vos_printf("\n 收到超长包总数           : %d", g_stNdClientEntity[ulIndex].stStats.ulRevTooBigPackageNum);
    vos_printf("\n 发送数据包总数           : %d", g_stNdClientEntity[ulIndex].stStats.ulSendPackageNum);
    vos_printf("\n 发送RS总数               : %d", g_stNdClientEntity[ulIndex].stStats.ulSendRsPackageNum);
    vos_printf("\n 发送ECHO REPLY总数       : %d", g_stNdClientEntity[ulIndex].stStats.ulSendEchoReplyPackageNum);
    vos_printf("\n 发送超长包响应总数       : %d", g_stNdClientEntity[ulIndex].stStats.ulSendICMPV6TooBigPackageNum);
    vos_printf("\n 丢弃数据包总数           : %d", g_stNdClientEntity[ulIndex].stStats.ulDiscPackageNum);
    vos_printf("\n 收到非本机数据包总数     : %d", g_stNdClientEntity[ulIndex].stStats.ulRevNotForMePackageNum);
    vos_printf("\n 收到非法数据包总数       : %d", g_stNdClientEntity[ulIndex].stStats.ulRevInvalidPackageNum);

    vos_printf("\n 启动Timer次数            : %d", g_stNdClientEntity[ulIndex].stStats.ulTimerNum);
    vos_printf("\n Timer超时处理次数        : %d", g_stNdClientEntity[ulIndex].stStats.ulTimeoutNum);

    vos_printf("\n 动态分配内存次数         : %d", g_stNdClientEntity[ulIndex].stStats.ulMemAllocNum);
    vos_printf("\n 动态释放内存次数         : %d", g_stNdClientEntity[ulIndex].stStats.ulMemFreeNum);
    vos_printf("\n****************************\n");

}


VOS_VOID IP_NDCLIENT_ShowEntityInfo(VOS_VOID)
{
    VOS_UINT32  ulLoop;

    vos_printf("\r\n");

    for(ulLoop = 0; ulLoop < IP_MAX_NDCLIENT_ENTITY_NUM; ulLoop++)
    {
        if(0 == g_stNdClientEntity[ulLoop].ucflag)
        {
            continue;
        }

        vos_printf("\r\n");
        vos_printf("\n**************NDCLIENT实体 %d 信息*****************", g_stNdClientEntity[ulLoop].ucEpsbId);
        if(IP_ICMPV6_TYPE_BUTT != g_stNdClientEntity[ulLoop].enMsgType)
        {
            vos_printf("\n 当前正在发送消息        : %d", g_stNdClientEntity[ulLoop].enMsgType);
        }
        vos_printf("\n 网侧分配接口ID   :%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[0],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[1],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[2],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[3],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[4],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[5],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[6],
                                    g_stNdClientEntity[ulLoop].aucInterfaceId[7]);
        vos_printf("\n Router IP   :%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[0],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[1],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[2],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[3],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[4],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[5],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[6],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[7],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[8],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[9],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[10],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[11],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[12],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[13],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[14],
                                    g_stNdClientEntity[ulLoop].aucRouterIpAddr[15]);
        vos_printf("\n Router生存期       : 0x%x", g_stNdClientEntity[ulLoop].usRouterLifeTime);
        vos_printf("\n 可达周期           : 0x%x", g_stNdClientEntity[ulLoop].ulReachableTimer);
        vos_printf("\n 重传间隔           : 0x%x", g_stNdClientEntity[ulLoop].ulReTransTime);
        vos_printf("\n RA中指示的MTU      : 0x%x", g_stNdClientEntity[ulLoop].ulMtu);

        vos_printf("\r\n");
    }

    return;
}


VOS_VOID  IP_NDCLIENT_CmdHelp( VOS_VOID )
{
    vos_printf("\r\n");
    vos_printf("********************** NDCLIENT 软调命令列表 *********************\r\n");
    vos_printf("%-30s : %s\r\n","IP_NDCLIENT_ShowEntityInfo","显示NDCLIENT实体信息");
    vos_printf("%-30s : %s\r\n","IP_NDCLIENT_Stat(5~15)","显示NDCLIENT相关统计信息");
    vos_printf("******************************************************************\r\n");
    vos_printf("\r\n");

    return;
}






#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif /* end of IpNdClient.c */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

