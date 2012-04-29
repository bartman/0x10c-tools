#ifndef __included_dcpu_fifo_h__
#define __included_dcpu_fifo_h__

#include <stdint.h>
#include <stdbool.h>
#include "dcpu_def.h"

struct dcpu_fifo {
	dcpu_word head, tail;
	dcpu_word size, used;
	dcpu_word elements[0];
};

#define DECLARE_DCPU_FIFO(name,count) \
	struct { \
		struct dcpu_fifo name; \
		dcpu_word name##_data[count]; \
	}

static inline void dcpu_fifo_init(struct dcpu_fifo *fifo, unsigned max)
{
	fifo->head = fifo->tail = fifo->used = 0;
	fifo->size = max;
}

static inline void dcpu_fifo_reset(struct dcpu_fifo *fifo)
{
	fifo->head = fifo->tail = fifo->used = 0;
}

static inline int dcpu_fifo_is_empty(const struct dcpu_fifo *fifo)
{
	return fifo->used == 0;
}

static inline int dcpu_fifo_item_count(const struct dcpu_fifo *fifo)
{
	return fifo->used;
}

static inline int dcpu_fifo_room_left(const struct dcpu_fifo *fifo)
{
	return fifo->size - fifo->used;
}

static inline bool dcpu_fifo_put(struct dcpu_fifo *fifo, dcpu_word item)
{
	if (fifo->used >= fifo->size)
		return false;

	fifo->elements[fifo->head] = item;
	fifo->head = (fifo->head + 1) % fifo->size;
	fifo->used ++;

	return true;
}

static inline bool dcpu_fifo_get(struct dcpu_fifo *fifo, dcpu_word *item)
{
	if (!fifo->used)
		return false;

	*item = fifo->elements[fifo->tail];
	fifo->tail = (fifo->tail + 1) % fifo->size;
	fifo->used --;

	return true;
}

static inline bool dcpu_fifo_peek(struct dcpu_fifo *fifo, dcpu_word *item)
{
	if (!fifo->used)
		return false;

	*item = fifo->elements[fifo->tail];
	return true;
}

#endif // __included_dcpu_fifo_h__
