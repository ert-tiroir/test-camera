
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
   
    #define bsize 128

    unsigned char buf[bsize] = { 1, 4, 9, 16, 25, 36, 49, 64 };
    long long start = millis();
    long long laten = 0;
    for (int i = 0; i < 1000000; i ++) {
        int l0 = millis();
        transfer(buf, bsize);
        int l1 = millis();

        laten += l1 - l0;

        for (int j = 0; j < 8; j ++) {
            if (buf[j] != j + 1) {
                printf("ERROR %d: %d\n", j, buf[j]);
            }
        }
        
        //printf("Data returned: ");
        //for (int i = 0; i < 8; i ++) printf("%d ", +buf[i]);
        if (i % 1000 == 999) {
            printf("%d: ", i + 1);
            double data  = bsize * 8 * (i + 1);
            double time  = millis() - start;
            double total = data / time;
            printf("%d / %d = %f b/millis, latency of %f\n", (int) data, (int) time, total, ((double) laten) / ((double) i + 1));
        }
        //break ;
    }
    long long end = millis();

    printf("%lld %lld: %d", start, end, 1000000 * 8);

    return 0;
}
