
// Para inicializar la placa, configurar el procesador 
// Tools-> Processor -> ATMega 328P (Old Bootlader)

#include <SPI.h>  //Comunicación con el NFC mediante el protocolo SPI
#include <MFRC522.h>  //Lectura y escritura RFID

// Crear instancia de la clase MFRC522 con los dos pines
#define SS_PIN 10 
#define RST_PIN 5 
MFRC522 mfrc522(SS_PIN, RST_PIN);  

// Llave de autenticación (por defecto de fábrica)
MFRC522::MIFARE_Key key;
byte keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void readUID(byte numsis){
  Serial.print("UID de la tarjeta: ");
      
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], numsis);
    Serial.print(" ");
  }
  Serial.println();
}

void readBlock(byte* arquetipos, byte numsis, int block){
  Serial.print(F("Datos bloque "));
  Serial.print(block);
  Serial.print(F(": "));

  for (byte i = 0; i < numsis; i++) {
    if(numsis == HEX && arquetipos[i] < 0x10) Serial.print("0");
    Serial.print(arquetipos[i], numsis);
    Serial.print(" ");
  }
  Serial.println();
}

void overwrite(byte block, byte* arquetipos){
  // Números que quieres de arquetipos del 0 al 200. Los 6 últimos serán de 0.
  byte ow[16] = {195, 2, 3, 6, 1, 2, 3, 4, 8, 9, 0, 0, 0, 0, 0, 0};
  check0200(ow);
  for (byte i = 0; i < 16; i++) {
    arquetipos[i] = ow[i];
  }
  byte status = mfrc522.MIFARE_Write(block, arquetipos, 16);
  if (status == MFRC522::STATUS_OK) {
    Serial.println("Bloque 4 escrito correctamente con el array ow!");
  } else {
    Serial.println("Error al escribir en el bloque 4.");
  }
}

void sum(byte* arquetipos, byte num, byte arq, byte block){
  arquetipos[arq] = arquetipos[arq] + num;
  check0200(arquetipos);
  byte status = mfrc522.MIFARE_Write(block, arquetipos, 16);
  if (status == MFRC522::STATUS_OK) {
    Serial.println("Suma hecha");
  } else {
    Serial.println("Error en la suma");
  }
}

void check0200(byte* arquetipos){
  for (byte i = 0; i < sizeof(arquetipos); i++) {
    if (arquetipos[i] > 200){
      arquetipos[i]=200;
    }else if (arquetipos[i] < 0){
      arquetipos[i]=0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin();                    // Inicia la comunicación SPI
  mfrc522.PCD_Init();             // Inicializa el lector MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = keyA[i];
  }
  
  Serial.println("Escanea un tag NFC...");
}

void loop() {
  // Verifica si hay una tarjeta NFC cerca del lector
  if (mfrc522.PICC_IsNewCardPresent()) {
    // Si hay una tarjeta, lee la tarjeta NFC
    if (mfrc522.PICC_ReadCardSerial()) {
      
      // Leer el UID
      readUID(HEX); //En Hexadecimal

      byte block = 4;
      byte arquetipos[18]; // Cada bloque tiene 16 bytes + 2 del checksum()
      byte size = sizeof(arquetipos);
      byte arq = 2; //Qué arquetipo quieres modificar
      byte num = 10; // Número a sumar o a restar

      // Autenticar con la tarjeta
      byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));

      // Comprobar la autentificación
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Autentificación fallida: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        return;
      }

      // Leer el bloque
      status = mfrc522.MIFARE_Read(block, arquetipos, &size);
      if (status == MFRC522::STATUS_OK) {
        // Mostrar datos del bloque
        readBlock(arquetipos, DEC, block);
        // Sobreescribir el bloque
        // overwrite(block, arquetipos);
        // Sumar valores
        sum(arquetipos, num, arq, block);
        // Validar que los números de arquetipo vayan del 0 al 200
        check0200(arquetipos);
      } else {
          Serial.println("Error al leer el bloque.");
          Serial.println(mfrc522.GetStatusCodeName(status));
      }

      // Detener la comunicación con la tarjeta
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  }
}
