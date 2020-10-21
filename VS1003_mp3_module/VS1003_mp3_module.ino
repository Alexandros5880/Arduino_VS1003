#include <SD.h>
#include <SPI.h>

// FOR SD TEST
File root;
int i = 0;
int fileNumber = 0;

// SET PINS VS1003
int xCs = 9;
int xResete = 8;
int dreq = 7;
int xDcs = 6;

// SD SS
int sd_sspin = 4;


int volume = 0x30;
int DREQ = digitalRead(dreq);

char * playlist[] = {};

File mp3;






void setup() {
  delay(10);
  // SET 1003
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  pinMode(dreq, INPUT);
  pinMode(xResete, OUTPUT);
  pinMode(xDcs, OUTPUT);
  pinMode(xCs, OUTPUT);
  digitalWrite(sd_sspin, HIGH);
  Mp3Resete();
  // Serial Setup
  Serial.begin(115200);
  // SD Setup
  Serial.println("Set SD card...");
  while(! SD.begin(sd_sspin)) { }
  Serial.println("SD Ready");
  root = SD.open("/");
  printDirectory(root, 0);
  Serial.print("Totally ");
  Serial.print(fileNumber);
  Serial.print(" files(s)");
  Serial.println();
}



void loop() {
  play("mp3.mp3");
}





void play(char * playplay) {
  int val , i;
  mp3 = SD.open(playplay);
  delay(10);
  for ( i = 0; i < 548; i++) {
    digitalWrite(sd_sspin, HIGH);
    val = mp3.read();
    digitalWrite(xDcs, LOW);
    SPI.transfer(val);
    digitalWrite(xDcs, HIGH);
    digitalWrite(sd_sspin, LOW);
  }
  if ( DREQ == HIGH) {
    while (mp3.available()) {
      for ( int i = 0; i < 32; i++) {
        digitalWrite(sd_sspin, HIGH);
        val = mp3.read();
        digitalWrite(xDcs, LOW);
        SPI.transfer(val);
        digitalWrite(xDcs, HIGH);
        digitalWrite(sd_sspin, LOW);
      }
      delayMicroseconds(35);
    }
  }
  mp3.close();
}


void Mp3Resete() {
  digitalWrite(xResete, LOW);
  delay(100);
  digitalWrite(xCs, HIGH);
  digitalWrite(xDcs, HIGH);
  digitalWrite(xResete, HIGH);
  commad(0x00,0x08,0x04); // write into mode
  delay(10);
  if ( DREQ == HIGH ) {
    commad(0x03, 0xc0, 0x00); // set vs1003 clock
    delay(10);
    commad(0x05, 0xbb, 0x81); // set vs1003 sample rate at 44kps stero
    delay(10);
    commad(0x02, 0x00, 0x55); // set dublicate sound
    delay(10);
    commad(0x0b, volume, volume); // highest volum at 0x0000, and lowest at 0xfefe
    delay(10);
    SPI.transfer(0);
    SPI.transfer(0);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(xCs, HIGH);
    digitalWrite(xResete, HIGH);
    digitalWrite(xDcs, HIGH);
    digitalWrite(sd_sspin, LOW);
  }
}


void commad(unsigned char addr, unsigned char hdat, unsigned char ldat) {
  int DREQ = digitalRead(dreq);
  if ( DREQ == HIGH ) {
    digitalWrite(xCs, LOW);
    SPI.transfer(0x02);
    SPI.transfer(addr);
    SPI.transfer(hdat);
    SPI.transfer(ldat);
    digitalWrite(xCs, HIGH);
  }
}


void printDirectory(File dir, int numTabs) {
  while(i < 6) {
    File entry = dir.openNextFile();
    if (! entry) {
      i++;
    }
    for ( uint8_t i = 0; i < numTabs; i++ ) {
      Serial.println('\t');
    }
    Serial.println(entry.name());
    if ( entry.isDirectory() ) {
      Serial.println("/");
      printDirectory(entry, numTabs+1);
    }
    else if ( entry.size() ) {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      fileNumber++;
    }
  }
}
