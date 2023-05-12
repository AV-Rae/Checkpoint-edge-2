#include <Adafruit_Sensor.h>                        
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>

//************************************************************************
// Definiçoes de controle
//************************************************************************
#define DHTTYPE     DHT11                      // Sensor DHT11
#define DHTPIN      2                          // Pino do Arduino conectado no Sensor(Data) 

// Controle de Luz
int ESCURO   = 0;
int MEIA_LUZ = 1;
int CLARO    = 2;
int luz = ESCURO;

// Controle de temperatura
int TEMP_OK    = 0;
int FRIO       = 1;
int QUENTE     = 2;
int TEMP_BAIXA = 20;
int TEMP_ALTA  = 28;
int temperaturaAmbiente = TEMP_OK;

// Controle de umidade
int UMID_OK    = 0;
int UMID_BAIXA = 1;
int UMID_ALTA  = 2;
int umidadeAmbiente = UMID_OK;

// Controle do display
int tempoMostrarDisplay = 1000;//5000;

int const MOSTRA_LUZ  = 0;
int const MOSTRA_TEMP = 1;
int const MOSTRA_UMID = 2;
int estadoDisplay = MOSTRA_LUZ;

//************************************************************************
// Mapeamento dos pinos
//************************************************************************
int ledVermelhoPin = 11;  // LED vermelho no pino 9
int ledAmareloPin  = 12;  // LED amarelo no pino 10
int ledVerdePin    = 13;  // LED verde no pino 11
int ldrPin         = A1;  // LDR no pino A1
int buzzerPin      = 10;   // Buzzer no pino 7
int sensorTempPin  = A2;  // Sensor de temperatura no pino A2
int sensorUmidPin  = A0;  // Sensor de umidade no pino A0

LiquidCrystal LCD(9, 8, 7, 6, 5, 4, 3);
DHT_Unified dht(DHTPIN, DHTTYPE);                                               // Pino do Arduino conectado no Sensor(Data) 

//************************************************************************
// Variaveis
//************************************************************************
int   valorLDR    = 0;
float temperatura = 0;
int   umidade     = 0;

int millisDisplayAgora = 0;
int millisDisplayAnterior = 0;

int   contadorMedia = 0;
float somaTemperatura = 0;
int   somaUmidade = 0;

float mediaTemperatura = 0;
int   mediaUmidade = 0;

//************************************************************************
// Funcao de configuracao dos pinos e perifericos
//************************************************************************
void setup(){
  LCD.begin(16,2);
  pinMode (ledVermelhoPin, OUTPUT);  
  pinMode (ledAmareloPin, OUTPUT);
  pinMode (ledVerdePin, OUTPUT);
  pinMode (ldrPin, INPUT);
  pinMode (buzzerPin, OUTPUT);
  //pinMode (sensorTempPin, INPUT);
  pinMode (sensorUmidPin, INPUT);
  Serial.begin(9600);
  dht.begin(); 
}

//************************************************************************
// Funcao principal - ela vai ficar em loop executando direto
//************************************************************************
void loop(){

  // A primeira coisa que precisamos fazer é ler os sensores
  ler_sensores();
  
  // A segunda coisa que precisamos fazer é verificar se as leituras estao OK
  verifica_sensores();
  
  // A terceira coisa é ativar os LEDs e o Buzzer
  aciona_alarmes();
  
  // Por fim, precisamos mostrar a mensagem no LCD
  mostra_mensagem();
}

//************************************************************************
// Funcao de leitura dos sensores
//************************************************************************
void ler_sensores(){

  //Lê o valor do sensor ldr e armazena na variável valorLDR
  valorLDR = analogRead (ldrPin); 
  Serial.print("LDR: ");   
  Serial.println(valorLDR);
   
  // Le o valor da temperatura
  sensors_event_t event;                        // inicializa o evento da Temperatura
  dht.temperature().getEvent(&event);           // faz a leitura da Temperatura
  temperatura = event.temperature;
  
  Serial.print("Temperatura: ");
  Serial.println(temperatura);
  
  // Le o valor de Umidade
  dht.humidity().getEvent(&event); 
  umidade = event.relative_humidity; 
  Serial.print("Umidade: ");
  Serial.println(umidade);
}

//************************************************************************
// Funcao para avaliar as leituras dos sensores e acionar os leds/buzina
//************************************************************************
void verifica_sensores(){

  // Primeiro vamos verificar se a iluminacao esta ok
  Serial.print("Ambiente esta ");
  if(valorLDR <= 700){
    luz = ESCURO;
    Serial.println("Escuro");
  } 
  else if(valorLDR > 700 && valorLDR <= 900){
    luz = MEIA_LUZ;
    Serial.println("Meia Luz");
  }
  else if(valorLDR > 900){
    luz = CLARO;
    Serial.println("Claro");
  }
  
  // Segundo, vamos verificar o sensor de temperatura
  Serial.print("Temperatura esta ");
  if(temperatura < TEMP_BAIXA){
    temperaturaAmbiente = FRIO;
    Serial.println("fria");
  }
  else if(temperatura > TEMP_ALTA){
    temperaturaAmbiente = QUENTE;
    Serial.println("quente");
  }
  else {
    temperaturaAmbiente = TEMP_OK;
    Serial.println("OK");
  }
  
  // Por fim vamos verificar o sensor de umidade
  Serial.print("Umidade esta ");
  if(umidade < 50){
    umidadeAmbiente = UMID_BAIXA;
    Serial.println("baixa");
  }
  else if(umidade > 70){
    umidadeAmbiente = UMID_ALTA;
    Serial.println("alta");
  }
  else {
    umidadeAmbiente = UMID_OK;
    Serial.println("OK");
  }
}

