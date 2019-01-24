#include <check.h>
#include <stdlib.h>

#include "pvm.h"

#define ck_assert_cpu_eq(CPU, NZVC, A, X, PC, SP, IR, OPSPEC)\
  ck_assert_uint_eq((CPU).nzvc, NZVC);\
  ck_assert_uint_eq((CPU).a, A);\
  ck_assert_uint_eq((CPU).x, X);\
  ck_assert_uint_eq((CPU).pc, PC);\
  ck_assert_uint_eq((CPU).sp, SP);\
  ck_assert_uint_eq((CPU).ir, IR);\
  ck_assert_uint_eq((CPU).opspec, OPSPEC)

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

START_TEST(t_init_w_default_os)
{
  int ret;
  struct cpu cpu;

  /* Get initial state of cpu. */
  ret = vm.examine(CPU, &cpu);
  ck_assert_int_eq(ret, 0);

  /* Check initial state of cpu. */
  ck_assert_cpu_eq(cpu, 0x00, 0x0000, 0x0000, 0x0000, 0xfb8f, 0x00, 0x0000);
}
END_TEST

START_TEST(t_load_w_null_prog)
{
  int ret;

  /* Call with size of 1, so only the prog pointer will cause error. */
  ret = vm.load(NULL, 1);
  ck_assert_int_ne(ret, 0);
}
END_TEST

START_TEST(t_load_w_zero_prog_len)
{
  int ret;

  /* Call with non-null prog pointer 1, so only the size will cause error. */
  ret = vm.load((void*)1, 0);
  ck_assert_int_ne(ret, 0);
}
END_TEST

START_TEST(t_step_unary)
{
  int ret;
  struct cpu cpu;
  char prog[] = { 0x00 }; /* STOP: 0000 0000 */

  /* Load program. */
  ret = vm.load(prog, sizeof(prog));
  ck_assert_int_eq(ret, 0);

  /* Step program. */
  ret = vm.step();
  ck_assert_int_eq(ret, 0);

  /* Get cpu state. */
  ret = vm.examine(CPU, &cpu);
  ck_assert_int_eq(ret, 0);

  /* Check cpu state. */
  ck_assert_cpu_eq(cpu, 0x00, 0x0000, 0x0000, 0x0001, 0xfb8f, 0x00, 0x0000);
}
END_TEST

START_TEST(t_step_nonunary)
{
  int ret;
  struct cpu cpu;
  char prog[] = { 0xd0, 0x00, 0x48 }; /* LDBA 'H',i */

  /* Load program. */
  ret = vm.load(prog, sizeof(prog));
  ck_assert_int_eq(ret, 0);

  /* Step program. */
  ret = vm.step();
  ck_assert_int_ne(ret, 0);

  /* Get cpu state. */
  ret = vm.examine(CPU, &cpu);
  ck_assert_int_eq(ret, 0);

  /* Check cpu state. */
  ck_assert_cpu_eq(cpu, 0x00, 0x0048, 0x0000, 0x0003, 0xfb8f, 0xd0, 0x0048);
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
  tcase_add_test(tc, t_init_w_default_os);
  tcase_add_test(tc, t_load_w_null_prog);
  tcase_add_test(tc, t_load_w_zero_prog_len);
  tcase_add_test(tc, t_step_unary);
  tcase_add_test(tc, t_step_nonunary);

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
