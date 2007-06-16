/*
	link_ops.h
	link operations

	$Id: link_ops.h,v 1.2 2007-06-16 07:30:29 sezero Exp $
*/

#ifndef __LINKOPS_H
#define __LINKOPS_H

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (intptr_t)&(((t *)0)->m)))


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);


#endif	/* __LINKOPS_H */

