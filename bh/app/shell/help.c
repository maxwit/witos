#include <g-bios.h>

extern const struct gapp g_app_begin[], g_app_end[];


int main(int argc, char *argv[])
{
	int ret;
	const struct gapp *app;

	switch (argc)
	{
	case 1:
		printf("\ng-bios commands:\n");

		for (app = g_app_begin; app < g_app_end; app++)
			printf("  %-8s\n", app->name);

		ret = 0;
		break;

	case 2:
		for (app = g_app_begin; app < g_app_end; app++)
		{
			if (!strcmp(app->name, argv[1]))
			{
				ret = 0;
				break;
			}
		}

		if (g_app_end == app)
		{
			printf("help: %s: No such command\n", argv[1]);
			ret = -ENOENT;
		}

		break;

	default:
		printf("error: too many arguments\n");

		ret = -EINVAL;
		break;
	}

	return ret;
}
