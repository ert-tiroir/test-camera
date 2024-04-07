
TaskHandle_t TaskWiFi;

const int PAGE_SIZE  = 16;
const int PAGE_COUNT = 16; // must be a power of 2
const int PAGE_MASK  = PAGE_COUNT - 1;

int buffer_start;
int buffer_end;

uint8_t buffer[PAGE_SIZE * PAGE_COUNT];

void taskWiFiCode (void * parameters) {
    for (;;) {
        if (buffer_start == buffer_end) continue ;

        // TODO read

        buffer_start = (buffer_start + 1) & PAGE_MASK;
    }
}

void setup () {
    xTaskCreatePinnedToCore(
        taskWiFiCode,
        "TASK::RECEIVE",
        10000,
        NULL,
        1,
        &TaskWiFi,
        0
    );
    delay(500);

    for (;;) {
        int next = (buffer_end + 1) & PAGE_MASK;
        if (next == buffer_start) continue ;

        // TODO write

        buffer_end = next;
    }
}
