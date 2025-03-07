
#include <stdio.h>
#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPI

#include <stdio.h>
#include <wiringPiSPI.h>

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

int main(int argc, char **argv)
{
    wiringPiSetup();
    pinMode(4, INPUT);
    pinMode(5, INPUT);

    int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
    if (fd == -1) {
        printf("Failed to init SPI communication.\n");
        return -1;
    }
    printf("SPI communication successfully setup.\n");
   
    #define bsize 1024

    unsigned char buf[bsize] = { 1, 4, 9, 16, 25, 36, 49, 64 };
    
    int s0 = 0;

    FILE* file = NULL;

    while (1) {
        transfer(buf, bsize);

        int offset = 0;

        if (s0 == 0){
            file = fopen("result", "w");
            s0 = (buf[0] << 24) + (buf[1] <<16) + (buf[2] << 8) + buf[3]; 
            offset = 4; 
        }
        int q = s0; 
        if (1024-offset< s0)
            q=1024-offset;

        fwrite(buf + offset, 1, q, file);
        
        s0 -= q;
        if (s0 == 0) {
            fclose(file);
            file = NULL;
        }
    }

    return 0;
}
