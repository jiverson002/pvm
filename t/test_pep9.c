#include <check.h>
#include <stdlib.h>

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
