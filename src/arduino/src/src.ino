#include "PololuDriver.h"
#include "Ramps.h"

// For RAMPS 1.4
#define SEMI_ATI_MOTOR   3
#define SEMI_ATI_MANCAL  2

#define CAR_ATI          14
#define CAR_PAS          15

#define SEMI_PAS_MANCAL  18
#define SEMI_PAS_MOTOR   19

#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

#define VELMIM 0
#define VELMAX 1023
#define MICROMAX 10000
#define MICROMIN 250

#define SLEEP_MOTORS 32

Ramps ramps = Ramps();


// Variaveis globais.
char msg[MSG_MAX];
boolean newMsg = false;

int lenght = 0;
boolean naoh_pohde = false;

int velY = 0;
int velX = 0;
unsigned long microsecondsX = 1000;
unsigned long microsecondsY = 1000;
int xativo = 0;
int yativo = 0;
unsigned long lastupdatY = 0;
unsigned long lastupdatX = 0;

void getValidData()
{
  static int msgp = 0;    // pontero da msg.
  char startChar = ':';   // char define inicio da msg.
  char endChar = ';';     // char define fim da msg.

  while((Serial.available()) && (!newMsg))
  {
    msg[msgp] = Serial.read();
    //Serial.print(msg[msgp]);      // Apenas para testes.
    if (msg[msgp] == startChar)     // Ageita para comecar a colocar a msg no inicio de msg[].
    {
      msgp = -1;
    }
    else
    if (msg[msgp] == endChar)     // Finaliza a obtencao da msg encerrando a string msg[].
    {
      msg[msgp] = '\0';
      lenght = msgp - 1;
      newMsg = true;            // Sinaliza uma nova msg recebida.
    }
    
    msgp++;
    if (msgp >= MSG_MAX)        // Evitar a escrita em memoria inacessivel.
    {
      msgp = MSG_MAX - 1;
    }
  }
}

void storeValidData() // Aqui que sera mudado para nossas necessidades. No caso espera receber apenas um float.
{
  if(newMsg)
  {
    if(msg[lenght] == 'y')
    {
      velY = String(msg).toInt();
    } else if(msg[lenght] == 'x') {
      velX = String(msg).toInt();
    }
    //valor = String(msg).toFloat();    // Transforma float, 0 para entradas invalidos.
    //Serial.println(msg);        // Apenas para testes.
    //Serial.println(valor, 5);     // Apenas para testes.
    newMsg = false;
  }
}

void send_msg()
{
//  Serial.print(":");
//  Serial.print(ldisplacement, SEND_PRECISION);
//  Serial.print("dl;:");
//
//  //Serial.print(":");
//  Serial.print(velocidade_esquerda, SEND_PRECISION);
//  Serial.print("sl;:");
//  
//  //Serial.print(":");
//  Serial.print(rdisplacement, SEND_PRECISION);
//  Serial.print("dr;:");
//  
//  //Serial.print(":");
//  Serial.print(velocidade_direita, SEND_PRECISION);
//  Serial.print("sr;");
}


void serialEvent()  // Obs: ocorre apenas depois de loop() ser executado.
{
  getValidData();     // Obtem msg sem parar o codigo, importante que se tenha uma boa frequencia de execucao de loop().
  storeValidData();   // Armazena msg caso tenha tido uma msg completa recebida.
  if (naoh_pohde) {
    Serial.print("f");  // Fim de curso.
  } else {
    Serial.print("r");  //ack de recebimento
  }
}

void setup()
{
  int ok = 0;
  noInterrupts(); // Nao permite interrupcoes.
  Serial.begin(BAUDRATE);
  interrupts();// Volta a permitir interrupcoes.
  while(!ok)            // Espera a estabilizacao da comunicacao serial.
  {
    if (Serial.available() > 0)
    {
      msg[0] = Serial.read();
      if(msg[0] == 'b')     // Msg esperada receber, envio e recibo de msg funcionando.
      {
        Serial.print("a");    // Resposta de que obteve comunicacao, 
        ok = 1;
      }
    }
    else
    {
      Serial.print("c");      // Msg de persistencia na espera.
    }
    delay(100);         // Simplesmente para nao enviar muitas msgs desnecessarias.
  }


  pinMode(SLEEP_MOTORS, OUTPUT);
    
}

