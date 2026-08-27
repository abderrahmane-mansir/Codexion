#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <signal.h>

typedef enum e_sched
{
	POLICY_FIFO,
	POLICY_EDF
}	t_sched;

typedef struct s_request
{
	int		coder_id;
	long	seq;
	long	deadline;
}	t_request;

typedef struct s_heap
{
	t_request	*data;
	int			size;
	int			capacity;
	t_sched		sched;
}	t_heap;

typedef struct s_dongle
{
	int		taken;
	long	cooldown_until;
}	t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int		id;
	pthread_t	thread;
	int		left;
	int		right;
	long	last_compile_start;
	int		compiles_done;
	int		granted;
	t_sim	*sim;
}	t_coder;

struct s_sim
{
	int					n_coders;
	long				time_to_burnout;
	long				time_to_compile;
	long				time_to_debug;
	long				time_to_refactor;
	int					compiles_required;
	long				dongle_cooldown;
	t_sched				scheduler;

	t_coder				*coders;
	t_dongle			*dongles;
	int					n_dongles;

	pthread_mutex_t		table_lock;
	pthread_cond_t		table_cond;
	t_heap				heap;
	long				seq_counter;

	pthread_mutex_t		print_lock;
	pthread_mutex_t		state_lock;
	struct timeval		start_time;

	volatile sig_atomic_t	stop;
	pthread_t			monitor;
};

/* parsing.c */
int			parse_args(int argc, char **argv, t_sim *sim);

/* init.c */
int			init_sim(t_sim *sim);
void		destroy_sim(t_sim *sim);

/* utils.c */
long		get_time_ms(t_sim *sim);
void		log_state(t_sim *sim, int coder_id, const char *msg);
void		precise_sleep(t_sim *sim, long ms);
int			is_stopped(t_sim *sim);
void		set_stop(t_sim *sim);

/* heap.c */
void		heap_init(t_heap *h, t_sched sched);
void		heap_push(t_heap *h, t_request r);
t_request	heap_pop(t_heap *h);
int			heap_empty(t_heap *h);
void		heap_free(t_heap *h);

/* dongle.c */
int			acquire_dongles(t_coder *c);
void		release_dongles(t_coder *c);
void		try_dispatch(t_sim *sim);

/* coder.c */
void		*coder_routine(void *arg);

/* monitor.c */
void		*monitor_routine(void *arg);

#endif
