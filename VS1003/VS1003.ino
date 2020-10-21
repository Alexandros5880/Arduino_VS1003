#include <VS1003.h>


#define XDCS 6
#define DREQ 5
#define XRES 7
#define XCS  3
#define SS   4
 
VS1003 player(XCS, XDCS, DREQ, XRES, 4); // cs_pin, dcs_pin, dreq_pin, reset_pin, cs pin SD card module


void setup () {
    Serial.begin(115200);
    player.begin();
    player.setVolume(1);
    player.sdi_mic();
}

void loop() {
    //player.sdi_play("mp3.mp3", 100);
    Serial.println( player.read_mic() );
    //delay(500);
}
