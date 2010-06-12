#include <g-bios.h>
#include <flash/flash.h>
//fixme:
#include <flash/part.h>


int main(int argc, char *argv[])
{
	int ret = -EINVAL;
	static int mach_name_flat = 0;
	const struct gapp *app;
	extern const struct gapp g_app_begin[], g_app_end[];

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
			if (!strcmp(app->name, argv[1]))
			{
				mach_name_flat = 1;

				ret = 0;
				break;
			}

		if (!mach_name_flat)
		{
			printf("help: %s: No such command\n", argv[1]);
			ret = -EINVAL;
		}

		break;

	default:
		printf("error: too many arguments\n");

		ret = -EINVAL;
		break;
	}

	return ret;
}

