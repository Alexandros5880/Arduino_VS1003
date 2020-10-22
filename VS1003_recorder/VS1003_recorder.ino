

const unsigned char RIFFHeader0[] = {
 'R' , 'I' , 'F' , 'F' , // Chunk ID (RIFF)
 0x70, 0x70, 0x70, 0x70, // Chunk payload size (calculate after rec!)
 'W' , 'A' , 'V' , 'E' , // RIFF resource format type
 
 'f' , 'm' , 't' , ' ' , // Chunk ID (fmt )
 0x14, 0x00, 0x00, 0x00, // Chunk payload size (0x14 = 20 bytes)
 0x11, 0x00,             // Format Tag (IMA ADPCM)
 0x01, 0x00,             // Channels (1)
 0x80, 0x3e, 0x00, 0x00, // Sample Rate, 0x3e80 = 16.0kHz
 0xd7, 0x0f, 0x00, 0x00, // Average Bytes Per Second
 0x00, 0x01,             // Data Block Size (256 bytes) 
 0x04, 0x00,             // ADPCM encoded bits per sample (4 bits)
 0x02, 0x00,             // Extra data (2 bytes)
 0xf9, 0x01,             // Samples per Block (505 samples)
 
 'f' , 'a' , 'c' , 't' , // Chunk ID (fact)
 0xc8, 0x01, 0x00, 0x00, // Chunk payload size (456 bytes (zeropad!))
 0xff, 0xff, 0xff, 0xff  // Number of Samples (calculate after rec!)
 };


const unsigned char RIFFHeader504[] = {
 'd' , 'a' , 't' , 'a' , // Chunk ID (data)
 0x70, 0x70, 0x70, 0x70  // Chunk payload size (calculate after rec!)
 };





 


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
