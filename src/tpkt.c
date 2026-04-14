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

#include "tpkt_priv.h"

ULOG_DECLARE_TAG(tpkt);


static int tpkt_destroy(struct tpkt_packet *pkt)
{
	int ref;

	if (pkt == NULL)
		return 0;

	if (pkt->user_data.release != NULL)
		pkt->user_data.release(pkt, pkt->user_data.data);

	ref = tpkt_get_ref_count(pkt);
	if (ref > 0)
		ULOGW("%s: ref count is not null! (%d)", __func__, ref);

	if (pkt->buf != NULL)
		pomp_buffer_unref(pkt->buf);

	if (list_node_is_ref(&pkt->node)) {
		ULOGW("%s: packet was still in a list!", __func__);
		list_del(&pkt->node);
	}
	free(pkt);

	return 0;
}


static int tpkt_create(struct tpkt_packet **ret_obj)
{
	int res = 0;
	struct tpkt_packet *pkt;

	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	pkt = calloc(1, sizeof(*pkt));
	if (pkt == NULL) {
		res = -ENOMEM;
		ULOG_ERRNO("calloc", -res);
		return res;
	}
	list_node_unref(&pkt->node);

	/* Success */
	tpkt_ref(pkt);
	*ret_obj = pkt;
	return 0;
}


int tpkt_new(size_t cap, struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *pkt;

	res = tpkt_create(&pkt);
	if (res < 0)
		return res;

	pkt->buf = pomp_buffer_new(cap);
	if (!pkt->buf) {
		tpkt_destroy(pkt);
		return -ENOMEM;
	}

	*ret_obj = pkt;

	return 0;
}


int tpkt_new_from_buffer(struct pomp_buffer *buf, struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *pkt;

	ULOG_ERRNO_RETURN_ERR_IF(buf == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	res = tpkt_create(ret_obj);
	if (res < 0)
		return res;
	pkt = *ret_obj;

	/* Add a reference to the pomp_buffer */
	pomp_buffer_ref(buf);
	pkt->buf = buf;

	return 0;
}


int tpkt_new_from_data(void *data, size_t cap, struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *pkt;

	ULOG_ERRNO_RETURN_ERR_IF(data == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(cap == 0, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	res = tpkt_create(ret_obj);
	if (res < 0)
		return res;
	pkt = *ret_obj;

	pkt->data.data = data;
	pkt->data.cap = cap;

	return 0;
}


int tpkt_new_from_cdata(const void *data,
			size_t cap,
			struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *pkt;

	ULOG_ERRNO_RETURN_ERR_IF(data == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(cap == 0, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	res = tpkt_create(ret_obj);
	if (res < 0)
		return res;
	pkt = *ret_obj;

	pkt->data.cdata = data;
	pkt->data.cap = cap;
	pkt->data.cst = 1;

	return 0;
}


int tpkt_new_with_data(const void *data,
		       size_t cap,
		       struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *pkt;

	ULOG_ERRNO_RETURN_ERR_IF(data == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(cap == 0, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	res = tpkt_create(&pkt);
	if (res < 0)
		return res;

	pkt->buf = pomp_buffer_new_with_data(data, cap);
	if (!pkt->buf) {
		tpkt_destroy(pkt);
		return -ENOMEM;
	}

	*ret_obj = pkt;

	return 0;
}


int tpkt_clone(struct tpkt_packet *pkt, struct tpkt_packet **ret_obj)
{
	int res;
	struct tpkt_packet *new_pkt;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	res = tpkt_create(ret_obj);
	if (res < 0)
		return res;
	new_pkt = *ret_obj;

	if (pkt->buf != NULL) {
		/* Add a reference to the pomp_buffer */
		new_pkt->buf = pkt->buf;
		pomp_buffer_ref(new_pkt->buf);
	} else {
		new_pkt->data = pkt->data;
	}
	new_pkt->addr = pkt->addr;
	new_pkt->timestamp = pkt->timestamp;
	new_pkt->priority = pkt->priority;
	new_pkt->user_data = pkt->user_data;

	return 0;
}


int tpkt_ref(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

#if defined(__GNUC__)
	__atomic_add_fetch(&pkt->ref_count, 1, __ATOMIC_SEQ_CST);
#else
#	error no atomic increment function found on this platform
#endif

	return 0;
}


int tpkt_unref(struct tpkt_packet *pkt)
{
	int ref = 0;
	int res = 0;

	if (pkt == NULL)
		return 0;

	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) < 1, ENOENT);

#if defined(__GNUC__)
	ref = __atomic_sub_fetch(&pkt->ref_count, 1, __ATOMIC_SEQ_CST);
#else
#	error no atomic decrement function found on this platform
#endif

	if (ref == 0)
		res = tpkt_destroy(pkt);

	return res;
}


int tpkt_get_ref_count(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

#if defined(__GNUC__)
	int ref_count = __atomic_load_n(&pkt->ref_count, __ATOMIC_ACQUIRE);
#else
#	error no atomic load function found on this platform
#endif

	return ref_count;
}


struct pomp_buffer *tpkt_get_buffer(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_VAL_IF(pkt == NULL, EINVAL, NULL);

	return pkt->buf;
}


int tpkt_get_data(struct tpkt_packet *pkt,
		  void **data,
		  size_t *len,
		  size_t *cap)
{
	int res;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_data(pkt->buf, data, len, cap);
	} else {
		ULOG_ERRNO_RETURN_ERR_IF(pkt->data.cst, EPERM);
		if (data)
			*data = pkt->data.data;
		if (len)
			*len = pkt->data.len;
		if (cap)
			*cap = pkt->data.cap;
		res = 0;
	}

	return res;
}


int tpkt_get_cdata(struct tpkt_packet *pkt,
		   const void **data,
		   size_t *len,
		   size_t *cap)
{
	int res;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_cdata(pkt->buf, data, len, cap);
	} else {
		if (data)
			*data = pkt->data.cdata;
		if (len)
			*len = pkt->data.len;
		if (cap)
			*cap = pkt->data.cap;
		res = 0;
	}

	return res;
}


int tpkt_set_len(struct tpkt_packet *pkt, size_t len)
{
	int res;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	if (pkt->buf != NULL) {
		res = pomp_buffer_set_len(pkt->buf, len);
	} else {
		ULOG_ERRNO_RETURN_ERR_IF(len > pkt->data.cap, ENOBUFS);
		pkt->data.len = len;
		res = 0;
	}

	return res;
}


#ifdef _WIN32

int tpkt_get_wsabufs_read(struct tpkt_packet *pkt,
			  LPWSABUF *wsabufs,
			  size_t *wsabuf_count)
{
	int res;
	size_t len = 0;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_data(
			pkt->buf, (void **)&pkt->wsabuf.buf, NULL, &len);
		pkt->wsabuf.len = len;
	} else {
		ULOG_ERRNO_RETURN_ERR_IF(pkt->data.cst, EPERM);
		pkt->wsabuf.buf = pkt->data.data;
		pkt->wsabuf.len = pkt->data.cap;
		res = 0;
	}

	if (res < 0)
		return res;

	if (wsabufs)
		*wsabufs = &pkt->wsabuf;
	if (wsabuf_count)
		*wsabuf_count = 1;

	return 0;
}


#else /* _WIN32 */

int tpkt_get_iov_read(struct tpkt_packet *pkt,
		      struct iovec **iov,
		      size_t *iov_len)
{
	int res;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_data(
			pkt->buf, &pkt->iov.iov_base, NULL, &pkt->iov.iov_len);
	} else {
		ULOG_ERRNO_RETURN_ERR_IF(pkt->data.cst, EPERM);
		pkt->iov.iov_base = pkt->data.data;
		pkt->iov.iov_len = pkt->data.cap;
		res = 0;
	}

	if (res < 0)
		return res;

	if (iov)
		*iov = &pkt->iov;
	if (iov_len)
		*iov_len = 1;

	return 0;
}

#endif /* _WIN32 */


#ifdef _WIN32

int tpkt_get_wsabufs_write(struct tpkt_packet *pkt,
			   LPWSABUF *wsabufs,
			   size_t *wsabuf_count)
{
	int res;
	size_t len = 0;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_cdata(
			pkt->buf, (const void **)&pkt->wsabuf.buf, &len, NULL);
		pkt->wsabuf.len = len;
	} else {
		pkt->wsabuf.buf = (void *)pkt->data.cdata;
		pkt->wsabuf.len = pkt->data.len;
		res = 0;
	}

	if (res < 0)
		return res;

	if (wsabufs)
		*wsabufs = &pkt->wsabuf;
	if (wsabuf_count)
		*wsabuf_count = 1;

	return 0;
}


