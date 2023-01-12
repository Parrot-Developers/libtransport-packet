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

#ifndef _TPKT_H_
#define _TPKT_H_

#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#ifdef _WIN32
#	include <winsock2.h>
#else /* !_WIN32 */
#	include <arpa/inet.h>
#	include <sys/socket.h>
#endif /* !_WIN32 */

#include <libpomp.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* To be used for all public API */
#ifdef TPKT_API_EXPORTS
#	ifdef _WIN32
#		define TPKT_API __declspec(dllexport)
#	else /* !_WIN32 */
#		define TPKT_API __attribute__((visibility("default")))
#	endif /* !_WIN32 */
#else /* !TPKT_API_EXPORTS */
#	define TPKT_API
#endif /* !TPKT_API_EXPORTS */

/* QoS priority max value */
#define QOS_PRIORITY_MAX 7
/* QoS id max value */
#define QOS_ID_MAX 4


/* Forward declarations */
struct tpkt_packet;
struct tpkt_list;


/**
 * Packet API
 */

/**
 * Create a packet.
 * A packet is created with a reference count of 1. When no longer needed,
 * the packet must be unreferenced using the tpkt_unref() function.
 * When a packet is no longer referenced it is destroyed.
 * The packet will be associated to a pomp_buffer.
 * The created packet object is returned through the ret_obj parameter.
 * @param cap: internal buffer capacity in bytes
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_new(size_t cap, struct tpkt_packet **ret_obj);


/**
 * Create a packet from a buffer.
 * A packet is created with a reference count of 1. When no longer needed,
 * the packet must be unreferenced using the tpkt_unref() function.
 * When a packet is no longer referenced it is destroyed.
 * The packet creation adds a reference to the associated pomp_buffer.
 * The created packet object is returned through the ret_obj parameter.
 * @param buf: pointer on a pomp_buffer object
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_new_from_buffer(struct pomp_buffer *buf,
				  struct tpkt_packet **ret_obj);


/**
 * Create a packet from existing data (read/write).
 * A packet is created with a reference count of 1. When no longer needed,
 * the packet must be unreferenced using the tpkt_unref() function.
 * When a packet is no longer referenced it is destroyed.
 * The created packet object is returned through the ret_obj parameter.
 * @param data: pointer on the data
 * @param cap: data buffer capacity in bytes
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int
tpkt_new_from_data(void *data, size_t cap, struct tpkt_packet **ret_obj);


/**
 * Create a packet from existing data (read-only).
 * A packet is created with a reference count of 1. When no longer needed,
 * the packet must be unreferenced using the tpkt_unref() function.
 * When a packet is no longer referenced it is destroyed.
 * The created packet object is returned through the ret_obj parameter.
 * @param data: pointer on the data
 * @param cap: data buffer capacity in bytes
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int
tpkt_new_from_cdata(const void *data, size_t cap, struct tpkt_packet **ret_obj);


/**
 * Create a packet from existing data, copying them in a pomp buffer.
 * A packet is created with a reference count of 1. When no longer needed,
 * the packet must be unreferenced using the tpkt_unref() function.
 * When a packet is no longer referenced it is destroyed.
 * The created packet object is returned through the ret_obj parameter.
 * @param data: pointer on the data
 * @param cap: data buffer capacity in bytes
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int
tpkt_new_with_data(const void *data, size_t cap, struct tpkt_packet **ret_obj);


/**
 * Clone a packet.
 * If the paket was created from a pomp_buffer object, its refererence counter
 * is incremented. If the packet was created from plain data, the pointer is
 * simply copied and it's the application's responsibility to handle the life
 * cycle of the allocated memory.
 * @param pkt: object handle of the packet to clone
 * @param ret_obj: pointer to the created packet object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_clone(struct tpkt_packet *pkt, struct tpkt_packet **ret_obj);


/**
 * Reference a packet.
 * This function increments the reference counter of a packet.
 * @param pkt: packet object handle
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_ref(struct tpkt_packet *pkt);


/**
 * Unreference a packet.
 * This function decrements the reference counter of a packet.
 * If the packet's reference counter reaches zero, it is destroyed.
 * @param pkt: packet object handle
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_unref(struct tpkt_packet *pkt);


/**
 * Get the packet's reference count.
 * This function returns the current value of the reference counter.
 * @param pkt: packet object handle
 * @return the reference count on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_ref_count(struct tpkt_packet *pkt);


/**
 * Get the packet associated buffer.
 * If the packet is not associated with a pomp_buffer, NULL is returned.
 * @param pkt: packet object handle
 * @return pointer on the buffer on success, NULL in case of error
 */
