#include <stdio.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>

#include <stdlib.h>

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 16000000

int SPI_AVL[2] = { 4, 5 };
int SPI_AVL_offset = -1;
const int SPI_AVL_size = 2;

void transfer (unsigned char* buf, int sze) {
    while (SPI_AVL_offset == -1) {
        for (int p = 0; p < SPI_AVL_size; p ++) {
            if (digitalRead(SPI_AVL[p])) {
                SPI_AVL_offset = p;
                break ;
            }
        }
    }

    int pin = SPI_AVL[SPI_AVL_offset];
    int cnt = 0;
    while (!digitalRead(pin)) cnt ++;

    wiringPiSPIDataRW(SPI_CHANNEL, buf, sze);

    SPI_AVL_offset ++;
    if (SPI_AVL_offset == SPI_AVL_size) SPI_AVL_offset = 0;
}

int main(){
      wiringPiSetup();
    pinMode(4, INPUT);
    pinMode(5, INPUT);

    int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
    if (fd == -1) {
        printf("Failed to init SPI communication.\n");
        return -1;
    }
    printf("SPI communication successfully setup.\n");

    FILE * file = fopen("blink.c","rb");
    if (file == NULL){
        printf ("could not open file \n");
        return 0;
    }

    unsigned char buf[1024] = {};
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file); 

    int offset = 4;

   buf[0]= size >> 24;
   buf[1]= size >> 16; 
   buf[2]= size >> 8; 
   buf[3]= size >> 0; 

    while (1){
        size_t received_bytes = fread (buf + offset, 1, 1024 - offset, file);
        if (received_bytes <= 0) 
            break;
        transfer(buf, 1024);
        offset = 0;
    }
    
    fclose(file);

    return 0;
}