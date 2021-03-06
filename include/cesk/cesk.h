/**
 * @file cesk.h
 * @brief top level header file of CESK module
 **/
#include <log.h>
#include <stringpool.h>

#include <cesk/cesk_alloc_param.h>
#include <cesk/cesk_reloc.h>
#include <cesk/cesk_frame.h>
#include <cesk/cesk_diff.h>
#include <cesk/cesk_object.h>
#include <cesk/cesk_value.h>
#include <cesk/cesk_store.h>
#include <cesk/cesk_set.h>
#include <cesk/cesk_alloctab.h>
#include <cesk/cesk_arithmetic.h>
#include <cesk/cesk_block.h>
#include <cesk/cesk_method.h>
#include <cesk/cesk_static.h>


#ifndef __CESK_H__
#define __CESK_H__

/** @brief initialize 
 *  @return < 0 for error
 */
int cesk_init(void);
/** @brief finalize 
 *  @return nothing
 */
void cesk_finalize(void);
#endif