TPKT_API struct pomp_buffer *tpkt_get_buffer(struct tpkt_packet *pkt);


/**
 * Get the packet data for read/write.
 * If the packet is shared (more than 1 reference), getting the data for
 * read/write is not permitted and the function returns -EPERM.
 * @param pkt: packet object handle
 * @param data: pointer on the data (output; optional, can be NULL)
 * @param len: pointer on the data length in bytes
 *             (output; optional, can be NULL)
 * @param cap: pointer on the buffer capacity in bytes
 *             (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int
tpkt_get_data(struct tpkt_packet *pkt, void **data, size_t *len, size_t *cap);


/**
 * Get the packet data for read-only.
 * @param pkt: packet object handle
 * @param data: pointer on the data (output; optional, can be NULL)
 * @param len: pointer on the data length in bytes
 *             (output; optional, can be NULL)
 * @param cap: pointer on the buffer capacity in bytes
 *             (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_cdata(struct tpkt_packet *pkt,
			    const void **data,
			    size_t *len,
			    size_t *cap);


/**
 * Set the packet data size in bytes.
 * If the packet is shared (more than 1 reference), setting the size is
 * not permitted and the function returns -EPERM.
 * @param pkt: packet object handle
 * @param len: packet data size in bytes
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_set_len(struct tpkt_packet *pkt, size_t len);


#ifdef _WIN32
/**
 * Get the scatter-gather I/O array for reading.
 * The scatter-gather I/O array is used with the WSASendTo/WSARecvFrom API.
 * For reading, each WSABUF length is the capacity of the corresponding buffer.
 * @param pkt: packet object handle
 * @param wsabufs: pointer on the WSABUF array (output; optional, can be NULL)
 * @param wsabuf_count: pointer on the WSABUF count
 *                      (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_wsabufs_read(struct tpkt_packet *pkt,
				   /* codecheck_ignore[SPACING] */
				   LPWSABUF *wsabufs,
				   size_t *wsabuf_count);
#else /* _WIN32 */
/**
 * Get the scatter-gather I/O array for reading.
 * The scatter-gather I/O array is used with the sendmsg/recvmsg API.
 * For reading, each iovec length is the capacity of the corresponding buffer.
 * @param pkt: packet object handle
 * @param iov: pointer on the iovec array (output; optional, can be NULL)
 * @param iov_len: pointer on the iovec count (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int
tpkt_get_iov_read(struct tpkt_packet *pkt, struct iovec **iov, size_t *iov_len);
#endif /* _WIN32 */


#ifdef _WIN32
/**
 * Get the scatter-gather I/O array for writing.
 * The scatter-gather I/O array is used with the WSASendTo/WSARecvFrom API.
 * For writing, each WSABUF length is the length of the corresponding data.
 * @param pkt: packet object handle
 * @param wsabufs: pointer on the WSABUF array (output; optional, can be NULL)
 * @param wsabuf_count: pointer on the WSABUF count
 *                      (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_wsabufs_write(struct tpkt_packet *pkt,
				    /* codecheck_ignore[SPACING] */
				    LPWSABUF *wsabufs,
				    size_t *wsabuf_count);
#else /* _WIN32 */
/**
 * Get the scatter-gather I/O array for writing.
 * The scatter-gather I/O array is used with the sendmsg/recvmsg API.
 * For writing, each iovec length is the length of the corresponding data.
 * @param pkt: packet object handle
 * @param iov: pointer on the iovec array (output; optional, can be NULL)
 * @param iov_len: pointer on the iovec count (output; optional, can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_iov_write(struct tpkt_packet *pkt,
				struct iovec **iov,
				size_t *iov_len);
#endif /* _WIN32 */


/**
 * Get a pointer on the associated address.
 * When sending a packet, the value should be set before writing the packet.
 * When receiving a packet, the value should be read after reading the packet.
 * The returned pointer has the lifetime of the packet object; it must not
 * be freed.
 * @param pkt: packet object handle
 * @return a pointer on the address structure on success, NULL in case of error
 */
