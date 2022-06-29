/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** Connection handle for a UDP Server session */

#include "udp_perf_server.h"
#include "dlpack.h"

extern struct netif server_netif;
static struct udp_pcb *pcb;
static struct perf_stats server;

static struct python_header pac_head;
u8_t* image_frame_buf0 = NULL;
u8_t* image_frame_buf1 = NULL;

struct DLManagedTensor dlpack0;
struct DLManagedTensor dlpack1;

//struct DLManagedTensor dlpack0_to_initialize;
//struct DLManagedTensor dlpack1_to_initialize;
//
//dlpack0 = dlpack0_to_initialize;
//dlpack1 = dlpack1_to_initialize;

u8_t * udp_payload = NULL;
u8_t * image_frame;
u8_t make_frame_first = 1;
u8_t python_header_first = 1;
u32_t frame_size = 0;
u32_t previousFrameSize = 0;
u16_t i = 0;
u8_t drop_datagrams = 0;
u8_t first_byte = 0;
u8_t buf_flag = 0;

/* Report interval in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)

void print_app_header(void)
{
	xil_printf("UDP server listening on port %d\r\n",
			UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -u -b <bandwidth>\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);

}

static void print_udp_conn_stats(void)
{
	xil_printf("[%3d] local %s port %d connected with ",
			server.client_id, inet_ntoa(server_netif.ip_addr),
			UDP_CONN_PORT);
	xil_printf("%s port %d\r\n", inet_ntoa(pcb->remote_ip),
			pcb->remote_port);
	xil_printf("[ ID] Interval\t     Transfer     Bandwidth\t");
	xil_printf("    Lost/Total Datagrams\n\r");
}

void make_frame_image(u8_t *udp_payload )
{

	//xil_priu8_t first_byte = 0;ntf("%x " , udp_payload[0]);
	previousFrameSize = frame_size;
	frame_size = pac_head.width_frame * pac_head.height_frame * 64;
	if (((pac_head.seq_num == 0 || i == 0 ) && make_frame_first == 1 )||(frame_size != previousFrameSize)) {
		if (make_frame_first == 0) {
			xil_printf("Memory freed allocated for image_frame due to frame mismatch \r\n");

			free(image_frame_buf1);
			free(image_frame_buf0);
		}

		image_frame_buf0 =  malloc (frame_size);
		image_frame_buf1 =  malloc (frame_size);
		//image_frame = malloc (frame_size);
		make_frame_first = 0;
		if (image_frame_buf0 == NULL || image_frame_buf1 == NULL ) {
			xil_printf("Memory not allocated for image_frame \r\n");
			exit(0);
		}
		xil_printf(" MEMORY ALLOCATED for Image_frame_buf \r\n");
	}
	if (buf_flag == 0 && pac_head.frame_sequence == i) {
		if ((pac_head.frame_sequence <= (frame_size/1400))){
			//xil_printf(" %d %d", i, pac_head.frame_sequence);
			if (pac_head.frame_sequence == 0 && pac_head.marker_bit == 0){
//				xil_printf (" Start of frame capture ");
				memcpy (image_frame_buf0 , udp_payload , 1400);
				xil_printf("SOF %x ,%x \r\n",pac_head.seq_num,  image_frame_buf0[0]);
				first_byte = image_frame_buf0[0];
				i++;
			}
			else if ((pac_head.frame_sequence > 0) && (pac_head.frame_sequence < (frame_size/1400)) ){
				memcpy (image_frame_buf0+(1400*i) , udp_payload , 1400);
				//xil_printf(" %x ", image_frame_buf[0]);
				i++;
				if (first_byte != image_frame_buf0[0]) {
					xil_printf("first byte mismatch %x ", image_frame_buf0[0]);
				}
			}
			else if (pac_head.marker_bit == 1 && i == (frame_size/1400))
			{
				memcpy (image_frame_buf0+(1400*i) , udp_payload , (frame_size - 1400*i));
				//memcpy (image_frame , image_frame_buf , frame_size);
				//print_image_frame();
//				xil_printf("%x, %x EOF \r\n",image_frame_buf0[0], image_frame_buf0[frame_size-1]);
//				xil_printf (" EOF \r\n\r\n");
				i = 0;
				int64_t width = (int64_t) pac_head.width_frame*8;
				int64_t height = (int64_t) pac_head.height_frame*8;
				printf("in function width %"PRId64",height %"PRId64"\r\n",width, height);
				create_and_allocate_dlpack_tensor(&dlpack0, width, height, image_frame_buf0);
				xil_printf(" in udp serv function  dl_tensor data %x , address %x ,  and image_frame_buf %x , address %x \r\n "
							, dlpack0.dl_tensor.data, &dlpack0.dl_tensor.data[0], image_frame_buf0[0], &image_frame_buf0[0]);
//				u8_t (*image_frame_buf0_2d)[pac_head.height_frame*8]= (u8_t (*)[pac_head.height_frame*8])image_frame_buf0;
//				int (*newArray)[N] = (int (*)[N])array;
//				xil_printf("address of buffers%x, %x EOF \r\n",&image_frame_buf0[0],  &dlpack0.dl_tensor.data);
//				for(int y = 0; y < (pac_head.width_frame *8); y++) {
//				    for(int x = 0; x < (pac_head.height_frame*8); x++) {
//				    	xil_printf("2d frame %x  \r\n", image_frame_buf0_2d[x][y]);
//				    }
//				}
				buf_flag = 1;
			}
			else
			{
				xil_printf (" frame sequence out of bounds \r\n");
			}

		}
	}
	else if (buf_flag == 1 && pac_head.frame_sequence == i){
		if ((pac_head.frame_sequence <= (frame_size/1400))){
			//xil_printf(" %d %d", i, pac_head.frame_sequence);
			if (pac_head.frame_sequence == 0 && pac_head.marker_bit == 0){
//				xil_printf (" Start of frame capture ");
				memcpy (image_frame_buf1 , udp_payload , 1400);
				xil_printf("SOF %x ,%x \r\n ",pac_head.seq_num, image_frame_buf1[0]);
				first_byte = image_frame_buf1[0];
				i++;
			}
			else if ((pac_head.frame_sequence > 0) && (pac_head.frame_sequence < (frame_size/1400)) ){
				memcpy (image_frame_buf1+(1400*i) , udp_payload , 1400);
				//xil_printf(" %x ", image_frame_buf[0]);
				i++;
				if (first_byte != image_frame_buf1[0]) {
					xil_printf(" %x ", image_frame_buf1[0]);
				}
			}
			else if (pac_head.marker_bit == 1 && i == (frame_size/1400))
			{
				memcpy (image_frame_buf1+(1400*i) , udp_payload , (frame_size - 1400*i));
				//memcpy (image_frame , image_frame_buf , frame_size);
				//print_image_frame();
//				xil_printf("%x, %x EOF \r\n", image_frame_buf1[0], image_frame_buf1[frame_size-1]);
//				xil_printf (" End of frame capture \r\n\r\n");
				i = 0;
				int64_t width = pac_head.width_frame*8;
				int64_t height = pac_head.height_frame*8;
				printf("in function width %"PRId64",height %"PRId64"\r\n",width, height);
				create_and_allocate_dlpack_tensor(&dlpack1, width , height , image_frame_buf1);
				xil_printf(" in udp serv fn dl_tensor data %x , address %x ,  and image_frame_buf %x , address %x \r\n "
							, dlpack1.dl_tensor.data, &dlpack1.dl_tensor.data[0], image_frame_buf1[0], &image_frame_buf1[0]);
//				u8_t (*image_frame_buf1_2d)[pac_head.height_frame*8]= (u8_t (*)[pac_head.height_frame*8])image_frame_buf1;
//				xil_printf("address of buffers%x, %x EOF \r\n",&image_frame_buf1[0], &dlpack1.dl_tensor.data);
//				for(int y = 0; y < (pac_head.width_frame *8); y++) {
//					for(int x = 0; x < (pac_head.height_frame*8); x++) {
//						xil_printf("2d frame %x \r\n", image_frame_buf1_2d[x][y]);
//					}
//				}
				buf_flag = 0;
			}
			else
			{
				xil_printf (" frame sequence out of bounds \r\n");
			}
		}
	}
	else
	{
		drop_datagrams ++ ;

		if (i != 0) {
//			xil_printf(" %d %d", i, pac_head.frame_sequence);
			xil_printf (" packet dropped \r\n ");
			i = 0;
		}
	}

	//xil_printf("exit ");
}
void decode_python_header(u8_t *recv_buf )
{
	if (python_header_first == 1 ){
		python_header_first = 0;
		udp_payload = malloc (UDP_RECV_BUFSIZE );
		if (udp_payload == NULL) {
			xil_printf("Memory not allocated for payload");
			exit(0);
		}
		xil_printf(" MEMORY ALLOCATED for UDP_PAYLOAD \r\n");
	}
//	udp_payload = malloc (UDP_RECV_BUFSIZE );
//	if (udp_payload == NULL) {
//		xil_printf("Memory not allocated for payload");
//		exit(0);
//	}

	pac_head.seq_num = (((unsigned short)(*recv_buf++))<<24) | \
						(((unsigned short)(*recv_buf++))<<16) | \
						(((unsigned short)(*recv_buf++))<<8) | \
						(*recv_buf++);
	pac_head.marker_bit = *recv_buf++;
	pac_head.frame_sequence = ( 0x00000000 <<24) |
							(((unsigned short)(*recv_buf++))<<16) |
							(((unsigned short)(*recv_buf++))<<8) |
							(*recv_buf++);
	pac_head.width_frame = (*recv_buf++);
	pac_head.height_frame = (*recv_buf++);
	//xil_printf(" %d , %d ", pac_head.width_frame , pac_head.height_frame);
	//xil_printf(" %x ", udp_payload [0]);
	memcpy (udp_payload , recv_buf , UDP_RECV_BUFSIZE);
	make_frame_image(udp_payload);
	//xil_printf("exit from header decode");
	//free(udp_payload);

}

static void stats_buffer(char* outString,
		double data, enum measure_t type)
{
	int conv = KCONV_UNIT;
	const char *format;
	double unit = 1024.0;

	if (type == SPEED)
		unit = 1000.0;

	while (data >= unit && conv <= KCONV_GIGA) {
		data /= unit;
		conv++;
	}

	/* Fit data in 4 places */
	if (data < 9.995) { /* 9.995 rounded to 10.0 */
		format = "%4.2f %c"; /* #.## */
	} else if (data < 99.95) { /* 99.95 rounded to 100 */
		format = "%4.1f %c"; /* ##.# */
	} else {
		format = "%4.0f %c"; /* #### */
	}
	sprintf(outString, format, data, kLabel[conv]);
}


