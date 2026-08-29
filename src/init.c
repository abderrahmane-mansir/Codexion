#include "codexion.h"

static void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left = i;
		sim->coders[i].right = (i + 1) % sim->n_dongles;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].granted = 0;
		sim->coders[i].sim = sim;
		i++;
	}
}

static void	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_dongles)
	{
		sim->dongles[i].taken = 0;
		sim->dongles[i].cooldown_until = 0;
		i++;
	}
}

int	init_sim(t_sim *sim)
{
	sim->n_dongles = sim->n_coders;
	sim->coders = malloc(sizeof(t_coder) * sim->n_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->n_dongles);
	if (!sim->coders || !sim->dongles)
	{
		fprintf(stderr, "codexion: allocation failed\n");
		return (0);
	}
	init_coders(sim);
	init_dongles(sim);
	heap_init(&sim->heap, sim->scheduler);
	sim->seq_counter = 0;
	sim->stop = 0;
	pthread_mutex_init(&sim->table_lock, NULL);
	pthread_cond_init(&sim->table_cond, NULL);
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_mutex_init(&sim->state_lock, NULL);
	gettimeofday(&sim->start_time, NULL);
	return (1);
}

void	destroy_sim(t_sim *sim)
{
	pthread_mutex_destroy(&sim->table_lock);
	pthread_cond_destroy(&sim->table_cond);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->state_lock);
	heap_free(&sim->heap);
	free(sim->coders);
	free(sim->dongles);
}
