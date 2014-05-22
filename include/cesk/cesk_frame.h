#ifndef __CESK_FRAME_H__
#define __CESK_FRAME_H__
typedef struct _cesk_frame_t cesk_frame_t;
#include <cesk/cesk_set.h>
#include <cesk/cesk_reloc.h>
#include <dalvik/dalvik_instruction.h>
#include <cesk/cesk_diff.h>
#include <const_assertion.h>
/** @file cesk_frame.h
 *  @brief A stack frame of the virtual machine
 *
 *  @details see 
 *  <a herf=http://source.android.com/devices/tech/dalvik/dalvik-bytecode.html>
 *  http://source.android.com/devices/tech/dalvik/dalvik-bytecode.html</a> for
 *  the detail of dalvik virtual machine.
 *
 *  We do not need any function for destory an object, because we have garbage 
 *  collector in the frame
 */
/** @brief the actual register id of result register */
#define CESK_FRAME_RESULT_REG 0
/** @brief the actual register id of exception register */
#define CESK_FRAME_EXCEPTION_REG 1
/** @brief convert a general register to acual index, so that you can use frame->regs[id] to visit the register
 *  @param id general register index
 *  @return acutal index, 
 */
#define CESK_FRAME_GENERAL_REG(id) (id + 2)

/** @brief A Stack Frame of The Dalvik CESK Machine */
struct _cesk_frame_t{
	uint32_t       size;     /*!<the number of registers in this frame */
	cesk_store_t*  store;    /*!<the store for this frame */ 
	cesk_set_t*    regs[0];  /*!<array of all registers, include result and exception */
	cesk_set_t*    reg_result; /*!<result register*/
	cesk_set_t*    reg_exception;  /*!<exception register*/
	cesk_set_t*    general_regs[0]; /*!<begin address of gneral registers */
};
CONST_ASSERTION_FOLLOWS(cesk_frame_t, regs, reg_result);
CONST_ASSERTION_FOLLOWS(cesk_frame_t, reg_result, reg_exception);
CONST_ASSERTION_FOLLOWS(cesk_frame_t, reg_exception, general_regs);
CONST_ASSERTION_SIZE(cesk_frame_t, regs, 0);
CONST_ASSERTION_SIZE(cesk_frame_t, general_regs, 0);

/** @brief duplicate the frame 
 *  @param frame input frame
 *  @return a copy of this frame
 */
cesk_frame_t* cesk_frame_fork(const cesk_frame_t* frame);

/** @brief create an empty stack which is empty
 *  @param size number of general registers 
 *  @return new frame
 */
cesk_frame_t* cesk_frame_new(uint16_t size);

/** @brief despose a cesk frame
 *  @param frame the frame to be freed
 *  @return nothing
 */
void cesk_frame_free(cesk_frame_t* frame);

/** @brief compare if two stack frame are equal 
 *  @param first the first frame 
 *  @param second the second frame
 *  @return 1 if first == second, 0 otherwise
 */
int cesk_frame_equal(const cesk_frame_t* first, const cesk_frame_t* second);

/** @brief run garbage collector on a frame 
 * @param frame
 * @return >=0 means success
 */
int cesk_frame_gc(cesk_frame_t* frame);

/** @brief the hash fucntion of this frame 
 *  @param frame
 *  @return the hash code of the frame
 */
hashval_t cesk_frame_hashcode(const cesk_frame_t* frame);

/** @brief the hash code compute without incremental style 
 *  @param frame
 *  @return the hash code of the frame */
hashval_t cesk_frame_compute_hashcode(const cesk_frame_t* frame);

/**
 * @brief apply a diff to this frame
 * @todo implmentation
 * @param frame
 * @param diff
 * @param reloctable the relocation table
 * @return the result of operation <0 indicates error
 **/
int cesk_frame_apply_diff(cesk_frame_t* frame, const cesk_diff_t* diff, const cesk_reloc_table_t* reloctab);
/**
 * @brief move the content in source register to the destination register
 * @param frame 
 * @param dst_reg destination register
 * @param src_reg source register
 * @param diff_buf the the diff buffer
 * @param inv_buf the inverse diff buffer
 * @return < 0 indicates errors
 **/
int cesk_frame_register_move(
		cesk_frame_t* frame, 
		uint32_t dst_reg, 
		uint32_t src_reg, 
		cesk_diff_buffer_t* diff_buf, 
		cesk_diff_buffer_t* inv_buf);
/**
 * @brief clear the register, make the value set of the register is empty 
 * @param frame
 * @param dst_reg destination register
 * @param diff_buf the diff buffer
 * @param inv_buf  the inverse diff buffer
 * @return < 0 indicates errors 
 **/
int cesk_frame_register_clear(
		cesk_frame_t* frame,
		uint32_t dst_reg,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);
/**
 * @brief load a constant to the register
 * @param frame
 * @param dst_reg destination register
 * @param src_addr source instant number address
 * @param diff_buf the diff buffer
 * @param inv_buf the inverse diff buffer
 * @return < 0 indicates errors
 **/
int cesk_frame_register_load(
		cesk_frame_t* frame,
		uint32_t dst_reg,
		uint32_t src_addr,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);
/**
 * @brief push a value to the register
 * @param frame
 * @param dst_reg destination register
 * @param src_addr the source constant
 * @param diff_buff the diff buffer
 * @param inv_buf the inverse diff buffer
 * @return < 0 indicates errors
 **/
int cesk_frame_register_push(
		cesk_frame_t* frame,
		uint32_t dst_reg,
		uint32_t src_addr,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);

/** 
 * @brief append a value to store
 * @param frame
 * @param dst_reg destination
 * @param src_addr source store address
 * @param diff_buf
 * @param inv_buf
 * @return < 0 indicates error
 **/
int cesk_frame_register_append_from_store(
		cesk_frame_t* frame,
		uint32_t dst_reg,
		uint32_t src_addr,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);

/**
 * @brief load a object field from object bearing register to destiniation register
 * @param frame
 * @param dst_reg destination register
 * @param src_reg source boject bearing register
 * @param clspath the class path
 * @param fldname the field name
 * @param diff_buf
 * @param inv_buf
 * @return < 0 indicates error
 **/
int cesk_frame_register_load_from_object(
		cesk_frame_t* frame,
		uint32_t dst_reg,
		uint32_t src_reg,
		const char* clspath,
		const char* fldname,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);

/**
 * @brief allocate a new object in the frame store
 * @param frame
 * @param reloctab the relocation table
 * @param inst current instruction
 * @param clspath the class path
 * @param diff_buf
 * @param inv_buf
 * @return the address of newly created object , CESK_STORE_ADDR_NULL indicates an error
 **/
int cesk_frame_store_new_object(
		cesk_frame_t* frame,
		cesk_reloc_table_t* reloctab,
		const dalvik_instruction_t* inst,
		const char* clspath,
		cesk_diff_buffer_t* diff_buf,
		cesk_diff_buffer_t* inv_buf);

#endif /* __CESK_FRAME_H__ */
