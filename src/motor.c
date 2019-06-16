#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h> // using the termios.h library
#include <stdbool.h>

#define XMOTOR 0
#define YMOTOR 1

#define ARDUINO_RST -1 // pin which will reset the Arduino board whenever the code starts

#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

// Declaracao das funcoes.
void setupCommSerial();
void getValidData();
void storeValidData();

// Variaveis globais.
char msg[MSG_MAX];
bool newMsg = false;
int arduino;

int x_motor, y_motor;

void init_motors()
{
	int i = 0;
	char devpath[20];

	setupCommSerial(); // Setup comunicacao serial.

	unsigned long int now = micros();

	x_motor = 0;
	y_motor = 0;
}

void setupCommSerial()
{
	int ok = 0;
	char port[15];
	int i = 0;
	int arduinoACM;
	// printf("Trying to connect to Arduino...\n");
	do
	{
		if (i > 20)
		{
			i = 0;
		}
		sprintf(port, "/dev/ttyACM%d", i);
		arduinoACM = serialOpen(port, BAUDRATE); // Inicia comunicacao serial com baud rate 115200bps.
		if (arduinoACM <= 0)
		{
			sprintf(port, "/dev/ttyUSB%d", i++);
			arduino = serialOpen(port, BAUDRATE); // Inicia comunicacao serial com baud rate 115200bps.
		}
		else
		{
			arduino = arduinoACM;
		}
		delay(50);
	} while (arduino <= 0);
	// printf("Connected to Arduino at %s\n", port);

	//while (wiringPiSetup() == -1);		// Necessita de sudo.

	do // Espera a estabilizacao da comunicacao serial.
	{
		while (serialDataAvail(arduino))
		{
			msg[0] = serialGetchar(arduino);
			//printf("%c...\n",   msg[0]);			// Apenas para testes.
			if (msg[0] == 'c') // Msg de nao recebimento.
			{
				serialPutchar(arduino, 'b'); // Reenvia msg de confirmacao de recebimento.
			}
			else if (msg[0] == 'a') // Msg esperada receber, envio e recibo de msg funcionando.
			{
				ok = 1;
			}
		}
	} while (!ok);
}

static int lenght = 0;
void getValidData()
{
	// static int msgp = 0;		// pontero da msg.
	// char startChar = ':';		// char define inicio da msg.
	// char endChar = ';';			// char define fim da msg.

	// while((serialDataAvail(arduino)) && (!newMsg))
	// {
	// 	msg[msgp] = serialGetchar(arduino);
	// 	//printf("%c\n", msg[msgp]);			// Apenas para testes.
	// 	if (msg[msgp] == startChar)			// Ageita para comecar a colocar a msg no inicio de msg[].
	// 	{
	// 		msgp = -1;
	// 	}
	// 	else
	// 	if (msg[msgp] == endChar)			// Finaliza a obtencao da msg encerrando a string msg[].
	// 	{
	// 		msg[msgp] = '\0';
	// 		lenght = msgp - 1;
	// 		newMsg = true;						// Sinaliza uma nova msg recebida.
	// 	}

	// 	msgp++;
	// 	if (msgp >= MSG_MAX)				// Evitar a escrita em memoria inacessivel.
	// 	{
	// 		msgp = MSG_MAX - 1;
	// 	}
	// }
}

