#include <stdlib.h>
#include <check.h>
#include <stdlib.h>

#include <message.h>
#include <perf_ex.h>

#include <libswo/libswo.h>

START_TEST(test_perf_ex_01_init_fini)
	message_obj msg;
	perf_ex_obj perf_ex;

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);

	ck_assert_int_eq(message_fini(&msg), 0);
	ck_assert_int_eq(perf_ex_fini(&perf_ex), 0);
END_TEST

START_TEST(test_perf_ex_01_double_init)
	message_obj msg;
	perf_ex_obj perf_ex;

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), -1);

	ck_assert_int_eq(message_fini(&msg), 0);
	ck_assert_int_eq(perf_ex_fini(&perf_ex), 0);
END_TEST

START_TEST(test_perf_ex_01_double_fini)
	message_obj msg;
	perf_ex_obj perf_ex;

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);

	ck_assert_int_eq(message_fini(&msg), 0);
	ck_assert_int_eq(perf_ex_fini(&perf_ex), 0);
	ck_assert_int_eq(perf_ex_fini(&perf_ex), -1);
END_TEST

START_TEST(test_perf_ex_01_data_in_no_tc)
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);
	test_packets = (union libswo_packet *) msg.ptr(&msg);

	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		test_packet.pc_sample.pc = 0x08000004 + i;
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	ck_assert_int_eq(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg), -1);

END_TEST

START_TEST(test_perf_ex_01_data_in_tc)
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	ck_assert_int_eq(message_init(&msg), 0);
	ck_assert_int_eq(perf_ex_init(&perf_ex), 0);

	ck_assert_int_eq(perf_ex.set_tc(&perf_ex, "arm-none-eabi-"), 0);

	test_packets = (union libswo_packet *) msg.ptr(&msg);

	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		test_packet.pc_sample.pc = 0x08000004 + i;
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	ck_assert_int_gt(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg), 0);

END_TEST

static Suite *perf_ex_01_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Performance-execution");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_perf_ex_01_init_fini);
    tcase_add_test(tc_core, test_perf_ex_01_double_init);
    tcase_add_test(tc_core, test_perf_ex_01_double_fini);
    tcase_add_test(tc_core, test_perf_ex_01_data_in_no_tc);
    tcase_add_test(tc_core, test_perf_ex_01_data_in_tc);

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
