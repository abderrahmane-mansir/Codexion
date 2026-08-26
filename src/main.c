#include "../codexion.h"

static int	start_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			fprintf(stderr, "codexion: pthread_create failed\n");
			return (0);
		}
		i++;
	}
	return (1);
}

static void	join_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim->monitor, NULL);
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(sim));
	if (!parse_args(argc, argv, &sim))
		return (1);
	if (!init_sim(&sim))
		return (1);
	if (!start_coders(&sim))
	{
		destroy_sim(&sim);
		return (1);
	}
	if (pthread_create(&sim.monitor, NULL, monitor_routine, &sim) != 0)
	{
		fprintf(stderr, "codexion: pthread_create (monitor) failed\n");
		set_stop(&sim);
	}
	join_all(&sim);
	destroy_sim(&sim);
	return (0);
}