TPKT_API struct sockaddr_in *tpkt_get_addr(struct tpkt_packet *pkt);


/**
 * Get the packet timestamp.
 * The timestamp is in microseconds on the monotonic clock. For write
 * operations this is the packet send timestamp; for read operations this
 * is the packet receive timestamp.
 * @param pkt: packet object handle
 * @return timestamp on success, 0 in case of error
 */
TPKT_API uint64_t tpkt_get_timestamp(struct tpkt_packet *pkt);


/**
 * Set the packet timestamp.
 * The timestamp is in microseconds on the monotonic clock. For write
 * operations this is the packet send timestamp; for read operations this
 * is the packet receive timestamp.
 * If the packet is shared (more than 1 reference), setting the timestamp is
 * not permitted and the function returns -EPERM.
 * @param pkt: packet object handle
 * @param ts: timestamp in microseconds
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_set_timestamp(struct tpkt_packet *pkt, uint64_t ts);


/**
 * Get the packet user data.
 * @param pkt: packet object handle
 * @return user_data: user_data to be get
 */
TPKT_API void *tpkt_get_user_data(struct tpkt_packet *pkt);


typedef void (*tpkt_user_data_release_t)(struct tpkt_packet *pkt,
					 void *userdata);


/**
 * Set the packet user data.
 * @param pkt: packet object handle
 * @param release: optional user_data release callback
 * @param user_data: user_data to be set
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_set_user_data(struct tpkt_packet *pkt,
				tpkt_user_data_release_t release,
				void *user_data);


/**
 * Get the packet priority.
 * The priority value is a positive number between 0 (default value)
 * and QOS_PRIORITY_MAX (whose value is 7) (highest priority).
 *   value      priority
 *   0          lowest
 *   7          highest
 * @param pkt: packet object handle
 * @return priority on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_priority(struct tpkt_packet *pkt);


/**
 * Set the packet priority.
 * The priority value is a positive number between 0 (default value)
 * and QOS_PRIORITY_MAX (whose value is 7) (highest priority).
 *   value      priority
 *   0          lowest
 *   7          highest
 * @param pkt: packet object handle
 * @param priority: priority to be set
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_set_priority(struct tpkt_packet *pkt, int priority);


/**
 * Get the packet importance.
 * The importance value is a positive number between 0 (max value)
 * and UINT32_MAX (lowest importance).
 *   value      importance
 *   0          highest
 *   UINT32_MAX lowest
 * @param pkt: packet object handle
 * @param importance: pointer on the data (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_get_importance(struct tpkt_packet *pkt, uint32_t *importance);


/**
 * Set the packet importance.
 * The importance value is a positive number between 0 (max value)
 * and UINT32_MAX (lowest importance).
 *   value      importance
 *   0          highest
 *   UINT32_MAX lowest
 * @param pkt: packet object handle
 * @param importance: importance to be set
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_set_importance(struct tpkt_packet *pkt, uint32_t importance);


/**
 * List API
 */

/**
 * Create a packet list.
 * The created packet list object is returned through the ret_obj parameter.
 * When no longer needed, the list must be freed using the
 * tpkt_list_destroy() function.
 * @param ret_obj: pointer to the created packet list object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_new(struct tpkt_list **ret_obj);


/**
 * Free a packet list.
 * This function frees all resources associated with a packet list.
 * If the list is not empty, all packets are unreferenced.
 * @param list: packet list object handle
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_destroy(struct tpkt_list *list);


/**
 * Get the list packet count.
 * @param list: packet list object handle
 * @return the packet count on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_get_count(struct tpkt_list *list);


/**
 * Get the first packet in the list.
 * The packet is not removed from the list.
 * If the list is empty, NULL is returned.
 * @param list: packet list object handle
 * @return the first packet on success, NULL in case of error
 */
TPKT_API struct tpkt_packet *tpkt_list_first(struct tpkt_list *list);


/**
 * Get the last packet in the list.
 * The packet is not removed from the list.
 * If the list is empty, NULL is returned.
 * @param list: packet list object handle
 * @return the last packet on success, NULL in case of error
 */
TPKT_API struct tpkt_packet *tpkt_list_last(struct tpkt_list *list);


/**
 * Get the previous packet in the list.
 * The packet is not removed from the list.
 * If next is NULL, the last packet is returned;
 * otherwise, the packet preceeding next is returned.
 * If the list is empty, NULL is returned.
 * @param list: packet list object handle
 * @param next: handle of the next packet (optional, can be NULL)
 * @return the previous packet on success, NULL in case of error
 */
