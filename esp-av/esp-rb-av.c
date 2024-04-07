
// ======================================================================================
// ======================================== SPI =========================================
// ======================================================================================

#include "driver/spi_slave.h"

const int SPI_AVL0 = 17;
const int SPI_AVL1 = 18;

int SPI_AVL[2] = {SPI_AVL0, SPI_AVL1};
int SPI_AVL_offset = 0;

const int SPI_AVL_SIZE = 2;

void spi_slave_post_setup_cb(spi_slave_transaction_t *trans)
{
    digitalWrite(SPI_AVL[SPI_AVL_offset], HIGH);
}

void spi_slave_post_trans_cb(spi_slave_transaction_t *trans)
{
    digitalWrite(SPI_AVL[SPI_AVL_offset], LOW);
    SPI_AVL_offset++;
    if (SPI_AVL_offset == SPI_AVL_SIZE)
        SPI_AVL_offset = 0;
}

// ======================================================================================
// ======================================== ROTB ========================================
// ======================================================================================

const int PAGE_SIZE = 1024;
const int PAGE_COUNT = 16; // must be a power of 2
const int PAGE_MASK = PAGE_COUNT - 1;

volatile int buffer_start = 0;
volatile int buffer_end = 0;

DMA_ATTR uint8_t buffer[PAGE_SIZE * PAGE_COUNT];
DMA_ATTR uint8_t tx[PAGE_SIZE];

// ======================================================================================
// ===================================== TASK WIFI ======================================
// ======================================================================================

#include "esp_wifi.h"
#include "WiFi.h"

const char *ssid = "NORDLI-ESP32";
const char *pwd = "nordli-esp32";

IPAddress ip(192, 168, 0, 1);

TaskHandle_t TaskWiFi;

void taskWiFiCode(void *parameters)
{
    Serial.println("INIT SERVER");

    for (;;)
    {
        Serial.println("Attempting to connect...");
        WiFiClient client;
        client.connect(ip, 4242);
        client.setTimeout(50);

        if (client && client.connected())
        {
        }
        else
        {
            delay(500);
            continue;
        }

        Serial.println("Connected...");

        unsigned long lastVTaskDelay = millis();

        for (; client.connected();)
        {
            if (millis() - lastVTaskDelay > 250)
            {
                vTaskDelay(1);
                lastVTaskDelay = millis();
            }

            if (buffer_start == buffer_end)
                continue;

            Serial.println("Sending packet");
            client.write(buffer + (buffer_start * PAGE_SIZE), PAGE_SIZE);

            buffer_start = (buffer_start + 1) & PAGE_MASK;
        }
    }
}

void setup()
{
    Serial.begin(1000000);

    Serial.println("\nINIT WIFI");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pwd);
    delay(1000);

    Serial.println("INIT TASK");
    xTaskCreatePinnedToCore(
        taskWiFiCode,
        "TASK::WIFI @ ESP-GS",
        10000,
        NULL,
        1,
        &TaskWiFi,
        1 - xPortGetCoreID());
    delay(500);

    pinMode(SPI_AVL0, OUTPUT);
    pinMode(SPI_AVL1, OUTPUT);

    Serial.println("INIT SPI");

    int n = 0;
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = SCK,
        .data2_io_num = -1,
        .data3_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 4096,
        .flags = SPICOMMON_BUSFLAG_SLAVE,
        .intr_flags = 0};
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = SS,
        .flags = 0,
        .queue_size = 1,
        .mode = SPI_MODE0,
        .post_setup_cb = spi_slave_post_setup_cb,
        .post_trans_cb = spi_slave_post_trans_cb,
    };
    ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);

    if (ret != ESP_OK)
    {
        Serial.println("\n==========\nINIT ERROR\n==========\n");
        return;
    }
    Serial.println("\n==========\nINIT OK\n==========\n");

    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));

    unsigned long lastVTaskDelay = millis();
    for (;;)
    {
        if (millis() - lastVTaskDelay > 250)
        {
            vTaskDelay(1);
            lastVTaskDelay = millis();
        }
        int next = (buffer_end + 1) & PAGE_MASK;
        if (next == buffer_start)
            continue;

        Serial.println("Read data");
        t.length = PAGE_SIZE * 8;
        t.trans_len = PAGE_SIZE * 8;
        t.rx_buffer = buffer + (buffer_end * 1024);
        t.tx_buffer = tx;
        ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);

        buffer_end = next;
    }
}

void loop()
{
}