//=====================================INCLUINDO AS BIBLIOTECAS==============================================
  #include <LiquidCrystal_I2C.h>
  #include <Wire.h>
  #include "BluetoothSerial.h"

//=====================================CONNFIGURANDO SERVO===============================================
  #include <ESP32Servo.h>
  #define DCSBIOS_DEFAULT_SERIAL
  #include "DcsBios.h"
  #include "SPI.h"
  Servo myservo;  

//=====================================ADIOCIONANDO AS FUNÇÕES===============================================
  void Aservo();
  void Amotor();
  void Aled();
  void Bluetooth();
  void LCD();
  void Contas();

//=====================================DEFININDO OS NOME PARA CADA PINO======================================
  #define IN1 12 //_________Bomba Entrada 1 da Ponte H
  #define IN2 14 //_________Bomba Entrada 2 da Ponte H
  #define velocidadeA 27 //_Velocidade Bomba
  #define Shumidity1 33 //__Sensor de Umidade 1
  #define SLM35 35 //_______Sensor de Temperatura
  #define Sldr 34 //________LDR
  #define Schuva 15 //______Sensor de Chuva
  #define LED 18

//=====================================CONFIGURANDO CONEXÃO BLUETOOTH=========================================
  BluetoothSerial serialBLE;
//=====================================CONFIGURANDO DISPLAY LCD==============================================
  LiquidCrystal_I2C lcd (0x27, 20, 4);
 
  //=====================================CRIANDO O SIMBOLO DE PORCENTAGEM======================================
  byte porcentagem[8] = {0B11000, 0B11001, 0B00010, 0B00100, 0B01000, 0B10011, 0B00011, 0B00000 };
  byte grau[8] = { 0B00110, 0B01001, 0B01001, 0B00110, 0B00000, 0B00000, 0B00000, 0B00000 };
 
//=====================================ADICIONANDO AS VARIÁVEIS==============================================
  //_____INT_____
    int humidity1, Vumid1; // Sensor de umidade 1 e 2
    int Vtemp = 0, LM35; //_____Sensor de Temperatura
    int Vldr = 0, ldr; //_______LDR
    int Vchuva = 0, Sensor; //___Chuva
    int cont;
    int SoloSeco = 4095;
    int SoloMolhado = 1700;
    int PoucaLuz = 300;
    int MuitaLuz = 4095;
    int SemChuva = 4095;
    int ComChuva = 700;
    int percBaixo = 0; //MENOR PERCENTUAL DO SOLO SECO (0% - NÃO ALTERAR)
    int percAlto = 100; //MAIOR PERCENTUAL DO SOLO MOLHADO (100% - NÃO ALTERAR)
    int FV;
    int SV;
    int pos ;    //
    int ser;

  //_____FLOAT____
    float mV; //Sensor de Temperatura
  //_____CHAR_____
    char dados;
  //_____BOOL_____
    bool conexao;


void setup() {
 
    Serial.begin(9600); //___Inicia comunicação Serial
    serialBLE.begin("BLE"); //____Nome de identificação do BLUETOOTH

  //====================================DEFININDO PINOS DO SERVO===================================================
    myservo.attach(4);  // attaches the servo on pin 9 to the servo object
    myservo.attach(5);  // attaches the servo on pin 9 to the servo object

  //====================================INICIANDO O DIPLAY===================================================
    lcd.init(); //_________Inicia o LCD_________
    lcd.backlight(); //____Liga o Backlight_____
    lcd.createChar(0, porcentagem);
    lcd.createChar(1, grau);

  //====================================DEFININDO PINOS COMO SAÍDA===========================================
    pinMode(IN1, OUTPUT); //______Bomba______
    pinMode(IN2, OUTPUT);
    pinMode(velocidadeA,OUTPUT); //______Bomba______
    pinMode(LED, OUTPUT);
    
  //====================================DEFININDO PINOS COMO ENTRADA=========================================
    pinMode(Shumidity1, INPUT); //___Sensor de Umidade 1_____
    pinMode(SLM35, INPUT); //________Sensor de Temperatura___
    pinMode(Sldr, INPUT); //_________LDR
    pinMode(Schuva, INPUT); //_______Sensor de Chuva_________
 
  //====================================CONFIGURANDO A VELOCIDADE DO MOTOR===================================
    analogWrite(velocidadeA, 230); //Bomba

  //====================================IMPRIMINDO TEXTO UMA UNNICA VEZ NO DISPLAY===========================
    lcd.setCursor(2,0);
    lcd.print("Sejam bem vindos!");
    lcd.setCursor(0,2);
    lcd.print("Sistema de Irrigacao");
    lcd.setCursor(5,3);
    lcd.print("Automatico");
    delay(5000);
    lcd.clear();
}

