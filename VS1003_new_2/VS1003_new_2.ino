#include <SPI.h>
#include <SD.h>


// VS1003 SCI Write Command byte is 0x02
#define VS_WRITE_COMMAND 0x02

// VS1003 SCI Read COmmand byte is 0x03
#define VS_READ_COMMAND  0x03


const int xCs = A3;
const int xDcs = A2;
const int xDreq = 5;
const int xReset = A0;

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


void control_mode_on(void) {
  digitalWrite(xDcs, HIGH);
  digitalWrite(xCs, LOW);
}

void await_data_request(void) {
  while (!digitalRead(xDreq));
}

void control_mode_off(void) {
  digitalWrite(xCs, HIGH);
}

uint16_t read_register_my_own(uint8_t _reg) {
  while (!digitalRead(xDreq));
  digitalWrite(xCs, LOW);
  SPI.transfer(VS_READ_COMMAND);
  SPI.transfer(_reg);
  unsigned char response1 = SPI.transfer(0xFF);
  unsigned char response2 = SPI.transfer(0xFF);
  digitalWrite(xCs, HIGH);
  return ((unsigned int) response1 << 8) | (response2 & 0xFF);
}

void write_register(uint8_t _reg, uint16_t _value) {
  control_mode_on();
  delayMicroseconds(1); // tXCSS
  SPI.transfer(VS_WRITE_COMMAND); // Write operation
  SPI.transfer(_reg); // Which register
  SPI.transfer(_value >> 8); // Send hi byte
  SPI.transfer(_value & 0xff); // Send lo byte
  delayMicroseconds(1); // tXCSH
  await_data_request();
  control_mode_off();
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
  write_register(SCI_VOL, value);
}











const unsigned char header[] = {
0x52, 0x49, 0x46, 0x46, 0x1c, 0x10, 0x00, 0x00,
0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20, /*|RIFF....WAVEfmt |*/
0x14, 0x00, 0x00, 0x00, 0x11, 0x00, 0x01, 0x00,
0x40, 0x1f, 0x00, 0x00, 0x75, 0x12, 0x00, 0x00, /*|........@......|*/
0x00, 0x01, 0x04, 0x00, 0x02, 0x00, 0xf9, 0x01,
0x66, 0x61, 0x63, 0x74, 0x04, 0x00, 0x00, 0x00, /*|.......fact....|*/
0x5c, 0x1f, 0x00, 0x00, 0x64, 0x61, 0x74, 0x61,
0xe8, 0x0f, 0x00, 0x00
};
unsigned char db[512];  // data buffer for saving to disk




void RecordAdpcm1003(void) {
  uint16_t w = 0, idx = 0;
  setVolume(200); // Recording monitor volume
  write_register(SCI_BASS, 0); // Bass/treble disabled
  write_register(SCI_CLOCKF, 0x4430); // 2.0x 12.288MHz
  delay(100);
  write_register(SCI_AICTRL0, 12); // Div -> 12=8kHz 8=12kHz 6=16kHz
  delay(100);
  write_register(SCI_AICTRL1, 0); // Auto gain
  delay(100);
  if (1) {
    write_register(SCI_MODE, 0x5804); // Normal SW reset + other bits
  } else {
    write_register(SCI_MODE, 0x1804); // Normal SW reset + other bits
  }
  // Save Header
  for (idx = 0; idx < sizeof(header); idx++) {
    db[idx] = header[idx];
  }
  // Get Records loop 
  while (1) {
    do {
      w = read_register_my_own(SCI_HDAT1);
      Serial.println(w);
    } while (w < 256 || w >= 896); // wait until 512 bytes available
    while (idx < 512) {
      w = read_register_my_own(SCI_HDAT0);
      db[idx++] = w >> 8;
      db[idx++] = w & 0xFF;
    }
    idx = 0;
    // Save db Array to a file.mp3
  }
  //ResetMP3(); // Normal reset, restore default settings
  setVolume(200);
}




























void setup() {
  // Serial Begin
  Serial.begin(115200);
  // VS1003 Begin
  pinMode(xReset, OUTPUT);
  pinMode(xCs, OUTPUT);
  pinMode(xDcs, OUTPUT);
  pinMode(xDreq, INPUT);
  digitalWrite(xReset, LOW);
  digitalWrite(xCs, HIGH);
  digitalWrite(xDcs, HIGH);
  digitalWrite(xReset, HIGH);
  // initiate SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
  write_register(SCI_VOL, 200);
  write_register(SCI_BASS, 0);
  write_register(SCI_CLOCKF, 0x4430);
  write_register(SCI_AICTRL0, 12);
  /*
  delay(100);
  write_register(SCI_AICTRL1, 200);
  delay(100);
  write_register(SCI_MODE, 0x1804);
  SPI.endTransaction();
  */
}
















uint16_t t, w;


void loop() {

  RecordAdpcm1003();
  //activating_ADPCM();
  //Serial.println(read_register_my_own(SCI_HDAT1)>>8);
  //Serial.println(read_register_my_own(SCI_HDAT0)>>8);

}