/** The report function of a TCP server session */
static void udp_conn_report(u64_t diff,
		enum report_type report_type)
{
	u64_t total_len, cnt_datagrams, cnt_dropped_datagrams, total_packets;
	u32_t cnt_out_of_order_datagrams;
	double duration, bandwidth = 0;
	char data[16], perf[16], time[64], drop[64];

	if (report_type == INTER_REPORT) {
		total_len = server.i_report.total_bytes;
		cnt_datagrams = server.i_report.cnt_datagrams;
		cnt_dropped_datagrams = server.i_report.cnt_dropped_datagrams;
	} else {
		server.i_report.last_report_time = 0;
		total_len = server.total_bytes;
		cnt_datagrams = server.cnt_datagrams;
		cnt_dropped_datagrams = server.cnt_dropped_datagrams;
		cnt_out_of_order_datagrams = server.cnt_out_of_order_datagrams;
	}

	total_packets = cnt_datagrams + cnt_dropped_datagrams;
	/* Converting duration from milliseconds to secs,
	 * and bandwidth to bits/sec .
	 */
	duration = diff / 1000.0; /* secs */
	if (duration)
		bandwidth = (total_len / duration) * 8.0;

	stats_buffer(data, total_len, BYTES);
	stats_buffer(perf, bandwidth, SPEED);
	/* On 32-bit platforms, xil_printf is not able to print
	 * u64_t values, so converting these values in strings and
	 * displaying results
	 */
	sprintf(time, "%4.1f-%4.1f sec",
			(double)server.i_report.last_report_time,
			(double)(server.i_report.last_report_time + duration));
	sprintf(drop, "%4llu/%5llu (%.2g%%)", cnt_dropped_datagrams,
			total_packets,
			(100.0 * cnt_dropped_datagrams)/total_packets);
	xil_printf("[%3d] %s  %sBytes  %sbits/sec  %s\n\r", server.client_id,
			time, data, perf, drop);

