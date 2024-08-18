
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/portmacro.h"
#include "driver/uart.h"
#include "main.h"
#include "sysevent.h"
#include "log.h"
#include "scale_read_485.h"
#include "event_ids.h"
#include "esp32/uart_pal.h"
#include "config.h"

#include <string.h>

#define CTRL_MSG_QUEUE_LEN 16

#define BUF_SIZE 255
#define EX_UART_NUM UART_PORT_NUM
#define PATTERN_CHR_NUM (3)
#define RD_BUF_SIZE (BUF_SIZE)

static const char *TAG = "ctrl_task";

// extern char indicator_set[20];

static TaskHandle_t control_handle = NULL;
static QueueHandle_t ctrl_msg_queue = NULL;

static bool _send_msg_event(Common_data_t *msg) {
  return ctrl_msg_queue && (xQueueSend(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static bool _get_msg_event(Common_data_t *msg) {
  return ctrl_msg_queue && (xQueueReceive(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

void send_msg_to_ctrl_task(void *msg, size_t msg_len) {
  Common_data_t message = { 0 };

  if (msg && (msg_len == sizeof(Common_data_t))) {
    memcpy(&message, msg, msg_len);
    if (!_send_msg_event(&message)) {
      LOGI(TAG, "Failed to send ctrl messag event!!!");
    }
  }
}

static QueueHandle_t uart2_queue;

struct uart_hal {
  uint8_t num;
  int ref_cnt;
  bool has_peek;
  uint8_t peek_byte;
  SemaphoreHandle_t mutex;
  QueueHandle_t uart_event_queue;
};

static void uart_event_task(void *pvParameters) {
  uart_event_t event;
  size_t buffered_size;
  uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
  for (;;) {
    // Waiting for UART event.
    if (xQueueReceive(uart2_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
      bzero(dtmp, RD_BUF_SIZE);
      LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
      switch (event.type) {
        // Event of UART receving data
        /*We'd better handler data event fast, there would be much more data events than
        other types of events. If we take too much time on data event, the queue might
        be full.*/
        case UART_DATA:
          LOGI(TAG, "[UART DATA]: %d", event.size);
          uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
          LOGI(TAG, "[receive data %s:", dtmp);
          sysevent_set(WEIGHT_DATA_EVENT, dtmp);
          break;
        // Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
          LOGI(TAG, "hw fifo overflow");
          // If fifo overflow happened, you should consider adding flow control for your application.
          // The ISR has already reset the rx FIFO,
          // As an example, we directly flush the rx buffer here in order to read more data.
          uart_flush_input(EX_UART_NUM);
          xQueueReset(uart2_queue);
          break;
        // Event of UART ring buffer full
        case UART_BUFFER_FULL:
          LOGI(TAG, "ring buffer full");
          // If buffer full happened, you should consider increasing your buffer size
          // As an example, we directly flush the rx buffer here in order to read more data.
          uart_flush_input(EX_UART_NUM);
          xQueueReset(uart2_queue);
          break;
        // Event of UART RX break detected
        case UART_BREAK: LOGI(TAG, "uart rx break"); break;
        // Event of UART parity check error
        case UART_PARITY_ERR: LOGI(TAG, "uart parity error"); break;
        // Event of UART frame error
        case UART_FRAME_ERR: LOGI(TAG, "uart frame error"); break;
        // UART_PATTERN_DET
        case UART_PATTERN_DET:
          uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
          int pos = uart_pattern_pop_pos(EX_UART_NUM);
          LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
          if (pos == -1) {
            // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
            // record the position. We should set a larger queue size.
            // As an example, we directly flush the rx buffer here.
            uart_flush_input(EX_UART_NUM);
          } else {
            uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
            uint8_t pat[PATTERN_CHR_NUM + 1];
            memset(pat, 0, sizeof(pat));
            uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
            LOGI(TAG, "read data: %s", dtmp);
            LOGI(TAG, "read pat : %s", pat);
          }
          break;
        // Others
        default: LOGI(TAG, "uart event type: %d", event.type); break;
      }
    }
  }
  free(dtmp);
  dtmp = NULL;
  vTaskDelete(NULL);
}

int uart_interrupt_config(void) {
  uart_hal_t *res = (uart_hal_t *)malloc(sizeof(uart_hal_t));
  if (res == NULL) {
    LOGI(TAG, "Failed to allocate memory");
    return -1;
  }
  res = uart_hal_initialize_intr(UART_PORT_NUM, BAUD_RATE, MB_RX_PIN, MB_TX_PIN, UART_RXBUF_SIZE, UART_TXBUF_SIZE);
  if (res == 0) {
    LOGI(TAG, "Configuration for interrupt is not set");
    return -1;
  }
  uart2_queue = res->uart_event_queue;
  // Set uart pattern detect function.
  uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
  // Reset the pattern queue length to record at most 20 pattern positions.
  uart_pattern_queue_reset(EX_UART_NUM, 20);

  return 0;
}

static void control_task(void *pvParameter) {
  Common_data_t weight_data;
  char str[100] = { 0 };
  char printer_data_buffer[10] = { 0 };
  int res, res_printer = 0;
  for (;;) {
    // LOGE(TAG, "Failed to receive ctrl message form queue");
    // LOGI(TAG, "start firmware");

    /////////////////// print operation logic //////////////////////
    res_printer = sysevent_get(SYSEVENT_BASE, WEIGHT_PRINTER_EVENT, printer_data_buffer, sizeof(printer_data_buffer));

    if (res_printer == 0) {
      SET_MUX_CONTROL(CH_1_SET);
      LOGI(TAG, "print start");

#ifdef CONFIG_PRINT_FORMAT_REGACY
      LOGI(TAG, "dlp print format");
      while (cas_dlp_label_weight_print_msg(printer_data_buffer) == -1) {
        vTaskDelay(200);  // 실제 읽기 시간
      }
      // cas_dlp_label_weight_print_msg(printer_data_buffer);
      real_print_cmd();
#else
      LOGI(TAG, "custom print format");
      weight_print_msg(printer_data_buffer, UNIT_G);
#endif

      SET_MUX_CONTROL(CH_2_SET);
    }

    ////////////// read weight data /////////////////////////
    memset(&weight_data, 0x00, sizeof(weight_data));
    if (strncmp(indicator_set, "SWII-CS", 7) == 0) {
      // LOGI(TAG, "start SWII-CB");
      res = indicator_CAS_sw_11_data(&weight_data);
    } else if (strncmp(indicator_set, "INNOTEM-T28", 11) == 0) {
      // LOGI(TAG, "start innotem T25");
      res = indicator_INNOTEM_T25_data(&weight_data);
      vTaskDelay(500);  // 실제 읽기 시간
    } else if (strncmp(indicator_set, "EC-D", 4) == 0) {
      res = indicator_EC_D_Serise_data(&weight_data);
    } else if (strncmp(indicator_set, "HB/HBI", 6) == 0) {
      res = indicator_CAS_hb_hbi_data(&weight_data);
    } else if (strncmp(indicator_set, "CB-SERIES", 9) == 0) {
      res = indicator_AND_CB_12K_data(&weight_data);
    } else {
      LOGI(TAG, "No paly control task");
    }

    // weight, unit, zero, stable, sign, trash
    // 자릿수확인 1 -> 10 변할 때 1자리에서 2자리로
    if (res == -1) {
      sysevent_set(WEIGHT_DATA_RES_EVENT, &res);
    } else {
      snprintf(str, sizeof(str), "%10s,%d,%d,%d,%d,%d", weight_data.weight_data, weight_data.spec.unit,
               weight_data.event[STATE_ZERO_EVENT], weight_data.event[STATE_STABLE_EVENT],
               weight_data.event[STATE_SIGN_EVENT], weight_data.event[STATE_TRASH_CHECK_EVENT]);
      sysevent_set(WEIGHT_DATA_EVENT, str);
      sysevent_set(WEIGHT_DATA_RES_EVENT, &res);
    }
    continue;
  }
  vTaskDelete(NULL);
  control_handle = NULL;
}

void create_control_task(char *indicator_set) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY;

  if (control_handle) {
    LOGI(TAG, "control task is already running...");
    return;
  }

  // create queue
  ctrl_msg_queue = xQueueCreate(CTRL_MSG_QUEUE_LEN, sizeof(Common_data_t));
  if (ctrl_msg_queue == NULL) {
    return;
  }

  if (strncmp(indicator_set, "INNOTEM-T28", 11) == 0) {
    xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 0);
  } else if (strncmp(indicator_set, "MWII-H", 6) == 0 || strncmp(indicator_set, "DB-1/1H", 7) == 0) {
    // Create a task to handler UART event from ISR
    uart_interrupt_config();
    xTaskCreatePinnedToCore(uart_event_task, "uart_event_task", 4096, NULL, 12, NULL, 0);
  } else if (strncmp(indicator_set, "SWII-CS", 7) == 0 || strncmp(indicator_set, "EC-D", 4) == 0 ||
             strncmp(indicator_set, "HB/HBI", 6) == 0 || strncmp(indicator_set, "CB-SERIES", 9) == 0) {
    xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 0);
  } else {
    LOGI(TAG, "None task");
  }
}
