#include <SPI.h>
#include <SD.h>





// Size of the "wav" file set to 100*512 bytes including the header files
const unsigned char RIFFHeader0fn[] = {
    'R' , 'I' , 'F' , 'F' , // Chunk ID (RIFF)
    0xF8, 0xC7, 0x00, 0x00, // Chunk payload size (calculate after rec!)
    'W' , 'A' , 'V' , 'E' , // RIFF resource format type

    'f' , 'm' , 't' , ' ' , // Chunk ID (fmt )
    0x14, 0x00, 0x00, 0x00, // Chunk payload size (0x14 = 20 bytes)
    0x11, 0x00,             // Format Tag (IMA ADPCM)
    0x01, 0x00,             // Channels (1)
    0x40, 0x1f, 0x00, 0x00, // Sample Rate, 0x3e80 = 16.0kHz
    0x75, 0x12, 0x00, 0x00, // Average Bytes Per Second
    0x00, 0x01,             // Data Block Size (256 bytes) 
    0x04, 0x00,             // ADPCM encoded bits per sample (4 bits)
    0x02, 0x00,             // Extra data (2 bytes)
    0xf9, 0x01,             // Samples per Block (505 samples)

    'f' , 'a' , 'c' , 't' , // Chunk ID (fact)
    0xc8, 0x01, 0x00, 0x00, // Chunk payload size (456 bytes (zeropad!))
    0x96, 0x86, 0x01, 0x00  // Number of Samples (calculate after rec!)
};

const unsigned char RIFFHeader504fn[] = {
    'd' , 'a' , 't' , 'a' , // Chunk ID (data)
    0x00, 0xC6, 0x00, 0x00, // Chunk payload size (calculate after rec!)
};




// VS1003
SPIClass mySPI;
// VS1003 SCI Write Command byte is 0x02
#define VS_WRITE_COMMAND 0x02
// VS1003 SCI Read COmmand byte is 0x03
#define VS_READ_COMMAND  0x03
// VS1003 PINS
const int xCs = 3; // SCI S PIN PORT SELECTOR ACTIVE LOW
const int xDcs = 6;
const int xDreq = 5;
const int xReset = 7;
// SCI Registers
const uint8_t SCI_MODE = 0x0;
const uint8_t SCI_STATUS = 0x1;
const uint8_t SCI_BASS = 0x2;
const uint8_t SCI_CLOCKF = 0x3;
const uint8_t SCI_DECODE_TIME = 0x4;
const uint8_t SCI_AUDATA = 0x5;
const uint8_t SCI_WRAM = 0x6;
const uint8_t SCI_WRAMADDR = 0x7;
const uint8_t SCI_HDAT0 = 0x8;
const uint8_t SCI_HDAT1 = 0x9;
const uint8_t SCI_AIADDR = 0xa;
const uint8_t SCI_VOL = 0xb;
const uint8_t SCI_AICTRL0 = 0xc;
const uint8_t SCI_AICTRL1 = 0xd;
const uint8_t SCI_AICTRL2 = 0xe;
const uint8_t SCI_AICTRL3 = 0xf;
const uint8_t SCI_num_registers = 0xf;
// SCI_MODE bits
const uint8_t SM_DIFF = 0;
const uint8_t SM_LAYER12 = 1;
const uint8_t SM_RESET = 2;
const uint8_t SM_OUTOFWAV = 3;
const uint8_t SM_EARSPEAKER_LO = 4;
const uint8_t SM_TESTS = 5;
const uint8_t SM_STREAM = 6;
const uint8_t SM_EARSPEAKER_HI = 7;
const uint8_t SM_DACT = 8;
const uint8_t SM_SDIORD = 9;
const uint8_t SM_SDISHARE = 10;
const uint8_t SM_SDINEW = 11;
const uint8_t SM_ADPCM = 12;
const uint8_t SM_ADCPM_HP = 13;
const uint8_t SM_LINE_IN = 14;

uint16_t t, w;
bool recordModeOn;

int iCount = 0;
long int loopCount = 0;

// SD
const int sdCs = 4;
char fileName[] = "wavFile6.WAV";
File myFile;
byte data[4];
unsigned char db[512];




// Functions Declerations
void control_mode_on(void);
void await_data_request(void);
void control_mode_off(void);
uint16_t vs1003_read(uint8_t _reg);
void vs1003_write(uint8_t _reg, uint16_t _value);
void data_mode_on(void);
void data_mode_off(void);
void setVolume(uint8_t vol);
void recording();
void updateAndCloseAudioFile();








