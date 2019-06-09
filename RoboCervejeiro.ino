#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#define ONE_WIRE_BUS 3

OneWire oneWire(ONE_WIRE_BUS);

//Inicializa sensor de temperatura
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// Inicializa o LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Inicializa o pino do rele
int pinoRele = 2;

void setup() {
  Serial.begin(9600);
  sensors.begin();
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0))
    Serial.println("Sensores nao encontrados !");
  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1);
  Serial.println();
  Serial.println();

  pinMode(pinoRele, OUTPUT);
  digitalWrite(pinoRele, HIGH);// Inicia o Rele desligado

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Inicializando sistema");
  lcd.setCursor(0, 1);
}
void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
int define_rampas() {
  int rampas = 0;
  lcd.setCursor(0, 0);
  lcd.print("Defina rampas");
  lcd.setCursor(0, 1);
  lcd.print("Rampas: ");
  int botao;
  bool confirmouRampas = false;  lcd.setCursor(8, 1);

  lcd.print(String(rampas));
  do {
    delay(100);
    botao = analogRead(0);
    //Direita
    if (botao < 80) {
      lcd.setCursor(8, 1);
      lcd.print(String(rampas));
    }
    //Cima
    else if (botao < 200) {
      rampas++;
      lcd.setCursor(8, 1);
      lcd.print(String(rampas));
    }
    //Baixo
    else if (botao < 300) {
      rampas--;
      lcd.setCursor(8, 1);
      lcd.print(String(rampas));
    }
    //Esquerda
    else if (botao < 500) {
      lcd.setCursor(8, 1);
      lcd.print(String(rampas));
    }
    else if (botao < 800) {
      confirmouRampas = true;
      lcd.setCursor(8, 1);
      lcd.print(String(rampas));
    }
  } while (!confirmouRampas);
  return rampas;
}

int defineMinutosFervura() {
  Serial.println("Definir minutos de fervura");
  int minFervura = 60;
  bool fervuraConfirmada = false;
  lcd.clear();
  do {

    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("tempo fervura");
    int botao;
    botao = analogRead(0);
    //Direita
    if (botao < 80) {
      lcd.setCursor(0, 1);
      lcd.print(String(minFervura));
    }
    //Cima
    else if (botao < 200) {
      minFervura++;
      lcd.setCursor(0, 1);
      lcd.print(String(minFervura));
    }
    //Baixo
    else if (botao < 300) {
      minFervura--;
      lcd.setCursor(0, 1);
      lcd.print(String(minFervura));
    }
    //Esquerda
    else if (botao < 500) {
      lcd.setCursor(0, 1);
      lcd.print(String(minFervura));
    }
    else if (botao < 800) {
      fervuraConfirmada = true;
      lcd.setCursor(0, 1);
      lcd.print(String(minFervura));
    }
    Serial.print("Minutos de fervura");
    Serial.println(minFervura);
  } while (!fervuraConfirmada);
}


bool ligaResistencia() {
  Serial.println("Resistencia ligada");
  digitalWrite(pinoRele, LOW);
  return true;
}

bool desligaResistencia() {
  Serial.println("Resistencia desligada");
  digitalWrite(pinoRele, HIGH);
  return false;
}

