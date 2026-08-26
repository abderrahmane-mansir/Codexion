#include "../codexion.h"

static int	req_less(t_heap *h, t_request *a, t_request *b)
{
	if (h->sched == POLICY_FIFO)
		return (a->seq < b->seq);
	if (a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	return (a->seq < b->seq);
}

void	heap_init(t_heap *h, t_sched sched)
{
	h->capacity = 16;
	h->size = 0;
	h->sched = sched;
	h->data = malloc(sizeof(t_request) * h->capacity);
}

static void	heap_grow(t_heap *h)
{
	t_request	*bigger;
	int			i;

	h->capacity *= 2;
	bigger = malloc(sizeof(t_request) * h->capacity);
	i = 0;
	while (i < h->size)
	{
		bigger[i] = h->data[i];
		i++;
	}
	free(h->data);
	h->data = bigger;
}

static void	swap_req(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	sift_up(t_heap *h, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!req_less(h, &h->data[i], &h->data[parent]))
			break ;
		swap_req(&h->data[i], &h->data[parent]);
		i = parent;
	}
}

static void	sift_down(t_heap *h, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = i * 2 + 1;
		right = i * 2 + 2;
		smallest = i;
		if (left < h->size && req_less(h, &h->data[left], &h->data[smallest]))
			smallest = left;
		if (right < h->size && req_less(h, &h->data[right], &h->data[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		swap_req(&h->data[i], &h->data[smallest]);
		i = smallest;
	}
}

void	heap_push(t_heap *h, t_request r)
{
	if (h->size == h->capacity)
		heap_grow(h);
	h->data[h->size] = r;
	sift_up(h, h->size);
	h->size++;
}

t_request	heap_pop(t_heap *h)
{
	t_request	top;

	top = h->data[0];
	h->size--;
	h->data[0] = h->data[h->size];
	if (h->size > 0)
		sift_down(h, 0);
	return (top);
}

int	heap_empty(t_heap *h)
{
	return (h->size == 0);
}

void	heap_free(t_heap *h)
{
	free(h->data);
	h->data = NULL;
	h->size = 0;
	h->capacity = 0;
}
