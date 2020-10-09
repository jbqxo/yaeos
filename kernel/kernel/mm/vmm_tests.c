// UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
// UNITY_TEST DEPENDS ON: kernel/test_fakes/kmm.c

// UNITY_TEST DEPENDS ON: kernel/kernel/mm/vmm.c
// UNITY_TEST DEPENDS ON: kernel/kernel/ds/rbtree.c

#include "kernel/mm/vmm.h"

#include "kernel/mm/kmm.h"

#include <assert.h>
#include <stdlib.h>
#include <unity.h>

void setUp(void)
{
	vmm_init();
}

void tearDown(void)
{}

static bool rbt_empty(struct vm_space *vms)
{
	return (vms->vmappings.tree.root == NULL);
}

static int vmspace_mappings_count(struct vm_space *vms)
{
	int counter = 0;
	struct vm_mapping *it;
	vm_space_mappings_foreach(vms, it)
	{
		TEST_ASSERT_NOT_NULL(it->start);
		TEST_ASSERT_GREATER_THAN_INT(0, it->length);
		counter++;
	}
	return (counter);
}

static bool vmspace_empty(struct vm_space *vms)
{
	return (rbt_empty(vms) && vmspace_mappings_count(vms) == 0);
}

static void allocate_single_page(void)
{
	struct vm_space vspace = vm_space_new(0x0);
	struct vm_mapping *m = vmm_alloc_pages(&vspace, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(m, "Allocated mapping doesn't point anywhere");

	vmm_free_mapping(&vspace, m);

	TEST_ASSERT_TRUE_MESSAGE(vmspace_empty(&vspace), "Unknown mappings left");
	TEST_ASSERT_TRUE(rbt_empty(&vspace));
}

static void free_single_by_ptr(void)
{
	struct vm_space vspace = vm_space_new(0x0);
	struct vm_mapping *m = vmm_alloc_pages(&vspace, 1);
	TEST_ASSERT_NOT_NULL(m);
	vmm_free_pages(&vspace, m->start, 1);
	TEST_ASSERT_EQUAL_INT(0, vmspace_mappings_count(&vspace));
}

static void allocate_many_pages(void)
{
	struct vm_space vspace = vm_space_new(0x0);
	struct vm_mapping *m = vmm_alloc_pages(&vspace, 10);
	TEST_ASSERT_NOT_NULL(m);
	vmm_free_mapping(&vspace, m);
	TEST_ASSERT_EQUAL_INT(0, vmspace_mappings_count(&vspace));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(allocate_single_page);
	RUN_TEST(allocate_many_pages);
	RUN_TEST(free_single_by_ptr);
	UNITY_END();
	return (0);
}
