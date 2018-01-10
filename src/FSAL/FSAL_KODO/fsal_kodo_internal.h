/*******************************************************************************************
 *  @file      fsal_kodo_internal.h 2017\12\20 15:23:19 $
 *  @author    Wang Xiaotao<wangxiaotao1980@gmail.com> (中文编码测试)
 *  @note      summary
 *******************************************************************************************/
#ifndef  FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__
#define  FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__

/*******************************************************************************************/

#include "../../include/fsal.h"
#include "../../include/fsal_types.h"
#include "../../include/fsal_api.h"
#include "../../include/fsal_convert.h"
#include "../../include/sal_data.h"

/**
 * KODO Main (global) module object
 */

struct kodo_fsal_module
{
    struct fsal_module  fsal;
    fsal_staticfsinfo_t fs_info;
};

extern struct kodo_fsal_module KODOFSM;

/*******************************************************************************************/
#endif// FSAL_KODO_INTERNAL_CD7F7735_C8E9_43E2_9635_0ADBD791521F_H__