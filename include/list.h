
/* list.h - list operator
 *
 * Copyright (C) 2009-2010 Kingstone, ltd
 *
 * Written by microcai in 2009-2010
 *
 * This software is lisenced under the Kingstone mid-ware Lisence.
 *
 * For more infomation see COPYING file shipped with this software.
 *
 * If you have any question with this software, please contract microcai, the
 * original writer of this software.
 *
 * If you have any question with law suite, please contract 黄小克, the owner of
 * this company.
 *
 */

#ifndef __LIST__H
#define __LIST__H

#include <stdio.h>

__BEGIN_DECLS

struct list_node{
	struct list_node * prev;
	struct list_node * next;
};

typedef struct list_slot{
	struct list_node * tail;
	struct list_node * head;

}list_slot;

/*
 * Use it inside a structure, One structure can belone to many lists.
 */
#define DEFINE_LIST(listname)	struct list_node listname


/*
 * Macro that helps your life a bit easier
 */
#define LIST_NODE_OFFSET(v,listname)	((size_t)((char*)&(((typeof(v))0)->listname)))
#define LIST_NODE_OFFSET_(type,listname)	((size_t)((char*)&(((struct type*)0)->listname)))

#define LIST_HEAD(node,structname,listname)	(struct structname *)( (char*)node - LIST_NODE_OFFSET_(structname,listname)  )

/*
 * Use this to declare a list slot.
 * Thus will generate a struct has a head and a tail point to your struct
 */
#define LIST_SLOT_DECLARE(name)	extern list_slot name;
#define LIST_SLOT_DEFINE(name) list_slot name = { (struct list_node *) &name , (struct list_node *) &name }

/*
 * A macro to test whether a list is empty or not.
 */
#define LIST_ISEMPTY(listname)	(listname.head == listname.tail)


/*
 * Use this to insert a node at head, use LIST_NODE_OFFSET to generate the offset
 */
static inline	void LIST_ADDTOHEAD( list_slot *listslot, struct list_node * node )
{
	node->next = listslot->head;
	node->prev = (struct list_node*)listslot;
	listslot->head->prev = node;
	listslot->head = node;
}

/*
 * Use this to insert a node at the end, see LIST_ADDTOHEAD
 */
static inline void LIST_ADDTOTAIL(list_slot * slot, struct list_node * node)
{
	node->prev = slot->tail;
	node->next = (struct list_node*)slot;

	slot->tail->next = node;
	slot->tail = node;
}

static inline void LIST_INSERT_AFTER(struct list_node * at,struct list_node * node)
{
	node->next = at->next ;

	node->prev = at;

	at->next->prev = node;
	at->next = node;
}

static inline void LIST_INSERT_BEFOR(struct list_node * at,struct list_node * node )
{
	node->next = at;
	node->prev = at->prev;

	at->prev->next = node;
	at->prev = node;
}

/*
 * Detach the node
 */
static inline LIST_DELETE_AT(struct list_node* node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

__END_DECLS
#endif // __LIST__H
