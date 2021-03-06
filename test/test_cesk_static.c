#include <assert.h>
#include <adam.h>

int main()
{
	adam_init();
	/* we need to load a package first */
	assert(dalvik_loader_from_directory("test/cases/static") >= 0);
	/* then we examine the number of static fields */
	extern const uint32_t dalvik_static_field_count;
	assert(0 != dalvik_static_field_count);
	LOG_DEBUG("%u static field has been loaded", dalvik_static_field_count);
	/* ok, then try to create a new static field */
	cesk_static_table_t* tab1 = cesk_static_table_fork(NULL);
	assert(NULL != tab1);
	/* then we try to fetch a static field from this table */
	uint32_t addr_I723 = cesk_static_field_query(stringpool_query("case0"), stringpool_query("I723"));
	assert(CESK_FRAME_REG_IS_STATIC(addr_I723));
	assert(addr_I723 == (CESK_FRAME_REG_STATIC_PREFIX | 723));
	const cesk_set_t* ro_ret = cesk_static_table_get_ro(tab1, addr_I723, 1); 
	assert(NULL != ro_ret);
	assert(1 == cesk_set_size(ro_ret));
	assert(cesk_set_contain(ro_ret, CESK_STORE_ADDR_POS));
	
	uint32_t addr_B999 = cesk_static_field_query(stringpool_query("case0"), stringpool_query("B999"));
	assert(addr_B999 == (CESK_FRAME_REG_STATIC_PREFIX |  1999));
	ro_ret = cesk_static_table_get_ro(tab1, addr_B999, 1); 
	assert(NULL != ro_ret);
	assert(1 == cesk_set_size(ro_ret));
	assert(cesk_set_contain(ro_ret, CESK_STORE_ADDR_ZERO));
	
	/* make a copy of this table */
	cesk_static_table_t* tab2 = cesk_static_table_fork(tab1);
	assert(NULL != tab2);
	assert(cesk_static_table_equal(tab1, tab2));

	/* now we can est write */
	uint32_t addr_I724 = cesk_static_field_query(stringpool_query("case0"), stringpool_query("I724"));
	cesk_set_t** rw_ret = cesk_static_table_get_rw(tab2, addr_I724, 1);
	assert(rw_ret != NULL);
	assert(0 == cesk_static_table_release_rw(tab2, addr_I724, *rw_ret));
	assert(cesk_static_table_equal(tab1, tab2));

	rw_ret = cesk_static_table_get_rw(tab2, addr_I723, 1);
	assert(rw_ret != NULL);
	assert(0 == cesk_set_push(*rw_ret, CESK_STORE_ADDR_NEG));
	assert(0 == cesk_static_table_release_rw(tab2, addr_I723, *rw_ret));
	assert(!cesk_static_table_equal(tab1, tab2));

	/* check the value set of case0.I723 in both table */
	const cesk_set_t* set1 = cesk_static_table_get_ro(tab1, addr_I723, 1);
	const cesk_set_t* set2 = cesk_static_table_get_ro(tab2, addr_I723, 1);
	assert(NULL != set1);
	assert(NULL != set2);
	assert(1 == cesk_set_size(set1));
	assert(2 == cesk_set_size(set2));
	assert(cesk_set_contain(set1, CESK_STORE_ADDR_POS));
	assert(cesk_set_contain(set2, CESK_STORE_ADDR_POS));
	assert(cesk_set_contain(set2, CESK_STORE_ADDR_NEG));

	/* test the iterator */
	uint32_t addr;
	cesk_static_table_iter_t iter;
	assert(NULL != cesk_static_table_iter(tab2, &iter));
	assert(ro_ret = cesk_static_table_iter_next(&iter, &addr));
	assert(addr == (CESK_FRAME_REG_STATIC_PREFIX | 723));
	assert(ro_ret = cesk_static_table_iter_next(&iter, &addr));
	assert(addr == (CESK_FRAME_REG_STATIC_PREFIX | 724));
	assert(ro_ret = cesk_static_table_iter_next(&iter, &addr));
	assert(addr == (CESK_FRAME_REG_STATIC_PREFIX | 1999));
	assert(NULL == cesk_static_table_iter_next(&iter, &addr));

	assert(NULL != cesk_static_table_iter(tab1, &iter));
	assert(ro_ret = cesk_static_table_iter_next(&iter, &addr));
	assert(addr == (CESK_FRAME_REG_STATIC_PREFIX | 723));
	assert(ro_ret = cesk_static_table_iter_next(&iter, &addr));
	assert(addr == (CESK_FRAME_REG_STATIC_PREFIX | 1999));
	assert(NULL == cesk_static_table_iter_next(&iter, &addr));

	/* check the hash code */
	assert(cesk_static_table_compute_hashcode(tab1) == cesk_static_table_hashcode(tab1));
	assert(cesk_static_table_compute_hashcode(tab2) == cesk_static_table_hashcode(tab2));

	/* then make another copy of tab2 */
	cesk_static_table_t* tab3 = cesk_static_table_fork(tab2);
	assert(NULL != tab3);
	rw_ret = cesk_static_table_get_rw(tab3, addr_I723, 1);
	assert(rw_ret != NULL);
	cesk_set_free(*rw_ret);
	assert(*rw_ret = cesk_set_empty_set());
	assert(0 == cesk_set_push(*rw_ret, CESK_STORE_ADDR_POS));
	assert(0 == cesk_static_table_release_rw(tab2, addr_I723, *rw_ret));
	assert(cesk_static_table_equal(tab1, tab3));

	/* finally check wether or not tab3 equals to empty tab */
	cesk_static_table_t* tab4 = cesk_static_table_fork(NULL);
	assert(cesk_static_table_equal(tab4, tab3));
	assert(cesk_static_table_equal(tab1, tab4));


	cesk_static_table_free(tab1);
	cesk_static_table_free(tab2);
	cesk_static_table_free(tab3);
	cesk_static_table_free(tab4);
	adam_finalize();
	return 0;
}
