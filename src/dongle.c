#include "codexion.h"

static int	dongle_ready(t_dongle *d, long now)
{
	if (d->taken)
		return (0);
	if (d->cooldown_until != 0 && now < d->cooldown_until)
		return (0);
	return (1);
}

void	try_dispatch(t_sim *sim)
{
	t_heap		pending;
	t_request	r;
	t_coder		*c;
	long		now;
	int			granted_any;

	heap_init(&pending, sim->scheduler);
	granted_any = 0;
	while (!heap_empty(&sim->heap))
	{
		r = heap_pop(&sim->heap);
		c = &sim->coders[r.coder_id - 1];
		now = get_time_ms(sim);
		if (c->left != c->right
			&& dongle_ready(&sim->dongles[c->left], now)
			&& dongle_ready(&sim->dongles[c->right], now))
		{
			sim->dongles[c->left].taken = 1;
			sim->dongles[c->right].taken = 1;
			c->granted = 1;
			granted_any = 1;
		}
		else
			heap_push(&pending, r);
	}
	while (!heap_empty(&pending))
		heap_push(&sim->heap, heap_pop(&pending));
	heap_free(&pending);
	if (granted_any)
		pthread_cond_broadcast(&sim->table_cond);
}

static void	build_deadline(t_request *req, long deadline_base, t_sim *sim)
{
	req->seq = sim->seq_counter++;
	req->deadline = deadline_base + sim->time_to_burnout;
}

/* Returns 1 if both dongles were obtained, 0 if the simulation stopped
** while waiting (coder must abandon and exit). */
int	acquire_dongles(t_coder *c)
{
	t_sim			*sim;
	t_request		req;
	struct timeval	now;
	struct timespec	ts;
	long			last_start;

	sim = c->sim;
	pthread_mutex_lock(&sim->state_lock);
	last_start = c->last_compile_start;
	req.coder_id = c->id;
	pthread_mutex_unlock(&sim->state_lock);
	pthread_mutex_lock(&sim->table_lock);
	c->granted = 0;
	build_deadline(&req, last_start, sim);
	heap_push(&sim->heap, req);
	try_dispatch(sim);
	while (!c->granted && !is_stopped(sim))
	{
		gettimeofday(&now, NULL);
		ts.tv_sec = now.tv_sec;
		ts.tv_nsec = (now.tv_usec + 500) * 1000;
		if (ts.tv_nsec >= 1000000000)
		{
			ts.tv_nsec -= 1000000000;
			ts.tv_sec += 1;
		}
		pthread_cond_timedwait(&sim->table_cond, &sim->table_lock, &ts);
		try_dispatch(sim);
	}
	pthread_mutex_unlock(&sim->table_lock);
	if (!c->granted)
		return (0);
	log_state(sim, c->id, "has taken a dongle");
	log_state(sim, c->id, "has taken a dongle");
	return (1);
}

void	release_dongles(t_coder *c)
{
	t_sim	*sim;
	long	now;

	sim = c->sim;
	pthread_mutex_lock(&sim->table_lock);
	now = get_time_ms(sim);
	sim->dongles[c->left].taken = 0;
	sim->dongles[c->left].cooldown_until = now + sim->dongle_cooldown;
	sim->dongles[c->right].taken = 0;
	sim->dongles[c->right].cooldown_until = now + sim->dongle_cooldown;
	try_dispatch(sim);
	pthread_mutex_unlock(&sim->table_lock);
}
