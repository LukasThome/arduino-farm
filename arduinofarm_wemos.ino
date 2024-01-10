#include <math.h>
#include <Wire.h>
#include <rgb_lcd.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>  // Make sure to include the ArduinoJson library



const char *ssid = "Hallo2s";
const char *password = "otyt0582";


int B = 4275;
int R0 = 100000;
int pinTempSensor = D0;
int pinSensorSolo = A0;
int pinoRele = D5;  // Usar a pinagem correta para o relé no Wemos D1 R1 (D1, D2, etc.)

float parametroBaixaUmidade = 0;
bool bombaLigada = false;
bool exibirMensagemBombaLigada = false;

rgb_lcd lcd;

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
  float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
  return temperature;
}

float LerUmidade() {
  int leitura = analogRead(pinSensorSolo);
  float humidity = map(leitura, 0, 1023, 0, 100);
  return humidity;
}

bool VerificarBaixaUmidade(float umidade) {
  return umidade < parametroBaixaUmidade;
}


void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


void enviarLeitura(float valor) {
  HTTPClient http;

  String fullUrl = "http://192.168.58.51:8090/receber-leitura" ;

  WiFiClient client;
  http.begin(client, fullUrl);

  // Building do JSON payload
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["valor"] = valor;
  Serial.println("Criou o Json document");


  // Serialize JSON -> String
  String jsonPayload;
  serializeJson(jsonDocument, jsonPayload);

  http.addHeader("Content-Type", "application/json");

  // Realiza o POST
  int httpCode = http.POST(jsonPayload);

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Leitura enviada com sucesso");
  } else {
    Serial.print("Erro no envio da leitura. Código HTTP: ");
    Serial.println(httpCode);
  }


  http.end();
}


void atualizarValorUmidade() {
  HTTPClient http;

  // Update the server URL to the appropriate endpoint
  String fullUrl = "http://192.168.58.51:8000/Config/ler-umidade";

  WiFiClient client;
  http.begin(client, fullUrl);

  // Realiza o GET
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String resposta = http.getString();

    // Parse the JSON response
    DynamicJsonDocument jsonDocument(200);
    DeserializationError error = deserializeJson(jsonDocument, resposta);

    if (error) {
        Serial.print("Erro ao analisar JSON: ");
        Serial.println(error.c_str());
    } else {
        // Extract the "umidade" value from the JSON object
        float novoValor = jsonDocument["umidade"].as<float>();

        Serial.print("Novo valor para parametroBaixaUmidade: ");
        Serial.println(novoValor);

        parametroBaixaUmidade = novoValor;  // Update the global variable
    }

  } else {
    Serial.print("Erro na atualizacao da umidade. Código HTTP: ");
    Serial.println(httpCode);
  }

  http.end();
}



void ExibirIconeTemperaturaNoLCD(float temperatura) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ativando ventilação");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
}

void VerificarAltaTemperatura(float temperatura) {
  if (temperatura > 20.0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ativando ventilação");
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temperatura normal");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
}

void ExibirTemperaturaUmidadeNoLCD(float temperatura, float umidade) {
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.setCursor(6, 0);
  lcd.print(temperatura, 1);
  lcd.print("C");
  lcd.setCursor(10, 1);
  lcd.print(umidade, 1);
  lcd.print("%");
}

void ExibirDesligamentoBombaNoLCD() {
  lcd.clear();
  lcd.setRGB(100, 100, 100);
  lcd.setCursor(0, 0);
  lcd.print("DESATIVANDO");
  lcd.setCursor(0, 1);
  lcd.print("   BOMBA   ");

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    BOMBA");
  lcd.setCursor(0, 1);
  lcd.print("    DESLIGADA");

  delay(2000);
  lcd.clear();
}

void ExibirInicializacaoBombaNoLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setRGB(0, 255, 0);
  lcd.print("    ATIVANDO");
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
  digitalWrite(pinoRele, HIGH);
}

void DesativarRele() {
  digitalWrite(pinoRele, LOW);
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

void ExibirNoLCDIconeBomba() {
  lcd.setCursor(13, 0);
  lcd.write(byte(0));
}

void setup() {
  pinMode(pinTempSensor, INPUT);
  pinMode(pinSensorSolo, INPUT);
  pinMode(pinoRele, OUTPUT);

  lcd.begin(0x3E, 0x62);

  lcd.begin(16, 2);  
  lcd.setRGB(100, 100, 100);
  lcd.createChar(0, ventilacaoIcone);

  Serial.begin(9600);

  setupWiFi();

}

void loop() {
  float temperatura = LerTemperatura();
  float umidade = LerUmidade();

  bool umidadeBaixa = VerificarBaixaUmidade(umidade);

  AtivarDesativarBomba(umidadeBaixa);

  if (!bombaLigada) {
    ExibirTemperaturaUmidadeNoLCD(temperatura, umidade);

  } else {
    if (exibirMensagemBombaLigada) {
      ExibirInicializacaoBombaNoLCD();
      ExibirNoLCDIconeBomba();
      exibirMensagemBombaLigada = false;
    } else {
      delay(20);
    }
    ExibirTemperaturaUmidadeNoLCD(temperatura, umidade);
  }

  if (Serial.available() > 0) {
    parametroBaixaUmidade = Serial.parseFloat();
    while (Serial.available() > 0) {
      Serial.read();
    }
    Serial.print("Novo valor para parametroBaixaUmidade: ");
    Serial.println(parametroBaixaUmidade);
    atualizarValorUmidade();


  }

  delay(5000);
  enviarLeitura(umidade); // Send temperature reading
  
  atualizarValorUmidade();
  Serial.println(parametroBaixaUmidade);


}
