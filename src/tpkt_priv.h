/**
 * Copyright (c) 2019 Parrot Drones SAS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Drones SAS Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE PARROT DRONES SAS COMPANY BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TPKT_PRIV_H_
#define _TPKT_PRIV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ULOG_TAG tpkt
#include <ulog.h>

#include <futils/futils.h>
#include <transport-packet/tpkt.h>


/* Transport packet */
struct tpkt_packet {
	/* Packet current reference count */
	unsigned int ref_count;

	/* Buffer associated with the packet (optional, can be NULL);
	 * if not NULL, this buffer must be used instead of the data
	 * structure */
	struct pomp_buffer *buf;

	/* Packet data (only if buf is NULL, undefined otherwise) */
	struct {
		union {
			/* Packet data */
			void *data;

			/* Packet data (const) */
			const void *cdata;
		};

		/* Buffer capacity (for reading only) */
		size_t cap;

		/* Packet length in bytes (write: filled by the caller;
		 * read: filled by the library) */
		size_t len;

		/* 1: buffer is read-only; 0: buffer is read/write */
		int cst;
	} data;

	/* Scatter-gather I/O structure */
#ifdef _WIN32
	WSABUF wsabuf;
#else /* _WIN32 */
	struct iovec iov;
#endif /* _WIN32 */

	/* Peer address (write: filled by the caller;
	 * read: filled by the library) */
	union {
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	} addr;

	/* Packet timestamp in microseconds on the monotonic clock
	 * (write: packet send timestamp; read: packet receive timestamp) */
	uint64_t timestamp;

	/* To be included in a list */
	struct list_node node;

	/* QoS priority */
	uint8_t priority;

	/* Importance of the packet (low numbers are more important) */
	uint32_t importance;

	/* User data */
	struct {
		tpkt_user_data_release_t release;
		void *data;
	} user_data;
};


/* Packet list */
struct tpkt_list {
	struct list_node packets;
	size_t count;
};


#endif /* !_TPKT_PRIV_H_ */
