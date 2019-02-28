#include <stdlib.h>
#include <check.h>
#include <stdlib.h>

#include <message.h>
#include <perf_ex.h>

START_TEST(test_perf_ex_01_init)
	message_obj msg;
	perf_ex_obj perf_ex;

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);
END_TEST

static Suite *perf_ex_01_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("performance execution");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_perf_ex_01_init);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = perf_ex_01_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
