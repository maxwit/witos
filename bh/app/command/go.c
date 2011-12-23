
int main(int argc, char *argv[])
{
	__u32 addr;

	if (argc == 1)
		addr = get_load_mem_addr();
	else if (str_to_val(argv[1], &addr) < 0) {
			usage();
			return -EINVAL;
	}

	printf("goto 0x%08x ...\n", addr);

	((void (*)(void))addr)();

	return -EIO;
}
