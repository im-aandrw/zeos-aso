#include <libc.h>

char buff[24];
extern struct circular_buffer keyboard_buffer; 
extern void del_circular_buffer(char *c, struct circular_buffer *b);

static void write_str(char *s)
{
    write(1, s, strlen(s));
}

static void write_int(int n)
{
    char c[32];
    itoa(n, c);
    write_str(c);
}

static void write_kv(char *label, int n)
{
    write_str(label);
    write_int(n);
    write_str("\n");
}

static void wait_ticks(int delta)
{
    int start = gettime();
    while (gettime() < start + delta) { }
}

static void rr_demo(char *tag, int iterations, int delta)
{
    int i;
    for (i = 0; i < iterations; ++i) {
        write_str(tag);
        write_str(" tick=");
        write_int(gettime());
        write_str("\n");
        wait_ticks(delta);
    }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    int ret;
    int pid;
    int value;

    write_str("ZEOS final integration test\n");
    write_kv("initial pid=", getpid());
    write_kv("initial ticks=", gettime());

    write_str("\n[1] Round robin + fork + exit\n");
    ret = fork();
    if (ret == 0) {
        pid = getpid();
        write_kv("rr child pid=", pid);
        rr_demo("rr child", 3, 3);
        write_str("rr child exits\n");
        exit();
    } else {
        write_kv("rr parent child_pid=", ret);
        rr_demo("rr parent", 3, 3);
        wait_ticks(15);
    }

    write_str("\n[2] Real block/unblock\n");
    ret = fork();
    if (ret == 0) {
        pid = getpid();
        write_kv("block child pid=", pid);
        write_str("block child goes to sleep\n");
        block();
        write_str("block child resumed\n");
        exit();
    } else {
        write_kv("block parent child_pid=", ret);
        wait_ticks(25);
        write_str("block parent calls unblock\n");
        value = unblock(ret);
        write_kv("block parent unblock ret=", value);
        wait_ticks(15);
    }

    write_str("\n[3] Pending unblock\n");
    ret = fork();
    if (ret == 0) {
        pid = getpid();
        write_kv("pending child pid=", pid);
        wait_ticks(15);
        write_str("pending child calls block\n");
        block();
        write_str("pending child continued\n");
        exit();
    } else {
        write_kv("pending parent child_pid=", ret);
        value = unblock(ret);
        write_kv("pending parent unblock ret=", value);
        wait_ticks(25);
    }

    write_str("\n[4] Exit with alive child\n");
    ret = fork();
    if (ret == 0) {
        int grandchild = fork();
        if (grandchild == 0) {
            write_str("grandchild alive after parent exit\n");
            wait_ticks(12);
            write_str("grandchild exits\n");
            exit();
        } else {
            write_kv("child created grandchild pid=", grandchild);
            write_str("child exits first\n");
            exit();
        }
    } else {
        wait_ticks(25);
        write_str("parent survived reparenting test\n");
    }

    write_str("\n[5] Invalid unblock\n");
    value = unblock(999);
    if (value < 0) {
        write_str("invalid unblock failed: ");
        perror();
        write_str("\n");
    } else {
        write_kv("invalid unblock ret=", value);
    }

    write_str("Pulsar teclas en Boch... \n");
	while(1)
	{
		int c = getchar();
		if (c != -1)
		{
			char ch = (char)c;
			write(1, &ch, 1);
		}
	}

    write_str("\nAll tests finished\n");
    while (1) { }
}