void storeValidData() // Aqui que sera mudado para nossas necessidades. No caso espera receber apenas um float.
{
	// if (newMsg)
	// {
	// 	//printf("msg: %s...\n", msg);
	// 	if(msg[lenght] == 'l')
	// 	{
	// 		if(msg[lenght-1] == 'd')
	// 		{
	// 			left_motor.vec_disp[left_motor.n_disp%MEDIAN_SIZE_MOTOR] = atof(msg);
	// 			left_motor.displacement = getMediana_motor(left_motor.vec_disp);
	// 			left_motor.n_disp++;

	// 			old_time_motorL = now_time_motorL;
	// 			now_time_motorL = micros();
	// 			left_motor.dt = (now_time_motorL - old_time_motorL)/1000000.0;
	// 		} else if(msg[lenght-1] == 's'){
	// 			left_motor.vec_speed[left_motor.n_speed%MEDIAN_SIZE_MOTOR] = atof(msg);
	// 			left_motor.speed = getMediana_motor(left_motor.vec_speed);
	// 			left_motor.n_speed++;
	// 		}
	// 	} else if(msg[lenght] == 'r') {
	// 		if(msg[lenght-1] == 'd')
	// 		{
	// 			right_motor.vec_disp[right_motor.n_disp%MEDIAN_SIZE_MOTOR] = atof(msg);
	// 			right_motor.displacement = getMediana_motor(right_motor.vec_disp);
	// 			right_motor.n_disp++;

	// 			old_time_motorR = now_time_motorR;
	// 			now_time_motorR = micros();
	// 			right_motor.dt = (now_time_motorR - old_time_motorR)/1000000.0;
	// 		} else if(msg[lenght-1] == 's'){
	// 			right_motor.vec_speed[right_motor.n_speed%MEDIAN_SIZE_MOTOR] = atof(msg);
	// 			right_motor.speed = getMediana_motor(right_motor.vec_speed);
	// 			right_motor.n_speed++;
	// 		}
	// 	}
	// 	//valor = atof(msg);						// Transforma double, 0 para entradas invalidos.
	// 	//printf("msg: %s...\n", msg);				// Apenas para testes.
	// 	//printf("valor: %f...\n", velocidade);		// Apenas para testes.
	// 	newMsg = false;
	// }
}

int write_motors()
{
	char deliver[MSG_MAX];
	char tmp;
	int i = 0;

	snprintf(deliver, MSG_MAX, ":%dx;", x_motor); // Prepara a msg a ser enviada.
	while (deliver[i] != '\0')
	{
		serialPutchar(arduino, deliver[i]); // Envio da msg char por char ate fim da string. Obs: existe na biblioteca a funcao "serialPrintf" mas ela nao funcionou.
		//printf("%c", deliver[i]);
		i++;
		if (i >= MSG_MAX) // Evitar ir alem da string.
		{
			break;
		}
	}
	tmp = serialGetchar(arduino);
	if ((tmp != 'r') && (tmp != 'f'))
	{
		serialClose(arduino);
		return -1;	// Character invalido.
	} else if (tmp == 'f') {
		return -2;	// Fim de curso ativado.
	}
	
	i = 0;
	snprintf(deliver, MSG_MAX, ":%dy;", y_motor); // Prepara a msg a ser enviada.
	while (deliver[i] != '\0')
	{
		serialPutchar(arduino, deliver[i]); // Envio da msg char por char ate fim da string. Obs: existe na biblioteca a funcao "serialPrintf" mas ela nao funcionou.
		//printf("%c", deliver[i]);
		i++;
		if (i >= MSG_MAX) // Evitar ir alem da string.
		{
			break;
		}
	}
	tmp = serialGetchar(arduino);
	if ((tmp != 'r') && (tmp != 'f'))
	{
		serialClose(arduino);
		return -1;	// Character invalido.
	} else if (tmp == 'f') {
		return -2;	// Fim de curso ativado.
	}
}

void setMotorSpeed(int motor, double speed)
{
	if (motor == XMOTOR)
	{
		x_motor = speed;
	}
	else if (motor == YMOTOR)
	{
		y_motor = speed;
	}
}

// // Abdullah's QuickSort implementation for usage with the Median Filter
// unsigned Partition_motor(double array[], unsigned f, unsigned l, double pivot)
// {
//     unsigned i = f-1, j = l+1;
//     while(1)
//     {
//         while(pivot < array[--j]);
//         while(array[++i] < pivot);
//         if(i<j)
//         {
//             double tmp = array[i];
//             array[i] = array[j];
//             array[j] = tmp;
//         }
//         else
//             return j;
//     }
// }

// void QuickSortImpl_motor(double array[], unsigned f, unsigned l)
// {
//     while(f < l)
//     {
//         unsigned m = Partition_motor(array, f, l, array[f]);
//         QuickSortImpl_motor(array, f, m);
//         f = m+1;
//     }
// }

// void QuickSort_motor(double array[], unsigned size)
// {
//     QuickSortImpl_motor(array, 0, size-1);
// }
