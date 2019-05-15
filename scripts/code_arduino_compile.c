#include<stdio.h>
#include<string.h>
#include<stdlib.h>


#define LOGIN		"pi"
#define	PASSWORD	"atitudes"

int main(int argc, char const *argv[]) 
{
	

	char ip_rasp[16] = "192.168.200.1\0";
	char name[50];
	char dev[20] = "/dev/ttyACM0\0";
	char scpcommand[500];
	char arduino_command[500];
	int c = 0, ip = 0, f = 0;

	// arduino -c codigo [-ip address] [-p port]
	if (argc < 3){
		printf ("Too few arguments, bro! Try something like: arduino_compile -c FILENAME [-ip IPADDRESS] [-f DEVFILE] \n");
		return 0;
	}
	

	for (int i = 1; i < argc; i++){
		if ((strcmp(argv[i], "-c"))==0){
			c = i;
		}
		else if ((strcmp(argv[i], "-ip"))==0){
			ip = i;
		}
		else if ((strcmp(argv[i], "-f")) ==0 ){
			f = i;
		}

	}
	

	if (c){

		strcpy(name,argv[c+1]);

			
	}
	else{
		printf ("Missing -c argument, bro! Try something like: arduino_compile -c FILENAME [-ip IPADDRESS] [-f DEVFILE] \n");
		return 0;
	}

	if (ip){
		strcpy(ip_rasp,argv[ip+1]);
	}
	else {
		printf("Just letting you know I'm using default IP Address: %s \n", ip_rasp);
	}

	if (f){
		strcpy(dev,argv[f+1]);
	} else {
		printf("Just letting you know I'm looking for an Arduino in: %s \n", dev);
	}
	

	sprintf(scpcommand, "sshpass -p %s scp -r ~/git/mesa/src/arduino/%s %s@%s:/home/pi/src/arduino", PASSWORD, name, LOGIN, ip_rasp);
	sprintf(arduino_command,"sshpass -p %s ssh %s@%s 'cd /home/pi/src/arduino && arduino --upload %s/%s.ino --port %s'", PASSWORD, LOGIN, ip_rasp,name,name, dev);


	system(scpcommand);
	system(arduino_command);

	return 0;
}