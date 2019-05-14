#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

int main()
{
	int event_fd;
	static struct ff_effect effects;
	static struct input_event play;

	effects.type = FF_RUMBLE;
	effects.u.rumble.strong_magnitude = 65535;//100%
	effects.u.rumble.weak_magnitude = 65535;//!00%
	effects.replay.length = 1000;//duração em milisegundos
	effects.replay.delay = 0;
	effects.id = -1;

	play.type = EV_FF;
	play.code = effects.id;
	play.value = 1;


	event_fd = open("/dev/input/event19", O_RDWR);

	printf("%d\n", ioctl(event_fd, EVIOCSFF, &effects));

	

	printf("%d\n", write(event_fd, (const void*) &play, sizeof(play)));

	

	system("sleep 2");

	return 0;
}