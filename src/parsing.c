#include "../codexion.h"

static int	fail(const char *msg)
{
	fprintf(stderr, "codexion: %s\n", msg);
	return (0);
}

static int	only_digits(const char *s)
{
	int	i;

	if (!s || !s[0])
		return (0);
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	parse_long(const char *s, long *out)
{
	if (!only_digits(s))
		return (0);
	*out = strtol(s, NULL, 10);
	return (1);
}

static int	parse_int(const char *s, int *out)
{
	long	val;

	if (!parse_long(s, &val))
		return (0);
	*out = (int)val;
	return (1);
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	if (argc != 9)
		return (fail("usage: number_of_coders time_to_burnout time_to_compile "
			"time_to_debug time_to_refactor number_of_compiles_required "
			"dongle_cooldown scheduler(fifo|edf)"));
	if (!parse_int(argv[1], &sim->n_coders) || sim->n_coders < 1)
		return (fail("invalid number_of_coders"));
	if (!parse_long(argv[2], &sim->time_to_burnout) || sim->time_to_burnout <= 0)
		return (fail("invalid time_to_burnout"));
	if (!parse_long(argv[3], &sim->time_to_compile) || sim->time_to_compile < 0)
		return (fail("invalid time_to_compile"));
	if (!parse_long(argv[4], &sim->time_to_debug) || sim->time_to_debug < 0)
		return (fail("invalid time_to_debug"));
	if (!parse_long(argv[5], &sim->time_to_refactor) || sim->time_to_refactor < 0)
		return (fail("invalid time_to_refactor"));
	if (!parse_int(argv[6], &sim->compiles_required) || sim->compiles_required < 0)
		return (fail("invalid number_of_compiles_required"));
	if (!parse_long(argv[7], &sim->dongle_cooldown) || sim->dongle_cooldown < 0)
		return (fail("invalid dongle_cooldown"));
	if (!strcmp(argv[8], "fifo"))
		sim->scheduler = POLICY_FIFO;
	else if (!strcmp(argv[8], "edf"))
		sim->scheduler = POLICY_EDF;
	else
		return (fail("scheduler must be exactly 'fifo' or 'edf'"));
	return (1);
}