void loop ()
{

//    Serial.print("Min X Y Z: ");
//    Serial.print(digitalRead(X_MIN_PIN));
//    Serial.print(digitalRead(Y_MIN_PIN));
//    Serial.print(digitalRead(Z_MIN_PIN));
//
//    Serial.print("Max X Y Z: ");
//    Serial.print(digitalRead(X_MAX_PIN));
//    Serial.print(digitalRead(Y_MAX_PIN));
//    Serial.print(digitalRead(Z_MAX_PIN));

    

    if(velX == 0 && velY == 0)
    {
      //dormindo
      digitalWrite(SLEEP_MOTORS,LOW);
    }
    else
    {
      //acordando
      digitalWrite(SLEEP_MOTORS,HIGH);
      if(xativo)
      {
        ramps.motorX.stepOff();
        xativo = 0;
      }
      if(velX == 0)
      {
        ramps.motorX.stepOff();
        lastupdatX = micros();  
      }
      else
      {
        if(velX>0)
        {
          if (!digitalRead(CAR_PAS)) {
            naoh_pohde = true;
            digitalWrite(SLEEP_MOTORS,LOW);
          } else {
            naoh_pohde = false;
            digitalWrite(SLEEP_MOTORS,HIGH);
            ramps.motorX.setDir(1);
            microsecondsX = map(velX, VELMIM, VELMAX, MICROMAX, MICROMIN); 
          }
        }
        else
        {
          if (!digitalRead(CAR_ATI)) {
            naoh_pohde = true;
            digitalWrite(SLEEP_MOTORS,LOW);
          } else {
            naoh_pohde = false;
            digitalWrite(SLEEP_MOTORS,HIGH);
            ramps.motorX.setDir(-1);
            microsecondsX = map(-velX, VELMIM, VELMAX, MICROMAX, MICROMIN);  
          }
        }
        
        if(micros()-lastupdatX >= microsecondsX)
        { 
          ramps.motorX.stepOn();
          xativo = 1;
          
          lastupdatX = micros();
        } 
      }
  
       if(yativo)
      {
        ramps.motorY.stepOff();
        ramps.motorZ.stepOff();
        yativo = 0;
      }
  
      if(velY == 0)
      {
        ramps.motorY.stepOff();
        ramps.motorZ.stepOff();
        lastupdatY = micros();  
      }
      else
      {
        if(velY>0)
        {
          if ((!digitalRead(SEMI_ATI_MANCAL)) || (!digitalRead(SEMI_PAS_MANCAL))) {
            naoh_pohde = true;
            digitalWrite(SLEEP_MOTORS,LOW);
          } else {
            naoh_pohde = false;
            digitalWrite(SLEEP_MOTORS,HIGH);
            ramps.motorY.setDir(1);
            ramps.motorZ.setDir(1);
            microsecondsY = map(velY, VELMIM, VELMAX, MICROMAX, MICROMIN); 
          }
        }
        else
        {
          if ((!digitalRead(SEMI_ATI_MOTOR)) || (!digitalRead(SEMI_PAS_MOTOR))) {
            naoh_pohde = true;
            digitalWrite(SLEEP_MOTORS,LOW);
          } else {
            naoh_pohde = false;
            digitalWrite(SLEEP_MOTORS,HIGH);
            ramps.motorY.setDir(-1);
            ramps.motorZ.setDir(-1);
            microsecondsY = map(-velY, VELMIM, VELMAX, MICROMAX, MICROMIN);  
          }
        }
        
        if(micros()-lastupdatY >= microsecondsY)
        {
          ramps.motorY.stepOn();
          ramps.motorZ.stepOn();
          yativo = 1;
          
          lastupdatY = micros();
        } 
      }
    }
}