void loop() {
  //====================================CALCULOS DOS SENSORES=====================================
    Contas();
  //====================================IMPRIMINDO OS VALORES NO DISPLAY=====================================
    LCD();
  //====================================ATUAÇÕES PARA CADA SENNSOR===========================================
  conexao = serialBLE.available();
  if (conexao == true){
    dados = serialBLE.read();
    if (dados == 'G'){
      while (dados != 'H'){
        Contas();
        LCD();
        dados = serialBLE.read();
        //_______SERVO_______
        if (dados == 'A' && FV == 0) {
          for (pos = 0; pos <= 180; pos +=1){
            myservo.write(pos);
            delay(20);
            FV = 180;
          }
        }
        else if(dados == 'B' && FV == 180){
          for (pos = 180; pos >= 0; pos -=1){
            myservo.write(pos);
            delay(20);
            FV = 0;
          }
        }
        //_______BOMBA_______
        if (dados == 'C'){
          digitalWrite(IN1, HIGH);
          digitalWrite(IN2, LOW);
        }
        else if (dados == 'D'){
          digitalWrite(IN1, LOW);
          digitalWrite(IN2, LOW);
        }
        //_______LED_______
        if (dados == 'E'){
          digitalWrite(LED, HIGH);
        }
        else if (dados == 'F'){
          digitalWrite(LED, LOW);
        }
      }
    }
  }
 
  //Abomba();
  Aled();
  Aservo();
}


//====================================FUNÇÃO PARA OS SENSORES==========================================
  void Contas(){
    //====================================CONTAS E MAPEAMENTO DOS SENSORES======================================
    LM35 = analogRead(SLM35);

    //-----------Mapeamentos Sensores-----------
      humidity1 = constrain(analogRead(Shumidity1), SoloMolhado, SoloSeco);
      Vumid1 = map(humidity1, SoloMolhado, SoloSeco, percAlto, percBaixo);

      ldr = constrain(analogRead(Sldr), PoucaLuz, MuitaLuz);
      Vldr = map(ldr, PoucaLuz, MuitaLuz, percBaixo, percAlto);

      Sensor = constrain(analogRead(Schuva), SemChuva, ComChuva);
      Vchuva = map(Sensor, ComChuva, SemChuva, percBaixo, percAlto);

    //-----------Calculo Stemp------------------
      mV = LM35 * (3300.0 / 4096.0);
      Vtemp = mV / 10;
  }

//====================================FUNÇÃO PARA O DISPLAY==========================================
  void LCD(){
    lcd.clear();
    //-----------Sensor de Temp-----------------
      lcd.setCursor(2,0);
      lcd.print("Temperatura: ");
      lcd.print(Vtemp);
      lcd.write(1);
      lcd.print("C");
   
    //-----------Sensor de Umidade--------------
      lcd.setCursor(2,1);
      lcd.print("Humidity: ");
      lcd.print(Vumid1);                            
      lcd.write(0);                          

    //-----------Sensor de Luminosidade(LDR)----
      if (Vldr >= 80){
        lcd.setCursor(2,2);
        lcd.print("Luminosidade ALTA");
      }
      else if (Vldr <= 79 && Vldr >= 30) {
        lcd.setCursor(2,2);
        lcd.print("Luminosidade BOA");
      }
      else if (Vldr <= 29) {
        lcd.setCursor(2,2);
        lcd.print("Luminosidade BAIXA");
      }

    //-----------Sensor de Chuva----
      Serial.print(Vchuva);
      if (Vchuva > 75) {
        lcd.setCursor(3,3);
        lcd.print("ESTA CHOVENDO!");
      }
    delay(1000);
  }
//====================================FUNÇÃO PARA A BOMBA============================================
  void Abomba(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    if (Vchuva >= 70 && Vumid1 <= 40){
      return loop();
    }
    else if (Vchuva <= 69 && Vumid1 <=40){
      do{
        //Liga a bomba
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        delay(2000);
      }while(Vumid1 >= 65 || dados == 'A');
    }
  }
//====================================FUNÇÃO PARA O SERVO============================================
  void Aservo(){
    if (Vldr <= 49 && FV == 0){
      for (pos = 0; pos <= 180; pos +=1){
        myservo.write(pos);
        delay(20);
        FV = 180;
      }
    }
    else if (Vldr >= 80 && FV == 180){
      for (pos = 180; pos >= 0; pos -=1){
        myservo.write(pos);
        delay(20);
        FV = 0;
      }
    }
  }
//====================================FUNÇÃO PARA O LED============================================
  void Aled(){
    if (Vldr <= 20) {
      digitalWrite(LED, HIGH);
    }

    else {
      digitalWrite(LED, LOW);
    }
  }
