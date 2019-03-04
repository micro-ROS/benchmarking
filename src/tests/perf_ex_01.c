#include <assert.h>
#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdlib.h>

#include <message.h>
#include <perf_ex.h>

#include <libswo/libswo.h>


typedef void (*test_func) (void);

static void test_perf_ex_01_init_fini(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}

static void test_perf_ex_01_double_init(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	assert(perf_ex_init(&perf_ex) == -1);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}

static void test_perf_ex_01_double_fini(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
	assert(perf_ex_fini(&perf_ex) == -1);
}

static void test_perf_ex_01_data_in_no_tc(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	test_packets = (union libswo_packet *) msg.ptr(&msg);

	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		test_packet.pc_sample.pc = 0x08000004 + i;
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	assert(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg) == -1);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}


static void test_perf_ex_01_data_in_tc_increasing_addresses(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	assert(perf_ex.set_tc(&perf_ex, "arm-none-eabi-") == 0);

	test_packets = (union libswo_packet *) msg.ptr(&msg);

	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		test_packet.pc_sample.pc = 0x08000320 + i;
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	assert(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg) > 0);

	perf_ex.proc_obj.req_end = true;
	assert(perf_ex.proc_obj.data_out(&perf_ex.proc_obj, &msg) > 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}

static void test_perf_ex_01_data_in_tc_decreasing_addresses(void)
{
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	assert(perf_ex.set_tc(&perf_ex, "arm-none-eabi-") == 0);

	test_packets = (union libswo_packet *) msg.ptr(&msg);

	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		test_packet.pc_sample.pc = 0x08000340 - i;
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	assert(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg) > 0);

	perf_ex.proc_obj.req_end = true;
	assert(perf_ex.proc_obj.data_out(&perf_ex.proc_obj, &msg) > 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}


static void test_perf_ex_01_data_in_tc_not_ordered_addresses_no_overlap(void) 
{
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	assert(perf_ex.set_tc(&perf_ex, "arm-none-eabi-") == 0);

	test_packets = (union libswo_packet *) msg.ptr(&msg);
	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		if (i%2) {
			test_packet.pc_sample.pc = 0x08000340 - i * 4;
		} else {
			test_packet.pc_sample.pc = 0x08000320 + i * 4;
		}
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	assert(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg) > 0);

	perf_ex.proc_obj.req_end = true;
	assert(perf_ex.proc_obj.data_out(&perf_ex.proc_obj, &msg) > 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}

static void test_perf_ex_01_data_in_tc_not_ordered_addresses_overlap(void) 
{
	message_obj msg;
	perf_ex_obj perf_ex;
	union libswo_packet *test_packets;
	union libswo_packet test_packet = {
		.pc_sample = {
			.type = LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE,
			.size = 8,
		}
	};

	assert(message_init(&msg) == 0);
	assert(perf_ex_init(&perf_ex) == 0);
	assert(perf_ex.set_tc(&perf_ex, "arm-none-eabi-") == 0);

	test_packets = (union libswo_packet *) msg.ptr(&msg);
	for (unsigned int i = 0; i < msg.total_len(&msg)/sizeof(test_packet); i++) {
		if (i%2) {
			test_packet.pc_sample.pc = 0x08000325 - i;
		} else {
			test_packet.pc_sample.pc = 0x08000320 + i;
		}
		memcpy(&test_packets[i], &test_packet, sizeof(test_packet)); 
	}

	msg.set_length(&msg, msg.total_len(&msg)/sizeof(test_packet));
	assert(perf_ex.proc_obj.data_in(&perf_ex.proc_obj, &msg) > 0);

	perf_ex.proc_obj.req_end = true;
	assert(perf_ex.proc_obj.data_out(&perf_ex.proc_obj, &msg) > 0);

	assert(message_fini(&msg) == 0);
	assert(perf_ex_fini(&perf_ex) == 0);
}

static test_func ftests[] = {	
	test_perf_ex_01_init_fini,
	test_perf_ex_01_double_init,
	test_perf_ex_01_double_fini,
	test_perf_ex_01_data_in_no_tc,
	test_perf_ex_01_data_in_tc_increasing_addresses,
	test_perf_ex_01_data_in_tc_decreasing_addresses,
	test_perf_ex_01_data_in_tc_not_ordered_addresses_no_overlap,
	test_perf_ex_01_data_in_tc_not_ordered_addresses_overlap,
	NULL,
};

int main(void)
{
	unsigned int i = 0;

	while (ftests[i]) {
		ftests[i++]();
	}

	return 0;
}
