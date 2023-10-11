#include <math.h>                 // Sensor Temp
#include <Wire.h>                 // LCD
#include "rgb_lcd.h"              // LCD

int B = 4275;               // B value of the thermistor - Sensor Temp
int R0 = 100000;            // R0 = 100k - Sensor Temp
int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0 - Sensor Temp
int pinSensorSolo = A1;     // Usando o valor 15 para A1
int pinoRele = 5;           // Exemplo de pino digital para o relé 


float parametroBaixaUmidade = 0.02;  // Usaremos este parametro para dizer que a terra precisa ser irrigada
bool bombaLigada = false;                  // Variável de estado para controlar se a bomba está ligada ou não
bool exibirMensagemBombaLigada = false; // Variável para controlar a exibição da mensagem "Bomba Ligada"


rgb_lcd lcd;                               // LCD

byte ventilacaoIcone[8] = {
  B00100,
  B01010,
  B01010,
  B11111,
  B00100,
  B00100,
  B11111,
  B00000
};  

float LerTemperatura() {
  int a = analogRead(pinTempSensor);
  float R = 1023.0 / a - 1.0;
  R = R0 * R;
  float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet
  return temperature;
}

float LerUmidade() {
  int leitura = analogRead(pinSensorSolo);
  float humidity = map(leitura, 0, 1023, 0, 100); // Mapeia a le itura para uma escala de 0 a 100%
  return humidity;
}

bool VerificarBaixaUmidade(float umidade) {
  if (umidade < parametroBaixaUmidade) {
    return true;
  } else {
    return false;
  }
}
void ExibirIconeTemperaturaNoLCD(float temperatura) {
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("Ativando ventilação");
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); // Exibe o ícone de ventilação personalizado
  
}

void VerificarAltaTemperatura(float temperatura) {
  if (temperatura > 20.0) { // Defina o limite de alta temperatura conforme necessário
    // Ativar ventilação
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print("Ativando ventilação");
    lcd.setCursor(0, 1);
    lcd.write(byte(0)); // Exibe o ícone de ventilação personalizado
  } else {
    // Desativar ventilação
    lcd.setCursor(0, 0);
    lcd.print("Temperatura normal");
    lcd.setCursor(0, 1);
    lcd.print("                "); // Limpa o ícone de ventilação
  }
}


void ExibirTemperaturaUmidadeNoLCD(float temperatura, float umidade) {
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.setCursor(6, 0);            // Set cursor to the first line
  lcd.print(temperatura, 1);      // Display temperature with 2 decimal places
  lcd.print("C");                 // Exibe a unidade de porcentagem
  lcd.setCursor(10, 1);           // Set cursor to the first line
  lcd.print(umidade, 1);          // Display humidity with 2 decimal places
  lcd.print("%");                 // Exibe a unidade de porcentagem
}
void ExibirDesligamentoBombaNoLCD(){
  lcd.clear();
  lcd.setRGB(100, 100, 100); //cor laranja
  lcd.setCursor(0, 0);
  lcd.print("DESATIAVANDO");
  lcd.setCursor(0, 1);
  lcd.print("   BOMBA   ");
    
  delay(2000); // Aguarda 2 segundos para exibir a mensagem de desativação
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    BOMBA");
  lcd.setCursor(0, 1);
  lcd.print("    DESLIGADA");

  delay(2000);
  lcd.clear();

}
void ExibirInicializacaoBombaNoLCD(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setRGB(0, 255, 0);   // verde   
  lcd.print("    ATIAVANDO");
  lcd.setCursor(0, 1);
  lcd.print("    BOMBA");
  delay(2000); 

  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("    BOMBA");
  lcd.setCursor(0, 1);
  lcd.print("    LIGADA");
  delay(2000);
  lcd.clear();
}


void AtivarRele() {
  digitalWrite(pinoRele, HIGH); // Define o pino do relé como alto (liga o relé)
}

void DesativarRele() {
  digitalWrite(pinoRele, LOW); // Define o pino do relé como baixo (desliga o relé)
}

void AtivarDesativarBomba(bool umidadeBaixa) {
  if (umidadeBaixa && !bombaLigada) {
    
    bombaLigada = true;

    exibirMensagemBombaLigada = true;  

    AtivarRele();
    
    
  } else if (!umidadeBaixa && bombaLigada) {
    bombaLigada = false;

    ExibirDesligamentoBombaNoLCD();

    DesativarRele();
  }
}
void ExibirNoLCDIconeBomba(){
  lcd.setCursor(13, 0);
  lcd.write(byte(0)); // Exibe o ícone de ventilação personalizado
}
void setup() {
  pinMode(pinTempSensor, INPUT);    // sets the digital pin 13 as output
  pinMode(pinSensorSolo, INPUT);    // sets the digital pin 13 as output
  pinMode(pinoRele, OUTPUT);      // Configura o pino do relé como saída

  lcd.begin(0x3E, 0x62);
  lcd.setRGB(100, 100, 100);

  lcd.createChar(0, ventilacaoIcone); // Crie o ícone de ventilação personalizado
  
  Serial.println("Digite um novo valor para parametroBaixaUmidade");


  Serial.begin(9600);
}

void loop() {
  float temperatura = LerTemperatura();
  float umidade = LerUmidade();
  
  bool umidadeBaixa = VerificarBaixaUmidade(umidade);
  
  AtivarDesativarBomba(umidadeBaixa);                    //Ativa ou não a bomba de agua/relé
  
  if (!bombaLigada) {
    ExibirTemperaturaUmidadeNoLCD(temperatura, umidade); // Se a bomba não estiver ligada, exiba as informações de temperatura e umidade

  } else {
    if (exibirMensagemBombaLigada) {
      ExibirInicializacaoBombaNoLCD();
      ExibirNoLCDIconeBomba();                            // Se a bomba estiver ligada e a mensagem "Bomba Ligada" deve ser exibida, faça isso

      exibirMensagemBombaLigada = false;                 // Defina a variável para que a mensagem não seja exibida novamente
    } else {
      // Se a bomba estiver ligada, mas a mensagem não deve ser exibida, exiba apenas o ícone de ventilação
      
      delay(20);
    }
      ExibirTemperaturaUmidadeNoLCD(temperatura, umidade);

  }

  if (Serial.available() > 0) {
    // Leia o valor float da entrada serial
    parametroBaixaUmidade = Serial.parseFloat();

    // Aguarde até que a entrada serial seja consumida completamente
    while (Serial.available() > 0) {
      Serial.read();
    }

    // Imprima o novo valor
    Serial.print("Novo valor para parametroBaixaUmidade: ");
    Serial.println(parametroBaixaUmidade);
  }


  delay(5000);    //5sec

}
