#include <check.h>
#include <stdlib.h>

#include "pvm.h"

static struct vm vm;

void setup(void)
{
  int ret;

  /* Select the Pep/9 virtual machine. */
  vm = pep9;

  /* Initialize the VM. */
  ret = vm.init();
  ck_assert_int_eq(ret, 0);
}

START_TEST(test_1)
{
}
END_TEST

int main(void)
{
  int number_failed;

  /* Create objects. */
  Suite *s    = suite_create("Core");
  TCase *tc   = tcase_create("Core");
  SRunner *sr = srunner_create(s);

  /* Create text fixtures. */
  tcase_add_checked_fixture(tc, setup, NULL);

  /* Add tests. */
  tcase_add_test(tc, test_1);

  /* Populate suite. */
  suite_add_tcase(s, tc);

  /* Run suite. */
  srunner_run_all(sr, CK_NORMAL);

  /* Check results. */
  number_failed = srunner_ntests_failed(sr);

  /* Free resources. */
  srunner_free(sr);
 
  return (0 == number_failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}
