/*
Modos:
1- Normal
2- Amarelo Piscante
3- Manual 

Estados:
1- 1Vd - 2Vm
2- 1Am - 2Vm
3- 1Vm - 2Vd
4- 1Vm - 2Am

Velocidades:
1-Rápido
2-Normal
3-Lento
*/

#include <EEPROM.h> //Biblioteca para para ler e escrever valores na memória EEPROM

#define delayV1 5; //definindo os delays
#define delayV1Am 1;
#define delayV2 20;
#define delayV2Am 6;
#define delayV3 80;
#define delayV3Am 12;

#define sem1Vm 7 //definindo as portas do semáforo 1
#define sem1Am 6
#define sem1Vd 5

#define sem2Vm 4 //definindo as portas do semáforo 2
#define sem2Am 3
#define sem2Vd 2



byte modoAnt;
byte estadoAnt;
byte modo;
byte estado;
byte velocidade;
byte startMode;
byte startVelo;
byte pisca;
int delayControl;
int cicloSemaforo;

String comando;
char serialBuff;

void setup() {
  
  pinMode(sem1Vm, OUTPUT); //definindo as portas como saída
  pinMode(sem1Am, OUTPUT); 
  pinMode(sem1Vd, OUTPUT); 
  pinMode(sem2Vm, OUTPUT); 
  pinMode(sem2Am, OUTPUT); 
  pinMode(sem2Vd, OUTPUT);

  //Lê modo de inicio e grava o próximo modo de inicio na EEPROM
  startMode = EEPROM.read(0);
  if (startMode == 0){
     EEPROM.write(0, 1);
  } else { 
     EEPROM.write(0, 0);
  }

  //Lê modo de inicio e grava o próximo modo de inicio na EEPROM
  startVelo = EEPROM.read(1);
  if ((startVelo < 1) || (startVelo > 3)) { 
     startVelo = 2; //Velocidade Inicial = Normal
     EEPROM.write(1, 2);
  } 

  //Inicia as variaveis
  modo   = startMode + 1;
  estado = 1; 
  velocidade = startVelo; 
  pisca  = 0;
  delayControl = 0;
  cicloSemaforo = 0;

  modoAnt = 99; //Força a primeira execução
  estadoAnt = 99; //Força a primeira execução

  comando = "";
  
  Serial.begin(9600);
}

void loop() {

  if (delayControl < 49) {  //Executa 49 ciclos sem atividade, porém recebendo comandos. As atividades são executadas apenas no ciclo 50.
    
    while (Serial.available() > 0) {
       serialBuff = Serial.read();
       comando = comando + serialBuff;
    }

    if (comando != "") {

       if ((comando.substring(0,1) == "V") || (comando.substring(0,1) == "v")) {  //Velocidade
           if (comando.substring(1,2) == "1") { velocidade = 1; }  //Rápido
           if (comando.substring(1,2) == "2") { velocidade = 2; }  //Normal
           if (comando.substring(1,2) == "3") { velocidade = 3; }  //Lento            
           cicloSemaforo = 0;
           EEPROM.write(1, velocidade);
        }
        
        if ((comando.substring(0,1) == "M") || (comando.substring(0,1) == "m")) {  //Modo
           if (comando.substring(1,2) == "1") { modo = 1; }  //Normal
           if (comando.substring(1,2) == "2") { modo = 2; }  //Amarelo Piscante
           if (comando.substring(1,2) == "3") { modo = 3; }  //Manual  

           cicloSemaforo = 0;
           delayControl = 50;
           estadoAnt = 99; //Força a execução
        }

        if (comando.substring(0,1) == "*") {  //Muda o estado do semáforo
           estado++;
           if (estado > 4) { estado = 1; }        

           delayControl = 50; //Força a execução imediata
        }

        comando = "";
    }

    delayControl++;
  } else {

    if ((modo == 1) || (modo == 3)) { //Modo Automático ou Manual

        if (modo != modoAnt) { //Inicia o estado para quando o modo for alterado para normal (também é executado quando iniciado)
           estado = 1;
           cicloSemaforo = 0;
        } 

        if (cicloSemaforo > 0) { //Conta os ciclos para mudar de fase
           cicloSemaforo--;
        } else { 

           if (modo == modoAnt) { //Inicia o estado para quando o modo for alterado para normal (também é executado quando iniciado)
              if (modo == 1) { //Se for modo Automático
                 estado++;
                 if (estado > 4) { estado = 1; }          
              }   
           }
          
           //Atualiza o tempo do ciclo
           if ((estado == 1) || (estado == 3)) {
              if (velocidade == 1) { //Semáforo rápido
                cicloSemaforo = delayV1;   
              }
                else if(velocidade == 2){ // Semáforo Normal
                  cicloSemaforo = delayV2; 
                }
                else{ //Semáforo lento
                  cicloSemaforo = delayV3;
                }
            
           } else {
              if(velocidade == 1) { //Semáforo rápido
                  cicloSemaforo = delayV1Am;   
              }
              else if(velocidade==2){ //Semáforo normal
                cicloSemaforo = delayV2Am; 
              }
              else{ //Semáforo lento
                cicloSemaforo = delayV3Am;
              }   
           }
        }

        //Processa o Estado
        if (estado != estadoAnt) {
          if (estado == 1) {
            digitalWrite(sem1Vm, LOW);
            digitalWrite(sem1Am, LOW);
            digitalWrite(sem2Vd, LOW);
            digitalWrite(sem2Am, LOW);
                  
            digitalWrite(sem1Vd, HIGH);                 
            digitalWrite(sem2Vm, HIGH);   
          }                
          else if (estado ==2){ //Semáforo 1 Amarelo aceso e Semáforo 2 vermelho aceso
            digitalWrite(sem1Vm, LOW);
            digitalWrite(sem1Vd, LOW);
            digitalWrite(sem2Vd, LOW);
            digitalWrite(sem2Am, LOW);
      
            digitalWrite(sem1Am, HIGH);                 
            digitalWrite(sem2Vm, HIGH);                 
          }
          else if(estado ==3){ //Semáforo 1 vermelho aceso e Semáforo 2 verde aceso
            digitalWrite(sem1Vd, LOW);
            digitalWrite(sem1Am, LOW);
            digitalWrite(sem2Am, LOW);
            digitalWrite(sem2Vm, LOW);
      
            digitalWrite(sem1Vm, HIGH);                 
            digitalWrite(sem2Vd, HIGH);                 
          }
          else if(estado == 4){ //Semáforo 1 vermelho aceso e Semáforo 2 amarelo aceso
            digitalWrite(sem1Vd, LOW);
            digitalWrite(sem1Am, LOW);
            digitalWrite(sem2Vd, LOW);
            digitalWrite(sem2Vm, LOW);
      
            digitalWrite(sem1Vm, HIGH);                 
            digitalWrite(sem2Am, HIGH);                 
          }
  
            estadoAnt = estado;
        }      
    }

    if (modo == 2) { //Modo Piscante
  
       digitalWrite(sem1Vm, LOW);
       digitalWrite(sem1Vd, LOW);
       digitalWrite(sem2Vm, LOW);
       digitalWrite(sem2Vd, LOW);
       
       if (pisca == 0) {
          digitalWrite(sem1Am, LOW);
          digitalWrite(sem2Am, LOW);
          pisca = 1;
       } else {
          digitalWrite(sem1Am, HIGH);
          digitalWrite(sem2Am, HIGH);
          pisca = 0;        
       }
        
    }     

    delayControl = 0;
    modoAnt = modo;    
  }

  delay(10);
}