//************************************************************************
// Funcao para ativar os LEDs e o Buzzer
//************************************************************************
void aciona_alarmes(){

  // Enquanto o ambiente estiver escuro, o LED Verde deve ficar aceso (Req. 1)
  if(luz == ESCURO){
    digitalWrite(ledVerdePin, HIGH);
  }
  else{
    digitalWrite(ledVerdePin, LOW);
  }
  
  // Enquanto o ambiente estiver a meia luz, o LED amarelo deve ficar aceso... (Req. 2)
  // Enquanto a temperatura estiver fora da faixa ideal, o LED Amarelo deve ficar aceso e o Buzzer deve ligar continuamente (Req. 8)
  if(luz == MEIA_LUZ || temperaturaAmbiente != TEMP_OK){
    digitalWrite(ledAmareloPin, HIGH);    
    if(temperaturaAmbiente != TEMP_OK){
      digitalWrite(buzzerPin, HIGH);
    }
  }
  else{
    digitalWrite(ledAmareloPin, LOW);
    
    // Verificando se o Buzzer precisa ficar ligado por causa da umidade
    if(umidadeAmbiente == UMID_OK){
      digitalWrite(buzzerPin, LOW);
    }   
  }
  
  // Enquanto o ambiente estiver totalmente iluminado, o LED vermelho deve ficar aceso... (Req. 3)
  // Enquanto a umidade estiver fora da faixa ideal, o LED Vermelho deve ficar aceso e o Buzzer deve ligar continuamente (Req. 11)
  if(luz == CLARO || umidadeAmbiente != UMID_OK){
    digitalWrite(ledVermelhoPin, HIGH);
    
    // Enquanto o ambiente estiver totalmente iluminado, o Buzzer deve ficar ligado continuamente (Req. 4)
    digitalWrite(buzzerPin, HIGH);
  }
  else{
    digitalWrite(ledVermelhoPin, LOW);
    
    // Verificando se o Buzzer precisa ficar ligado por causa da temperatura
    if(temperaturaAmbiente == TEMP_OK){
      digitalWrite(buzzerPin, LOW);
    }
  }
  
}

//************************************************************************
// Funcao para mostrar as mensagens no LCD
//************************************************************************
void mostra_mensagem(){
  
  // pegando quantos milissegundos o arduino esta ligado
  millisDisplayAgora = millis();
  
  // Para mostrar os valores, precisamos da média dos ultimos 5 segundos
  // Para isso, vamos primeiro fazer um somatorio dos dados lidos:
    somaTemperatura = somaTemperatura + temperatura;
    somaUmidade = somaUmidade + umidade;
  contadorMedia++;
  
  // Verifica se passou o tempo para poder atualizar a mensagem no display
  if(millisDisplayAgora - millisDisplayAnterior > tempoMostrarDisplay){
  
    // Preparando o LCD para mostrar a mensagem
      LCD.clear();
    LCD.setCursor(0,0); 
     
    switch(estadoDisplay){
      case (MOSTRA_LUZ):
        if(luz == ESCURO){
          LCD.print("Luz Ambiente OK");
        }
        else if(luz == MEIA_LUZ){
          LCD.print("Ambiente a");
          LCD.setCursor(0,1);
          LCD.print("meia luz");
        }
        else{
          LCD.print("Ambiente");
          LCD.setCursor(0,1);
          LCD.print("muito claro");
        }
        estadoDisplay = MOSTRA_TEMP;
      break;
      
      case(MOSTRA_TEMP):
        mediaTemperatura = somaTemperatura / contadorMedia;
        
        if(temperaturaAmbiente == TEMP_OK){
          LCD.print("Temperatura OK");
          LCD.setCursor(0,1);
          LCD.print("Temp. = "); LCD.print(mediaTemperatura); LCD.print("C");
        }
        else if(temperaturaAmbiente == FRIO){
          LCD.print("Temp. Baixa");
          LCD.setCursor(0,1);
          LCD.print("Temp. = "); LCD.print(mediaTemperatura); LCD.print("C");
        }
        else{
          LCD.print("Temp. Alta");
          LCD.setCursor(0,1);
          LCD.print("Temp. = "); LCD.print(mediaTemperatura); LCD.print("C");
        }
      estadoDisplay = MOSTRA_UMID;
      break;
      
      case(MOSTRA_UMID):
        mediaUmidade = somaUmidade / contadorMedia;
      
        if(umidadeAmbiente == UMID_OK){
          LCD.print("Umidade OK");
          LCD.setCursor(0,1);
          LCD.print("Umidade = "); LCD.print(mediaUmidade); LCD.print("%");
        }
        else if(umidadeAmbiente == UMID_BAIXA){
          LCD.print("Umidade Baixa");
          LCD.setCursor(0,1);
          LCD.print("Umidade = "); LCD.print(mediaUmidade); LCD.print("%");
        }
        else{
          LCD.print("Umidade Alta");
          LCD.setCursor(0,1);
          LCD.print("Umidade = "); LCD.print(mediaUmidade); LCD.print("%");
        }
      
      estadoDisplay = MOSTRA_LUZ;
      break;
      
      default:
        estadoDisplay = MOSTRA_LUZ;
      break;
    }
    // Zerando as vairaveis de média
    somaTemperatura = 0;
    somaUmidade = 0;
    contadorMedia = 0;
    
    // Atualizando o tempo para só entrar daqui a tempoMostrarDisplay milissegundos
    millisDisplayAnterior = millis();
  }
}