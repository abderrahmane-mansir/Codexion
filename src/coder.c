#include "../codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*c;
	t_sim	*sim;

	c = (t_coder *)arg;
	sim = c->sim;
	while (!is_stopped(sim))
	{
		if (!acquire_dongles(c))
			break ;
		if (is_stopped(sim))
		{
			release_dongles(c);
			break ;
		}
		pthread_mutex_lock(&sim->state_lock);
		c->last_compile_start = get_time_ms(sim);
		pthread_mutex_unlock(&sim->state_lock);
		log_state(sim, c->id, "is compiling");
		precise_sleep(sim, sim->time_to_compile);
		release_dongles(c);
		pthread_mutex_lock(&sim->state_lock);
		c->compiles_done++;
		pthread_mutex_unlock(&sim->state_lock);
		if (is_stopped(sim))
			break ;
		log_state(sim, c->id, "is debugging");
		precise_sleep(sim, sim->time_to_debug);
		if (is_stopped(sim))
			break ;
		log_state(sim, c->id, "is refactoring");
		precise_sleep(sim, sim->time_to_refactor);
	}
	return (NULL);
}
