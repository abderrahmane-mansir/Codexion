#include "codexion.h"

static int	check_burnout(t_sim *sim, long now)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		if (now - sim->coders[i].last_compile_start > sim->time_to_burnout)
			return (sim->coders[i].id);
		i++;
	}
	return (0);
}

static int	check_all_done(t_sim *sim)
{
	int	i;

	if (sim->compiles_required <= 0)
		return (0);
	i = 0;
	while (i < sim->n_coders)
	{
		if (sim->coders[i].compiles_done < sim->compiles_required)
			return (0);
		i++;
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	now;
	int		burned;

	sim = (t_sim *)arg;
	while (!is_stopped(sim))
	{
		usleep(500);
		pthread_mutex_lock(&sim->state_lock);
		now = get_time_ms(sim);
		burned = check_burnout(sim, now);
		if (!burned && check_all_done(sim))
		{
			pthread_mutex_unlock(&sim->state_lock);
			set_stop(sim);
			break ;
		}
		pthread_mutex_unlock(&sim->state_lock);
		if (burned)
		{
			log_state(sim, burned, "burned out");
			set_stop(sim);
			break ;
		}
	}
	return (NULL);
}
