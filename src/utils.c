#include "../codexion.h"

long	get_time_ms(t_sim *sim)
{
	struct timeval	now;
	long			sec;
	long			usec;

	gettimeofday(&now, NULL);
	sec = now.tv_sec - sim->start_time.tv_sec;
	usec = now.tv_usec - sim->start_time.tv_usec;
	return (sec * 1000 + usec / 1000);
}

void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	pthread_mutex_lock(&sim->print_lock);
	printf("%ld %d %s\n", get_time_ms(sim), coder_id, msg);
	fflush(stdout);
	pthread_mutex_unlock(&sim->print_lock);
}

int	is_stopped(t_sim *sim)
{
	return (sim->stop);
}

void	set_stop(t_sim *sim)
{
	sim->stop = 1;
	pthread_mutex_lock(&sim->table_lock);
	pthread_cond_broadcast(&sim->table_cond);
	pthread_mutex_unlock(&sim->table_lock);
}

/*
** Sleeps in 1ms chunks so a stop signal (burnout / all-done)
** is noticed quickly instead of blocking for the full duration.
*/
void	precise_sleep(t_sim *sim, long ms)
{
	long	slept;
	long	chunk;

	slept = 0;
	while (slept < ms)
	{
		if (is_stopped(sim))
			return ;
		chunk = ms - slept;
		if (chunk > 1)
			chunk = 1;
		usleep(chunk * 1000);
		slept += chunk;
	}
}
