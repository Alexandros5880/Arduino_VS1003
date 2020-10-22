// STL headers
// C headers
#include <avr/pgmspace.h>
// Framework headers
// Library headers
#include <SPI.h>
#include <SD.h>







#define XDCS 6
#define DREQ 5
#define XRES 7
#define XCS  3
#define SS   4



File mp3;

const uint8_t vs1003_chunk_size = 32;


// VS1003 SCI Write Command byte is 0x02
#define VS_WRITE_COMMAND 0x02

// VS1003 SCI Read COmmand byte is 0x03
#define VS_READ_COMMAND  0x03

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







#define MP3bufferLength 32
uint8_t mp3Data[MP3bufferLength];















void VS1003_await_data_request(void) {
  while ( !digitalRead(DREQ) );
}

void VS1003_control_mode_on(void) {
  //digitalWrite(XDCS,HIGH);
  digitalWrite(XCS, LOW);
}

void VS1003_control_mode_off(void) {
  digitalWrite(XCS, HIGH);
}

void VS1003_data_mode_on(void) {
  //digitalWrite(XCS,HIGH);
  digitalWrite(XDCS, LOW);
}

void VS1003_data_mode_off(void) {
  digitalWrite(XDCS, HIGH);
}




void VS1003_begin(void) {
  pinMode(SS, OUTPUT);
  digitalWrite (SS, HIGH);  // ss - HIGH
  if (!SD.begin(SS)) {
    Serial.println("SD doesn't begines");
  }
  pinMode(XRES, OUTPUT);
  digitalWrite(XRES, LOW);
  pinMode(XCS, OUTPUT);
  digitalWrite(XCS, HIGH);
  pinMode(XDCS, OUTPUT);
  digitalWrite(XDCS, HIGH);
  pinMode(DREQ, INPUT);
  delay(1);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64); // Slow!
  digitalWrite(XRES, HIGH);
  VS1003_await_data_request();
  VS1003_write_register(SCI_AUDATA, 44101); // 44.1kHz stereo
  VS1003_write_register(SCI_MODE, _BV(SM_SDINEW) | _BV(SM_RESET));
  VS1003_write_register(SCI_CLOCKF, 0xE800); // Experimenting with higher clock settings
  SPI.setClockDivider(SPI_CLOCK_DIV4); // Fastest available
}




void VS1003_write_register(uint8_t _reg, uint16_t _value) {
  VS1003_control_mode_on();
  delayMicroseconds(1); // tXCSS
  SPI.transfer(VS_WRITE_COMMAND); // Write operation
  SPI.transfer(_reg); // Which register
  SPI.transfer(_value >> 8); // Send hi byte
  SPI.transfer(_value & 0xff); // Send lo byte
  delayMicroseconds(1); // tXCSH
  VS1003_await_data_request();
  VS1003_control_mode_off();
}





uint16_t VS1003_read_register(uint8_t _reg) {
  uint16_t result;
  VS1003_control_mode_on();
  delayMicroseconds(1); // tXCSS
  SPI.transfer(VS_READ_COMMAND); // Read operation
  SPI.transfer(_reg); // Which register
  result = SPI.transfer(0xff) << 8; // read high byte
  result |= SPI.transfer(0xff); // read low byte
  delayMicroseconds(1); // tXCSH
  VS1003_await_data_request();
  VS1003_control_mode_off();
  return result;
}





void VS1003_setVolume(uint8_t vol) {
  uint16_t value = vol;
  value <<= 8;
  value |= vol;
  VS1003_write_register(SCI_VOL, value);
}




void VS1003_sdi_mic(void) {
  digitalWrite(XRES, LOW);
  digitalWrite(XCS, HIGH);
  digitalWrite(XDCS, HIGH);
  digitalWrite(XRES, HIGH);
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  VS1003_write_register(SCI_VOL, 0x0000);
  VS1003_write_register(SCI_BASS, 0);
  VS1003_write_register(SCI_CLOCKF, 0x4430);
  VS1003_write_register(SCI_AICTRL0, 12);
  // Rec level max: 200
  VS1003_write_register(SCI_AICTRL1, 200);
  VS1003_write_register(SCI_MODE, 0x1804);
}




void VS1003_sdi_play(const char * file, const int &volume) {
  int i = 0, val, byteCount;
  int vol = map(volume, 0, 100, 200, 0);
  mp3 = SD.open(file);
  while ( mp3.available() ) {
    VS1003_await_data_request();
    delayMicroseconds(3);
    VS1003_setVolume(vol);
    byteCount = mp3.read(mp3Data, 32);
    if (byteCount == 0) {
      mp3.close();
      break;
    }
    digitalWrite(XDCS, LOW);
    for (i = 0; i < 32; i++) {
      SPI.transfer(mp3Data[i]);
    }
    digitalWrite(XDCS, HIGH);
  }
}
























void setup() {
  Serial.begin(115200);
  VS1003_begin();
  VS1003_setVolume(1);
  //VS1003_sdi_mic();
}

void loop() {
  // put your main code here, to run repeatedly:

}
