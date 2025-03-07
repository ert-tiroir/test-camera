#include "driver/spi_slave.h"

const int SPI_AVL0 = 17;
const int SPI_AVL1 = 18;

int SPI_AVL[2] = { SPI_AVL0, SPI_AVL1 };
int SPI_AVL_offset = 0;

const int SPI_AVL_SIZE = 2;
 
void spi_slave_post_setup_cb(spi_slave_transaction_t *trans) {
    digitalWrite(SPI_AVL[SPI_AVL_offset], HIGH);
}

void spi_slave_post_trans_cb(spi_slave_transaction_t *trans) {
    digitalWrite(SPI_AVL[SPI_AVL_offset], LOW);
    SPI_AVL_offset ++;
    if (SPI_AVL_offset == SPI_AVL_SIZE) SPI_AVL_offset = 0;
}

const int BUFFER_SIZE = 1024;

DMA_ATTR uint8_t recvbuf[BUFFER_SIZE + 1] = { 1, 2, 3, 4, 5, 6, 7, 8 };   
DMA_ATTR uint8_t txbuf[BUFFER_SIZE + 1]   = { 1, 2, 3, 4, 5, 6, 7, 8 };

#include "esp_wifi.h"
#include "WiFi.h"

WiFiServer server;

const char* ssid = "NORDLI-ESP32";
const char* pwd  = "nordli-esp32";

void setup()
{
  pinMode(SPI_AVL0, OUTPUT);
  pinMode(SPI_AVL1, OUTPUT);
    int n=0;
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .mosi_io_num=MOSI,
        .miso_io_num=MISO,
        .sclk_io_num=SCK,
        .data2_io_num=-1,
        .data3_io_num=-1,
        .data4_io_num=-1,
        .data5_io_num=-1,
        .data6_io_num=-1,
        .data7_io_num=-1,
        .max_transfer_sz=4096,
        .flags=SPICOMMON_BUSFLAG_SLAVE,
        .intr_flags=0
    };
    spi_slave_interface_config_t slvcfg={
        .spics_io_num = SS,
        .flags = 0,
        .queue_size = 1,
        .mode = SPI_MODE0,
        .post_setup_cb = spi_slave_post_setup_cb,
        .post_trans_cb = spi_slave_post_trans_cb,
    };
    ret=spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);

  Serial.begin(1000000);
    if (ret != ESP_OK) {
      Serial.println("\n==========\nINIT ERROR\n==========\n");
      return ;
    }

  // put your setup code here, to run once:
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pwd);
  delay(1000);

  IPAddress ip (192, 168, 0, 1);
  IPAddress nm (255, 255, 255, 0);

  WiFi.softAPConfig(ip, ip, nm);

  Serial.println(WiFi.softAPIP());

  server.begin(4242);
}

uint8_t buffer[1024];

void loop() {
  WiFiClient client = server.available();
  if (client) {} else return ;

  spi_slave_transaction_t t;
  memset(&t, 0, sizeof(t));
  int cont = 0;

  while (client.connected()) {
    if (client.available() >= 1024) {
        client.read(txbuf, 1024);

        t.length = BUFFER_SIZE * 8;
        t.trans_len = BUFFER_SIZE * 8; 
        t.rx_buffer= recvbuf;
        t.tx_buffer = txbuf;
        ret=spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
    }
  }

}