void setup() {
  // Serial Begin
  Serial.begin(115200);
  // Keep the chip in reset until we are ready
  pinMode(xReset, OUTPUT);
  pinMode(xCs, OUTPUT);
  pinMode(xDcs, OUTPUT);
  pinMode(xDreq, INPUT); 
  pinMode(sdCs, OUTPUT);
  digitalWrite(xReset, LOW);
  digitalWrite(xCs, HIGH);
  digitalWrite(xDcs, HIGH);
  digitalWrite(xReset, HIGH);
  /*
  // SD Setup
  digitalWrite(sdCs, HIGH);
  while(!SD.begin(sdCs)) {
    Serial.println("SD initialization failed..");
  }
  Serial.println("SD initialization successfull..");
  if(SD.exists(fileName)) {
    SD.remove(fileName); // Remove the file if exists
  }
  (myFile = SD.open(fileName, FILE_WRITE)) ? Serial.println("SD opening successfull") : Serial.println("SD opening failed");
  myFile.write(RIFFHeader0fn, sizeof(RIFFHeader0fn));
  // Write '0' (0x30) from address 51 to 503 
  for (int i = 0; i<452; i++) {
    myFile.write('0');
  }
  // Write second RIFF header from 504 to 511
  myFile.write(RIFFHeader504fn, sizeof(RIFFHeader504fn));
  */
  // Initiate SPI
  mySPI.begin();
  mySPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
  vs1003_write(SCI_VOL, 0x0000);
  vs1003_write(SCI_BASS, 0);
  vs1003_write(SCI_CLOCKF, 0x4430);
  vs1003_write(SCI_AICTRL0, 12);
  delay(100);
  vs1003_write(SCI_AICTRL1, 200);
  delay(100);
  vs1003_write(SCI_MODE, 0x1804);
  mySPI.endTransaction();
  
}











void loop() {

  recording();
  
}
























void control_mode_on(void) {
  digitalWrite(xDcs, HIGH);
  digitalWrite(xCs,LOW);
}

void await_data_request(void) {
  while (!digitalRead(xDreq));
}

void control_mode_off(void) {
  digitalWrite(xCs, HIGH);
}

void data_mode_on(void) {
  digitalWrite(xCs, HIGH);
  digitalWrite(xDcs, LOW);
}

void data_mode_off(void) {
  digitalWrite(xDcs, HIGH);
}

void setVolume(uint8_t vol) {
  uint16_t value = vol;
  value <<= 8;
  value |= vol;
  vs1003_write(SCI_VOL, value); // VOL
}

uint16_t vs1003_read(uint8_t _reg) {
  while(!digitalRead(xDreq));
  digitalWrite(xCs, LOW);
  mySPI.transfer(VS_READ_COMMAND);
  mySPI.transfer(_reg);
  unsigned char response1 = mySPI.transfer(0xFF);
  unsigned char response2 = mySPI.transfer(0xFF);
  digitalWrite(xCs, HIGH);
  unsigned int res = ((unsigned int)response1 << 8) | ((unsigned int)response2 & 0xff);
  Serial.println("Response1: " + String(response1) + "  Response2: " + String(response2));
  return res;
}

void vs1003_write(uint8_t _reg, uint16_t _value) {
  control_mode_on();
  delayMicroseconds(1); // tXCSS
  mySPI.transfer(VS_WRITE_COMMAND); // Write operation
  mySPI.transfer(_reg); // Which register
  mySPI.transfer(_value >> 8); // Send hi byte
  mySPI.transfer(_value & 0xff); // Send lo byte
  delayMicroseconds(1); // tXCSH
  await_data_request();
  control_mode_off();
}







void recording() {
  mySPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
  t = vs1003_read(SCI_HDAT1);
  while(t >= 256 && t < 896) {
    for(int i=0; i<512; i=i+2) {
      w = vs1003_read(SCI_HDAT0);
      db[i] = w >> 8;
      db[i+1] = w & 0xFF;
    }
    mySPI.endTransaction();
    //myFile.write(db, sizeof(db));
    t-=256;
  }  
}




/*
void updateAndCloseAudioFile() {
  Serial.println(myFile.size());
  myFile.close();
  Serial.println("Recording off...");
  Serial.println("Closed the Audio file...");
  if(!(myFile = SD.open(fileName, O_WRITE))) {
    Serial.println("Failed to open record file in O_WRITE mode");
  }
  unsigned long int paySize1 = myFile.size() - 8;
  unsigned long int numSamp = ((myFile.size() - 512) / 256) * 505;
  unsigned long int paySize2 = myFile.size() - 512;
  data[3] = paySize1 >> 24; // shift it over so only last byte exists
  data[2] = paySize1 >> 16;
  data[1] = paySize1 >> 8;
  data[0] = paySize1;
  //    Update "RIFF" header chunk at byte 4
  myFile.seek(4);
  myFile.write(data, sizeof(data));
  data[3] = numSamp >> 24;  // shift it over so only last byte exists
  data[2] = numSamp >> 16;
  data[1] = numSamp >> 8;
  data[0] = numSamp;
  //    Update "FACT" header chunk at byte 48
  myFile.seek(48);
  myFile.write(data, sizeof(data));
  data[3] = paySize2 >> 24; // shift it over so only last byte exists
  data[2] = paySize2 >> 16;
  data[1] = paySize2 >> 8;
  data[0] = paySize2;
  //    Update "DATA" header chunk at byte 508
  myFile.seek(508);
  myFile.write(data, sizeof(data));
  Serial.println(myFile.size());
  myFile.close();
}
*/