#else /* _WIN32 */

int tpkt_get_iov_write(struct tpkt_packet *pkt,
		       struct iovec **iov,
		       size_t *iov_len)
{
	int res;

	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

	if (pkt->buf != NULL) {
		res = pomp_buffer_get_cdata(pkt->buf,
					    (const void **)&pkt->iov.iov_base,
					    &pkt->iov.iov_len,
					    NULL);
	} else {
		pkt->iov.iov_base = (void *)pkt->data.cdata;
		pkt->iov.iov_len = pkt->data.len;
		res = 0;
	}

	if (res < 0)
		return res;

	if (iov)
		*iov = &pkt->iov;
	if (iov_len)
		*iov_len = 1;

	return 0;
}

#endif /* _WIN32 */


struct sockaddr_in *tpkt_get_addr(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_VAL_IF(pkt == NULL, EINVAL, NULL);

	return &pkt->addr.in;
}


struct sockaddr_in6 *tpkt_get_addr6(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_VAL_IF(pkt == NULL, EINVAL, NULL);

	return &pkt->addr.in6;
}


uint64_t tpkt_get_timestamp(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_VAL_IF(pkt == NULL, EINVAL, 0);

	return pkt->timestamp;
}


int tpkt_set_timestamp(struct tpkt_packet *pkt, uint64_t ts)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	pkt->timestamp = ts;
	return 0;
}


int tpkt_get_priority(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);

	return pkt->priority;
}


int tpkt_set_priority(struct tpkt_packet *pkt, int priority)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(priority < 0, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(priority > QOS_PRIORITY_MAX, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	pkt->priority = priority;
	return 0;
}


int tpkt_get_importance(struct tpkt_packet *pkt, uint32_t *importance)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(importance == NULL, EINVAL);

	*importance = pkt->importance;
	return 0;
}


int tpkt_set_importance(struct tpkt_packet *pkt, uint32_t importance)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	pkt->importance = importance;
	return 0;
}


void *tpkt_get_user_data(struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_VAL_IF(pkt == NULL, EINVAL, NULL);

	return pkt->user_data.data;
}


int tpkt_set_user_data(struct tpkt_packet *pkt,
		       tpkt_user_data_release_t release,
		       void *user_data)
{
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(tpkt_get_ref_count(pkt) > 1, EPERM);

	pkt->user_data.release = release;
	pkt->user_data.data = user_data;
	return 0;
}