void iniciaBrassagem(int rampas, int temperaturas[], int minutosRampas[], int minutosFervura) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando brassagem");
  bool resistenciaLigada = false;
  delay(500);
  for (int i = 0; i < rampas; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(i + 1));
    lcd.print("a Rampa");
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(sensor1);
    Serial.print("Temperatura atual ");
    Serial.println(tempC);

    Serial.print("Temperatura alvo ");
    Serial.println(temperaturas[i]);

    if (tempC < temperaturas[i] && !resistenciaLigada) {
      resistenciaLigada = ligaResistencia();
    }
    lcd.clear();
    do {
      lcd.clear();
      Serial.println("Loop");
      sensors.requestTemperatures();
      tempC = sensors.getTempC(sensor1);
      Serial.print("TEMP");
      Serial.println(tempC);
      lcd.setCursor(0, 1);
      lcd.print(String(tempC));
      lcd.write(233);
      lcd.print("C alvo: ");
      lcd.print(temperaturas[i]);
      delay(3000);
    } while (tempC < temperaturas[i]);

    int tempoAtual = 0;

    do {
      delay(1000);
      tempoAtual++;
      Serial.print("Tempo atual ");
      Serial.println(tempoAtual);

      Serial.print("Tempo alvo ");
      Serial.println(minutosRampas[i] * 60);

      sensors.requestTemperatures();
      tempC = sensors.getTempC(sensor1);

      if (tempC > temperaturas[i] + 1 && resistenciaLigada) {
        resistenciaLigada = desligaResistencia();
      }

      if (tempC < temperaturas[i] - 1 && !resistenciaLigada) {
        resistenciaLigada = ligaResistencia();
      }

      lcd.setCursor(0, 1);
      lcd.print("Temp: ");
      lcd.print(tempC);
      lcd.print(" C");

    } while (tempoAtual < (minutosRampas[i] * 60) );

    lcd.setCursor(0, 0);
    lcd.print("Fim da rampa              ");
    delay(1000);
  }
  desligaResistencia();
  lcd.setCursor(0, 0);
  lcd.print("Fim brassagem");
  lcd.setCursor(0, 1);
  lcd.print("Retire os graos");
  bool graosRetirados = false;
  do {
    int botao;
    botao = analogRead(0);
    if (botao > 500 and botao < 800) {
      graosRetirados = true;
    }
  } while (!graosRetirados);

  lcd.setCursor(0, 0);
  lcd.print("Iniciando fervura");
  ligaResistencia();
  delay(2000);

  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensor1);

  do {
    sensors.requestTemperatures();
    tempC = sensors.getTempC(sensor1);
  } while (tempC < 96);

  int tempoAtualFervura = 0;
  do {
    delay(1000);
    tempoAtualFervura ++;
  } while (tempoAtualFervura < (minutosFervura * 3600));

}
void loop() {
  lcd.clear();
  int rampas = 0;
  rampas = define_rampas();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Serao feitas ");
  lcd.print(String(rampas));
  lcd.setCursor(0, 1);
  lcd.print("rampas");
  delay(2000);

  lcd.clear();
  int* arrTempRampas = new int [rampas];
  int ultimaTemperatura = 0;

  for (int i = 0; i < rampas; i++) {
    Serial.print("rampa ");
    int r = i + 1;
    Serial.println(r);
    Serial.print("Valor");
    Serial.println(arrTempRampas[i]);
    bool rampaTemperaturaConfirmada = false;
    arrTempRampas[i] = ultimaTemperatura;
    do {
      delay(200);
      lcd.setCursor(0, 0);
      lcd.print("temp. rampa ");
      lcd.setCursor(13, 0);
      lcd.print(r);
      int botao;
      botao = analogRead(0);
      //Direita
      if (botao < 80) {
        lcd.setCursor(0, 1);
        lcd.print(String(arrTempRampas[i]));
      }
      //Cima
      else if (botao < 200) {
        arrTempRampas[i]++;
        lcd.setCursor(0, 1);
        lcd.print(String(arrTempRampas[i]));
      }
      //Baixo
      else if (botao < 300) {
        arrTempRampas[i]--;
        lcd.setCursor(0, 1);
        lcd.print(String(arrTempRampas[i]));
      }
      //Esquerda
      else if (botao < 500) {
        lcd.setCursor(0, 1);
        lcd.print(String(arrTempRampas[i]));
      }
      else if (botao < 800) {
        rampaTemperaturaConfirmada = true;
        lcd.setCursor(0, 1);
        lcd.print(String(arrTempRampas[i]));
      }
      if (arrTempRampas[i] < 10) {
        lcd.print(" ");
      }
      lcd.setCursor(10, 1);
      lcd.write(223);
      lcd.print("C");
      Serial.print("Valor atualizado");
      Serial.println(arrTempRampas[i]);
    } while (!rampaTemperaturaConfirmada);
    ultimaTemperatura = arrTempRampas[i];
    lcd.clear();
  }

  for (int i = 0; i < rampas; i++) {
    Serial.print("Temperatura rampa ");
    int r = i + 1;
    Serial.println(r);
    Serial.println(arrTempRampas[i]);
  }

  lcd.clear();
  int* arrMinutosRampas = new int [rampas];
  for (int i = 0; i < rampas; i++) {
    Serial.println("Minutos");
    Serial.print("rampa ");
    int r = i + 1;
    Serial.println(r);
    Serial.print("Valor ");
    Serial.println(arrMinutosRampas[i]);
    bool rampaMinutosConfirmada = false;
    arrMinutosRampas[i] = 0;
    do {

      delay(200);
      lcd.setCursor(0, 0);
      lcd.print("min. rampa ");
      lcd.setCursor(12, 0);
      lcd.print(r);
      int botao;
      botao = analogRead(0);
      //Direita
      if (botao < 80) {
        lcd.setCursor(0, 1);
        lcd.print(String(arrMinutosRampas[i]));
      }
      //Cima
      else if (botao < 200) {
        arrMinutosRampas[i]++;
        lcd.setCursor(0, 1);
        lcd.print(String(arrMinutosRampas[i]));
      }
      //Baixo
      else if (botao < 300) {
        arrMinutosRampas[i]--;
        lcd.setCursor(0, 1);
        lcd.print(String(arrMinutosRampas[i]));
      }
      //Esquerda
      else if (botao < 500) {
        lcd.setCursor(0, 1);
        lcd.print(String(arrMinutosRampas[i]));
      }
      else if (botao < 800) {
        rampaMinutosConfirmada = true;
        lcd.setCursor(0, 1);
        lcd.print(String(arrMinutosRampas[i]));
      }
      if (arrMinutosRampas[i] < 10) {
        lcd.print(" ");
      }
      lcd.setCursor(10, 1);
      lcd.print("MIN");
      Serial.print("Valor atualizado");
      Serial.println(arrMinutosRampas[i]);
    } while (!rampaMinutosConfirmada);
    lcd.clear();
  }

  int tempoFervura = defineMinutosFervura();
  Serial.print("Tempo de fervura ");
  Serial.println(tempoFervura);


  iniciaBrassagem(rampas, arrTempRampas, arrMinutosRampas, tempoFervura);

  lcd.setCursor(0, 0);
  lcd.print("Fim da brassagem     ");
  lcd.setCursor(0, 1);
  lcd.print("Boa breja ;D");
  do {

  } while (1 == 1);
}