TPKT_API struct tpkt_packet *tpkt_list_prev(struct tpkt_list *list,
					    struct tpkt_packet *next);


/**
 * Get the next packet in the list.
 * The packet is not removed from the list.
 * If prev is NULL, the first packet is returned;
 * otherwise, the packet succeeding prev is returned.
 * If the list is empty, NULL is returned.
 * @param list: packet list object handle
 * @param prev: handle of the previous packet (optional, can be NULL)
 * @return the next packet on success, NULL in case of error
 */
TPKT_API struct tpkt_packet *tpkt_list_next(struct tpkt_list *list,
					    struct tpkt_packet *prev);


/**
 * Add a packet at the beginning of the list.
 * When added to the list, the packet reference counter is incremented.
 * A packet cannot be in more than one list; if the packet is already
 * in a list, -EBUSY is returned.
 * @param list: packet list object handle
 * @param pkt: handle of the packet to add
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_add_first(struct tpkt_list *list,
				 struct tpkt_packet *pkt);


/**
 * Add a packet at the end of the list.
 * When added to the list, the packet reference counter is incremented.
 * A packet cannot be in more than one list; if the packet is already
 * in a list, -EBUSY is returned.
 * @param list: packet list object handle
 * @param pkt: handle of the packet to add
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_add_last(struct tpkt_list *list,
				struct tpkt_packet *pkt);


/**
 * Add a packet before another packet in the list.
 * When added to the list, the packet reference counter is incremented.
 * A packet cannot be in more than one list; if the packet is already
 * in a list, -EBUSY is returned.
 * If next is NULL, the packet is added at the end of the list
 * @param list: packet list object handle
 * @param next: handle of the next packet (optional, can be NULL)
 * @param pkt: handle of the packet to add
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_add_before(struct tpkt_list *list,
				  struct tpkt_packet *next,
				  struct tpkt_packet *pkt);


/**
 * Add a packet after another packet in the list.
 * When added to the list, the packet reference counter is incremented.
 * A packet cannot be in more than one list; if the packet is already
 * in a list, -EBUSY is returned.
 * If prev is NULL, the packet is added at the beginning of the list
 * @param list: packet list object handle
 * @param prev: handle of the previous packet (optional, can be NULL)
 * @param pkt: handle of the packet to add
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_add_after(struct tpkt_list *list,
				 struct tpkt_packet *prev,
				 struct tpkt_packet *pkt);


/**
 * Move a packet at the beginning of the list.
 * @param list: packet list object handle
 * @param pkt: handle of the packet to move
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_move_first(struct tpkt_list *list,
				  struct tpkt_packet *pkt);


/**
 * Move a packet at the end of the list.
 * @param list: packet list object handle
 * @param pkt: handle of the packet to move
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_move_last(struct tpkt_list *list,
				 struct tpkt_packet *pkt);


/**
 * Move a packet before another packet in the list.
 * If next is NULL, the packet is moved at the end of the list
 * @param list: packet list object handle
 * @param next: handle of the next packet (optional, can be NULL)
 * @param pkt: handle of the packet to move
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_move_before(struct tpkt_list *list,
				   struct tpkt_packet *next,
				   struct tpkt_packet *pkt);


/**
 * Move a packet after another packet in the list.
 * If prev is NULL, the packet is moved at the beginning of the list
 * @param list: packet list object handle
 * @param prev: handle of the previous packet (optional, can be NULL)
 * @param pkt: handle of the packet to move
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_move_after(struct tpkt_list *list,
				  struct tpkt_packet *prev,
				  struct tpkt_packet *pkt);


/**
 * Remove a packet from the list.
 * When a packet is removed from the list, it is not unreferenced; it is up
 * to the caller to unreference the packet once it is no longer needed.
 * @param list: packet list object handle
 * @param pkt: handle of the packet to remove
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_remove(struct tpkt_list *list, struct tpkt_packet *pkt);


/**
 * Flush a packet list.
 * This function removes all packets from the list and unreferences them.
 * @param list: packet list object handle
 * @return 0 on success, negative errno value in case of error
 */
TPKT_API int tpkt_list_flush(struct tpkt_list *list);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_TPKT_H_ */
