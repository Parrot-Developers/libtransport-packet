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


int tpkt_list_new(struct tpkt_list **ret_obj)
{
	int res = 0;
	struct tpkt_list *list;

	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	list = calloc(1, sizeof(*list));
	if (list == NULL) {
		res = -ENOMEM;
		ULOG_ERRNO("calloc", -res);
		return res;
	}
	list_init(&list->packets);

	*ret_obj = list;
	return 0;
}


int tpkt_list_destroy(struct tpkt_list *list)
{
	int res;

	if (list == NULL)
		return 0;

	res = tpkt_list_flush(list);
	if (res < 0)
		return res;

	free(list);

	return 0;
}


int tpkt_list_get_count(struct tpkt_list *list)
{
	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);

	return (int)list->count;
}


struct tpkt_packet *tpkt_list_first(struct tpkt_list *list)
{
	return tpkt_list_next(list, NULL);
}


struct tpkt_packet *tpkt_list_last(struct tpkt_list *list)
{
	return tpkt_list_prev(list, NULL);
}


struct tpkt_packet *tpkt_list_prev(struct tpkt_list *list,
				   struct tpkt_packet *next)
{
	struct list_node *node;
	struct tpkt_packet *prev;

	ULOG_ERRNO_RETURN_VAL_IF(list == NULL, EINVAL, NULL);

	node = list_prev(&list->packets, next ? &next->node : &list->packets);
	if (!node)
		return NULL;

	prev = list_entry(node, struct tpkt_packet, node);
	return prev;
}


struct tpkt_packet *tpkt_list_next(struct tpkt_list *list,
				   struct tpkt_packet *prev)
{
	struct list_node *node;
	struct tpkt_packet *next;

	ULOG_ERRNO_RETURN_VAL_IF(list == NULL, EINVAL, NULL);

	node = list_next(&list->packets, prev ? &prev->node : &list->packets);
	if (!node)
		return NULL;

	next = list_entry(node, struct tpkt_packet, node);
	return next;
}


int tpkt_list_add_first(struct tpkt_list *list, struct tpkt_packet *pkt)
{
	return tpkt_list_add_after(list, NULL, pkt);
}


int tpkt_list_add_last(struct tpkt_list *list, struct tpkt_packet *pkt)
{
	return tpkt_list_add_before(list, NULL, pkt);
}


int tpkt_list_add_before(struct tpkt_list *list,
			 struct tpkt_packet *next,
			 struct tpkt_packet *pkt)
{
	struct list_node *node;

	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(next && list_node_is_unref(&next->node),
				 ENOENT);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&pkt->node), EBUSY);

	tpkt_ref(pkt);

	node = (next) ? &next->node : &list->packets;

	list_add_before(node, &pkt->node);

	list->count++;

	return 0;
}


int tpkt_list_add_after(struct tpkt_list *list,
			struct tpkt_packet *prev,
			struct tpkt_packet *pkt)
{
	struct list_node *node;

	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(prev && list_node_is_unref(&prev->node),
				 ENOENT);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&pkt->node), EBUSY);

	tpkt_ref(pkt);

	node = (prev) ? &prev->node : &list->packets;

	list_add_after(node, &pkt->node);

	list->count++;

	return 0;
}


int tpkt_list_move_first(struct tpkt_list *list, struct tpkt_packet *pkt)
{
	return tpkt_list_move_after(list, NULL, pkt);
}


int tpkt_list_move_last(struct tpkt_list *list, struct tpkt_packet *pkt)
{
	return tpkt_list_move_before(list, NULL, pkt);
}


int tpkt_list_move_before(struct tpkt_list *list,
			  struct tpkt_packet *next,
			  struct tpkt_packet *pkt)
{
	struct list_node *node;

	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(next && list_node_is_unref(&next->node),
				 ENOENT);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_unref(&pkt->node), ENOENT);

	node = (next) ? &next->node : &list->packets;

	list_move_before(node, &pkt->node);

	return 0;
}


int tpkt_list_move_after(struct tpkt_list *list,
			 struct tpkt_packet *prev,
			 struct tpkt_packet *pkt)
{
	struct list_node *node;

	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(prev && list_node_is_unref(&prev->node),
				 ENOENT);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_unref(&pkt->node), ENOENT);

	node = (prev) ? &prev->node : &list->packets;

	list_move_after(node, &pkt->node);

	return 0;
}


int tpkt_list_remove(struct tpkt_list *list, struct tpkt_packet *pkt)
{
	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(pkt == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_unref(&pkt->node), ENOENT);

	list_del(&pkt->node);
	list->count--;

	return 0;
}


int tpkt_list_flush(struct tpkt_list *list)
{
	struct tpkt_packet *pkt;
	struct tpkt_packet *pkt_tmp;

	ULOG_ERRNO_RETURN_ERR_IF(list == NULL, EINVAL);

	list_walk_entry_forward_safe(&list->packets, pkt, pkt_tmp, node)
	{
		list_del(&pkt->node);
		tpkt_unref(pkt);
	}

	list->count = 0;

	return 0;
}