	if (report_type == INTER_REPORT) {
		server.i_report.last_report_time += duration;
	} else if ((report_type != INTER_REPORT) && cnt_out_of_order_datagrams) {
		xil_printf("[%3d] %s  %u datagrams received out-of-order\n\r",
				server.client_id, time,
				cnt_out_of_order_datagrams);
	}
}




static void reset_stats(void)
{
	server.client_id++;
	/* Save start time */
	server.start_time = get_time_ms();
	server.end_time = 0; /* ms */
	server.total_bytes = 0;
	server.cnt_datagrams = 0;
	server.cnt_dropped_datagrams = 0;
	server.cnt_out_of_order_datagrams = 0;
	server.expected_datagram_id = 0;

	/* Initialize Interim report parameters */
	server.i_report.start_time = 0;
	server.i_report.total_bytes = 0;
	server.i_report.cnt_datagrams = 0;
	server.i_report.cnt_dropped_datagrams = 0;
	server.i_report.last_report_time = 0;
}

/** Receive data on a udp session */
static void udp_recv_perf_traffic(void *arg, struct udp_pcb *tpcb,
		struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	static u8_t first = 1;
	u32_t drop_datagrams = 0;
	s32_t recv_id;

	/* first, check if the datagram is received in order */
#ifdef __MICROBLAZE__
	/* For Microblaze, word access are at 32 bit boundaries.
	 * To read complete 4 byte of UDP ID from data payload,
	 * we should read upper 2 bytes from current word boundary
	 * of payload and lower 2 bytes from next word boundary of
	 * payload.
	 */
	s16_t *payload;
	payload = (s16_t *) (p->payload);
	recv_id = (ntohs(payload[0]) << 16) | ntohs(payload[1]);
#else
	recv_id = ntohl(*((int *)(p->payload)));
#endif
	if (first && (recv_id == 0)) {
		/* First packet should always start with recv id 0.
		 * However, If Iperf client is running with parallel
		 * thread, then this condition will also avoid
		 * multiple print of connection header
		 */
		pcb->remote_ip = *addr;
		pcb->remote_port = port;
		reset_stats();
		/* Print connection statistics */
		print_udp_conn_stats();
		first = 0;
	} else if (first) {
		/* Avoid rest of the packets if client
		 * connection is already terminated.
		 */
		free(udp_payload);
		free(image_frame_buf1);
		free(image_frame_buf0);
		make_frame_first = 1;
		python_header_first = 1;
		pac_head.frame_sequence = 0;
		xil_printf("in first else \n\r");
		return;
	}

	//xil_printf("before function call ");
	decode_python_header(p->payload);
	//xil_printf("after function call ");

	if (recv_id < 0) {
		u64_t now = get_time_ms();
		u64_t diff_ms = now - server.start_time;
		/* Send Ack */
		udp_sendto(tpcb, p, addr, port);
		udp_conn_report(diff_ms, UDP_DONE_SERVER);
		xil_printf("UDP test passed Successfully\n\r");
		first = 1;
		pbuf_free(p);
		return;
	}

	/* Update dropped datagrams statistics */
	if (server.expected_datagram_id != recv_id) {
		if (server.expected_datagram_id < recv_id) {
			drop_datagrams =
				recv_id - server.expected_datagram_id;
			server.cnt_dropped_datagrams += drop_datagrams;
			server.expected_datagram_id = recv_id + 1;
		} else if (server.expected_datagram_id > recv_id) {
			server.cnt_out_of_order_datagrams++;
		}
	} else {
		server.expected_datagram_id++;
	}

	server.cnt_datagrams++;

	/* Record total bytes for final report */
	server.total_bytes += p->tot_len;

//	if (REPORT_INTERVAL_TIME) {
//		u64_t now = get_time_ms();
//
//		server.i_report.cnt_datagrams++;
//		server.i_report.cnt_dropped_datagrams += drop_datagrams;
//
//		/* Record total bytes for interim report */
//		server.i_report.total_bytes += p->tot_len;
//		if (server.i_report.start_time) {
//			u64_t diff_ms = now - server.i_report.start_time;
//
//			if (diff_ms >= REPORT_INTERVAL_TIME) {
//				udp_conn_report(diff_ms, INTER_REPORT);
//				/* Reset Interim report counters */
//				server.i_report.start_time = 0;
//				server.i_report.total_bytes = 0;
//				server.i_report.cnt_datagrams = 0;
//				server.i_report.cnt_dropped_datagrams = 0;
//			}
//		} else {
//			/* Save start time for interim report */
//			server.i_report.start_time = now;
//		}
//	}

	pbuf_free(p);
	return;
}

void start_application(void)
{
	err_t err;

	/* Create Server PCB */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("UDP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = udp_bind(pcb, IP_ADDR_ANY, UDP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("UDP server: Unable to bind to port");
		xil_printf(" %d: err = %d\r\n", UDP_CONN_PORT, err);
		udp_remove(pcb);
		return;
	}

	/* specify callback to use for incoming connections */
	udp_recv(pcb, udp_recv_perf_traffic, NULL);

	return;
}
