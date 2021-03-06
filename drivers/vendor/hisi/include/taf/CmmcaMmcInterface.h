


#ifndef __CMMCA_MMC_INTERFACE_H__
#define __CMMCA_MMC_INTERFACE_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include  "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum CMMCA_IRAT_SYSTEM_CHG_RESULT_ENUM
{
    CMMCA_IRAT_SYSTEM_CHG_SUCCESS,
    CMMCA_IRAT_SYSTEM_CHG_ACQ_FAIL,
    CMMCA_IRAT_SYSTEM_CHG_PS_REG_FAIL,
    CMMCA_IRAT_SYSTEM_CHG_PS_SESSION_FAIL,
    CMMCA_IRAT_SYSTEM_CHG_FAIL,

    CMMCA_IRAT_SYSTEM_CHG_RESULT_BUTT
};

typedef VOS_UINT8  CMMCA_IRAT_SYSTEM_CHG_RESULT_ENUM_UINT8;


enum CMMCA_MMC_MSG_TYPE_ENUM
{
    /* CMMCA->MMC CDMA MSG: */
    ID_CMMCA_MMC_RESUME_IND,                                                    /* _H2ASN_MsgChoice CMMCA_MMC_RESUME_IND_STRU */

    ID_CMMCA_MMC_MSG_TYPE_BUTT
};

typedef VOS_UINT32  CMMCA_MMC_MSG_TYPE_ENUM_UINT32;


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
    CMMCA_IRAT_SYSTEM_CHG_RESULT_ENUM_UINT8       enResult;
}CMMCA_MMC_RAT_IRAT_SYSTEM_CHG_RSP_STRU;



typedef struct
{
    VOS_MSG_HEADER /*_H2ASN_Skip*/
    CMMCA_MMC_MSG_TYPE_ENUM_UINT32      enMsgId; /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserve[4];
}CMMCA_MMC_RESUME_IND_STRU;


typedef struct
{
    CMMCA_MMC_MSG_TYPE_ENUM_UINT32      enMsgID;                                /*_H2ASN_MsgChoice_Export CMMCA_MMC_MSG_TYPE_ENUM_UINT32*/
    VOS_UINT8                           aucMsg[4];
/***************************************************************************
_H2ASN_MsgChoice_When_Comment          CMMCA_MMC_MSG_TYPE_ENUM_UINT32
****************************************************************************/
}CMMCA_MMC_MSG_DATA;
/*_H2ASN_Length UINT32*/


typedef struct
{
    VOS_MSG_HEADER
    CMMCA_MMC_MSG_DATA                  stMsgData;
}CmmcaMmcInterface_MSG;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/








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

#endif /* end of CmmcaMmcInterface.h */

