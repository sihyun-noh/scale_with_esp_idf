#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <string.h>

#include <lwmqtt.h>

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
#include "tls_lwmqttc.h"
#endif

#include "lwmqttc.h"
#include "mqttc.h"

#define MQTT_TAG "lwmqtt"

static SemaphoreHandle_t mqtt_main_mutex = NULL;

#define MQTT_LOCK_MAIN() \
  do {                   \
  } while (xSemaphoreTake(mqtt_main_mutex, portMAX_DELAY) != pdPASS)

#define MQTT_UNLOCK_MAIN() xSemaphoreGive(mqtt_main_mutex)

static SemaphoreHandle_t mqtt_select_mutex = NULL;

#define MQTT_LOCK_SELECT() \
  do {                     \
  } while (xSemaphoreTake(mqtt_select_mutex, portMAX_DELAY) != pdPASS)

#define MQTT_UNLOCK_SELECT() xSemaphoreGive(mqtt_select_mutex)

static TaskHandle_t mqtt_task = NULL;

static size_t mqtt_buffer_size;
static uint32_t mqtt_command_timeout;

static struct {
  char *host;
  char *port;
  char *client_id;
  char *username;
  char *password;
} mqtt_config = { 0 };

static struct {
  char *topic;
  char *payload;
  int qos;
  bool retained;
} mqtt_lwt_config = { 0 };

static bool mqtt_running = false;
static bool mqtt_connected = false;
static bool mqtt_error = false;

static mqtt_status_callback_t mqtt_status_callback = NULL;
static mqtt_message_callback_t mqtt_message_callback = NULL;

static lwmqtt_client_t mqtt_client;

static lwmqtt_network_t mqtt_network = { 0 };

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
static bool mqtt_use_tls = false;
static tls_lwmqtt_network_t mqtt_tls_network = { 0 };
#endif

static lwmqtt_timer_t mqtt_timer1, mqtt_timer2;

static void *mqtt_write_buffer;
static void *mqtt_read_buffer;

static QueueHandle_t mqtt_event_queue = NULL;

typedef struct {
  void *buffer;
  lwmqtt_string_t topic;
  lwmqtt_message_t message;
} mqtt_event_t;

void lwmqtt_client_init(mqtt_status_callback_t scb, mqtt_message_callback_t mcb, size_t buffer_size,
                        int command_timeout) {
  // set callbacks
  mqtt_status_callback = scb;
  mqtt_message_callback = mcb;
  mqtt_buffer_size = buffer_size;
  mqtt_command_timeout = (uint32_t)command_timeout;

  // allocate buffers
  mqtt_write_buffer = malloc((size_t)buffer_size);
  mqtt_read_buffer = malloc((size_t)buffer_size);

  // create mutexes
  mqtt_main_mutex = xSemaphoreCreateMutex();
  mqtt_select_mutex = xSemaphoreCreateMutex();

  // create queue
  mqtt_event_queue = xQueueCreate(CONFIG_LWMQTT_EVENT_QUEUE_SIZE, sizeof(mqtt_event_t *));
}

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
bool lwmqtt_client_tls(bool enable, bool verify, const uint8_t *ca_buf, size_t ca_len) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // free existing buffer
  if (mqtt_tls_network.ca_buf) {
    free(mqtt_tls_network.ca_buf);
  }

  // disable if requested
  if (!enable) {
    mqtt_use_tls = false;
    mqtt_tls_network.verify = false;
    mqtt_tls_network.ca_buf = NULL;
    mqtt_tls_network.ca_len = 0;
    MQTT_UNLOCK_MAIN();
    return true;
  }

  // check ca certificate
  if (verify && (!ca_buf || ca_len <= 0)) {
    LOGE(MQTT_TAG, "mqtt_tls: ca_buf must be not NULL when verify is enabled.");
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // set configuration
  mqtt_use_tls = true;
  mqtt_tls_network.verify = verify;
  mqtt_tls_network.ca_buf = NULL;
  mqtt_tls_network.ca_len = 0;

  // copy buffer
  if (ca_buf) {
    mqtt_tls_network.ca_len = ca_len + 1;
    mqtt_tls_network.ca_buf = malloc(ca_len + 1);
    memcpy(mqtt_tls_network.ca_buf, ca_buf, ca_len);
    mqtt_tls_network.ca_buf[ca_len] = 0;
  }

  // release mutex
  MQTT_UNLOCK_MAIN();

  return true;
}
#endif

static void mqtt_message_handler(lwmqtt_client_t *client, void *ref, lwmqtt_string_t topic, lwmqtt_message_t msg) {
  // allocate buffer
  void *buffer = malloc(sizeof(mqtt_event_t) + (size_t)topic.len + 1 + msg.payload_len + 1);

  // prepare message
  mqtt_event_t *evt = buffer;

  // copy topic with additional null termination
  evt->topic.len = topic.len;
  evt->topic.data = buffer + sizeof(mqtt_event_t);
  memcpy(evt->topic.data, topic.data, (size_t)topic.len);
  evt->topic.data[topic.len] = 0;

  // copy message with additional null termination
  evt->message.retained = msg.retained;
  evt->message.qos = msg.qos;
  evt->message.payload_len = msg.payload_len;
  evt->message.payload = buffer + sizeof(mqtt_event_t) + (size_t)topic.len + 1;
  memcpy(evt->message.payload, msg.payload, (size_t)msg.payload_len);
  evt->message.payload[msg.payload_len] = 0;

  // queue event
  if (xQueueSend(mqtt_event_queue, &evt, 0) != pdTRUE) {
    LOGE(MQTT_TAG, "xQueueSend: queue is full, dropping message");
    free(evt);
  }
}

static void mqtt_dispatch_events() {
  // receive next event
  mqtt_event_t *evt = 0;
  while (xQueueReceive(mqtt_event_queue, &evt, 0) == pdTRUE) {
    // call callback if existing
    if (mqtt_message_callback) {
      mqtt_message_callback(evt->topic.data, evt->message.payload, evt->message.payload_len, (int)evt->message.qos,
                            evt->message.retained);
    }

    // free event
    free(evt);
  }
}

static bool mqtt_process_connect() {
  // initialize the client
  lwmqtt_init(&mqtt_client, mqtt_write_buffer, mqtt_buffer_size, mqtt_read_buffer, mqtt_buffer_size);

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
  if (mqtt_use_tls) {
    lwmqtt_set_network(&mqtt_client, &mqtt_tls_network, tls_lwmqtt_network_read, tls_lwmqtt_network_write);
  } else {
    lwmqtt_set_network(&mqtt_client, &mqtt_network, lwmqtt_network_read, lwmqtt_network_write);
  }
#else
  lwmqtt_set_network(&mqtt_client, &mqtt_network, lwmqtt_network_read, lwmqtt_network_write);
#endif

  lwmqtt_set_timers(&mqtt_client, &mqtt_timer1, &mqtt_timer2, lwmqtt_timer_set, lwmqtt_timer_get);
  lwmqtt_set_callback(&mqtt_client, NULL, mqtt_message_handler);

  // initiate network connection
  lwmqtt_err_t err;
#if defined(CONFIG_LWMQTT_TLS_ENABLE)
  if (mqtt_use_tls) {
    err = tls_lwmqtt_network_connect(&mqtt_tls_network, mqtt_config.host, mqtt_config.port);
  } else {
    err = lwmqtt_network_connect(&mqtt_network, mqtt_config.host, mqtt_config.port);
  }
#else
  err = lwmqtt_network_connect(&mqtt_network, mqtt_config.host, mqtt_config.port);
#endif

  if (err != LWMQTT_SUCCESS) {
    LOGE(MQTT_TAG, "lwmqtt_network_connect: %d", err);
    return false;
  }

  // release mutex
  MQTT_UNLOCK_MAIN();

  // acquire select mutex
  MQTT_LOCK_SELECT();

  // wait for connection
  bool connected = false;

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
  if (mqtt_use_tls) {
    err = tls_lwmqtt_network_wait(&mqtt_tls_network, &connected, mqtt_command_timeout);
  } else {
    err = lwmqtt_network_wait(&mqtt_network, &connected, mqtt_command_timeout);
  }
#else
  err = lwmqtt_network_wait(&mqtt_network, &connected, mqtt_command_timeout);
#endif

  if (err != LWMQTT_SUCCESS) {
    LOGE(MQTT_TAG, "lwmqtt_network_wait: %d", err);
    return false;
  }

  // release select mutex
  MQTT_UNLOCK_SELECT();

  // acquire mutex
  MQTT_LOCK_MAIN();

  // return if not connected
  if (!connected) {
    return false;
  }

  // setup connect data
  lwmqtt_options_t options = lwmqtt_default_options;
  options.keep_alive = 10;
  options.client_id = lwmqtt_string(mqtt_config.client_id);
  options.username = lwmqtt_string(mqtt_config.username);
  options.password = lwmqtt_string(mqtt_config.password);

  // last will data
  lwmqtt_will_t will;
  will.topic = lwmqtt_string(mqtt_lwt_config.topic);
  will.qos = (lwmqtt_qos_t)mqtt_lwt_config.qos;
  will.retained = mqtt_lwt_config.retained;
  will.payload = lwmqtt_string(mqtt_lwt_config.payload);

  // attempt connection
  lwmqtt_return_code_t return_code;
  err = lwmqtt_connect(&mqtt_client, options, will.topic.len ? &will : NULL, &return_code, mqtt_command_timeout);
  if (err != LWMQTT_SUCCESS) {
    LOGE(MQTT_TAG, "lwmqtt_connect: %d", err);
    return false;
  }

  return true;
}

static void mqtt_process() {
  // connection loop
  for (;;) {
    // log attempt
    LOGI(MQTT_TAG, "mqtt_process: begin connection attempt");

    // acquire mutex
    MQTT_LOCK_MAIN();

    // make connection attempt
    if (mqtt_process_connect()) {
      // log success
      LOGI(MQTT_TAG, "mqtt_process: connection attempt successful");

      // set flag
      mqtt_connected = true;

      // release mutex
      MQTT_UNLOCK_MAIN();

      // exit loop
      break;
    }

    // release mutex
    MQTT_UNLOCK_MAIN();

    // log fail
    LOGW(MQTT_TAG, "mqtt_process: connection attempt failed");

    // delay loop by 1s and yield to other processes
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // call callback if existing
  if (mqtt_status_callback) {
    mqtt_status_callback(MQTT_STATUS_CONNECTED);
  }

  // yield loop
  for (;;) {
    // check for error
    if (mqtt_error) {
      break;
    }

    // acquire select mutex
    MQTT_LOCK_SELECT();

    // block until data is available
    bool available = false;

    lwmqtt_err_t err;
#if defined(CONFIG_LWMQTT_TLS_ENABLE)
    if (mqtt_use_tls) {
      err = tls_lwmqtt_network_select(&mqtt_tls_network, &available, mqtt_command_timeout);
    } else {
      err = lwmqtt_network_select(&mqtt_network, &available, mqtt_command_timeout);
    }
#else
    err = lwmqtt_network_select(&mqtt_network, &available, mqtt_command_timeout);
#endif

    if (err != LWMQTT_SUCCESS) {
      LOGE(MQTT_TAG, "lwmqtt_network_select: %d", err);
      MQTT_UNLOCK_SELECT();
      break;
    }

    // release select mutex
    MQTT_UNLOCK_SELECT();

    // acquire mutex
    MQTT_LOCK_MAIN();

    // process data if available
    if (available) {
      // get available bytes
      size_t available_bytes = 0;

#if defined(CONFIG_LWMQTT_TLS_ENABLE)
      if (mqtt_use_tls) {
        err = tls_lwmqtt_network_peek(&mqtt_tls_network, &available_bytes, mqtt_command_timeout);
      } else {
        err = lwmqtt_network_peek(&mqtt_network, &available_bytes);
      }
#else
      err = lwmqtt_network_peek(&mqtt_network, &available_bytes);
#endif

      if (err != LWMQTT_SUCCESS) {
        LOGE(MQTT_TAG, "lwmqtt_network_peek: %d", err);
        MQTT_UNLOCK_MAIN();
        break;
      }

      // yield client only if there is still data to read since select might unblock because of incoming ack packets
      // that are already handled until we get to this point
      if (available_bytes > 0) {
        err = lwmqtt_yield(&mqtt_client, available_bytes, mqtt_command_timeout);
        if (err != LWMQTT_SUCCESS) {
          LOGE(MQTT_TAG, "lwmqtt_yield: %d", err);
          MQTT_UNLOCK_MAIN();
          break;
        }
      }
    }

    // do mqtt background work
    err = lwmqtt_keep_alive(&mqtt_client, mqtt_command_timeout);
    if (err != LWMQTT_SUCCESS) {
      LOGE(MQTT_TAG, "lwmqtt_keep_alive: %d", err);
      MQTT_UNLOCK_MAIN();
      break;
    }

    // release mutex
    MQTT_UNLOCK_MAIN();

    // dispatch queued events
    mqtt_dispatch_events();
  }

  // acquire mutex
  MQTT_LOCK_MAIN();

// disconnect network
#if defined(CONFIG_LWMQTT_TLS_ENABLE)
  if (mqtt_use_tls) {
    tls_lwmqtt_network_disconnect(&mqtt_tls_network);
  } else {
    lwmqtt_network_disconnect(&mqtt_network);
  }
#else
  lwmqtt_network_disconnect(&mqtt_network);
#endif

  // set flags
  mqtt_connected = false;
  mqtt_error = false;

  // release mutex
  MQTT_UNLOCK_MAIN();

  // log event
  LOGI(MQTT_TAG, "mqtt_process: lost connection");

  // call callback if existing
  if (mqtt_status_callback) {
    mqtt_status_callback(MQTT_STATUS_DISCONNECTED);
  }
}

static void mqtt_run(void *p) {
  // keep processing
  for (;;) {
    mqtt_process();
  }
}

void lwmqtt_client_lwt(const char *topic, const char *payload, int qos, bool retained) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // free topic if set
  if (mqtt_lwt_config.topic != NULL) {
    free(mqtt_lwt_config.topic);
    mqtt_lwt_config.topic = NULL;
  }

  // free payload if set
  if (mqtt_lwt_config.payload != NULL) {
    free(mqtt_lwt_config.payload);
    mqtt_lwt_config.payload = NULL;
  }

  // set topic if provided
  if (topic != NULL) {
    mqtt_lwt_config.topic = strdup(topic);
  }

  // set payload if provided
  if (payload != NULL) {
    mqtt_lwt_config.payload = strdup(payload);
  }

  // set qos
  mqtt_lwt_config.qos = qos;

  // set retained
  mqtt_lwt_config.retained = retained;

  // release mutex
  MQTT_UNLOCK_MAIN();
}

bool lwmqtt_client_start(const char *host, const char *port, const char *client_id, const char *username,
                         const char *password) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // check if already running
  if (mqtt_running) {
    LOGW(MQTT_TAG, "mqtt_start: already running");
    MQTT_UNLOCK_MAIN();
    return true;
  }

  // free host if set
  if (mqtt_config.host != NULL) {
    free(mqtt_config.host);
    mqtt_config.host = NULL;
  }

  // free port if set
  if (mqtt_config.port != NULL) {
    free(mqtt_config.port);
    mqtt_config.port = NULL;
  }

  // free client id if set
  if (mqtt_config.client_id != NULL) {
    free(mqtt_config.client_id);
    mqtt_config.client_id = NULL;
  }

  // free username if set
  if (mqtt_config.username != NULL) {
    free(mqtt_config.username);
    mqtt_config.username = NULL;
  }

  // free password if set
  if (mqtt_config.password != NULL) {
    free(mqtt_config.password);
    mqtt_config.password = NULL;
  }

  // set host if provided
  if (host != NULL) {
    mqtt_config.host = strdup(host);
  }

  // set port if provided
  if (port != NULL) {
    mqtt_config.port = strdup(port);
  }

  // set client id if provided
  if (client_id != NULL) {
    mqtt_config.client_id = strdup(client_id);
  }

  // set username if provided
  if (username != NULL) {
    mqtt_config.username = strdup(username);
  }

  // set password if provided
  if (password != NULL) {
    mqtt_config.password = strdup(password);
  }

  // create mqtt thread
  LOGI(MQTT_TAG, "mqtt_start: create task");
  BaseType_t ret = xTaskCreatePinnedToCore(mqtt_run, "mqtt", CONFIG_LWMQTT_TASK_STACK_SIZE, NULL,
                                           CONFIG_LWMQTT_TASK_STACK_PRIORITY, &mqtt_task, 1);
  if (ret != pdPASS) {
    LOGW(MQTT_TAG, "mqtt_start: failed to create task");
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // set flag
  mqtt_running = true;

  // release mutex
  MQTT_UNLOCK_MAIN();

  return true;
}

bool lwmqtt_client_subscribe(const char *topic, int qos) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // check if still connected
  if (!mqtt_connected) {
    LOGW(MQTT_TAG, "mqtt_subscribe: not connected");
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // subscribe to topic
  lwmqtt_err_t err = lwmqtt_subscribe_one(&mqtt_client, lwmqtt_string(topic), (lwmqtt_qos_t)qos, mqtt_command_timeout);
  if (err != LWMQTT_SUCCESS) {
    mqtt_error = true;
    LOGE(MQTT_TAG, "lwmqtt_subscribe_one: %d", err);
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // release mutex
  MQTT_UNLOCK_MAIN();

  return true;
}

bool lwmqtt_client_unsubscribe(const char *topic) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // check if still connected
  if (!mqtt_connected) {
    LOGW(MQTT_TAG, "mqtt_unsubscribe: not connected");
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // unsubscribe from topic
  lwmqtt_err_t err = lwmqtt_unsubscribe_one(&mqtt_client, lwmqtt_string(topic), mqtt_command_timeout);
  if (err != LWMQTT_SUCCESS) {
    mqtt_error = true;
    LOGE(MQTT_TAG, "lwmqtt_unsubscribe_one: %d", err);
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // release mutex
  MQTT_UNLOCK_MAIN();

  return true;
}

bool lwmqtt_client_publish(const char *topic, const uint8_t *payload, size_t len, int qos, bool retained) {
  // acquire mutex
  MQTT_LOCK_MAIN();

  // check if still connected
  if (!mqtt_connected) {
    LOGW(MQTT_TAG, "mqtt_publish: not connected");
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // prepare message
  lwmqtt_message_t message;
  message.qos = (lwmqtt_qos_t)qos;
  message.retained = retained;
  message.payload = (uint8_t *)payload;
  message.payload_len = len;

  // publish message
  lwmqtt_err_t err = lwmqtt_publish(&mqtt_client, lwmqtt_string(topic), message, mqtt_command_timeout);
  if (err != LWMQTT_SUCCESS) {
    mqtt_error = true;
    LOGE(MQTT_TAG, "lwmqtt_publish: %d", err);
    MQTT_UNLOCK_MAIN();
    return false;
  }

  // release mutex
  MQTT_UNLOCK_MAIN();

  return true;
}

void lwmqtt_client_stop() {
  // acquire mutexes
  MQTT_LOCK_MAIN();
  MQTT_LOCK_SELECT();

  // return immediately if not running anymore
  if (!mqtt_running) {
    MQTT_UNLOCK_SELECT();
    MQTT_UNLOCK_MAIN();
    return;
  }

  // attempt to properly disconnect a connected client
  if (mqtt_connected) {
    lwmqtt_err_t err = lwmqtt_disconnect(&mqtt_client, mqtt_command_timeout);
    if (err != LWMQTT_SUCCESS) {
      LOGE(MQTT_TAG, "lwmqtt_disconnect: %d", err);
    }

    // set flag
    mqtt_connected = false;
  }

// disconnect network
#if defined(CONFIG_LWMQTT_TLS_ENABLE)
  if (mqtt_use_tls) {
    tls_lwmqtt_network_disconnect(&mqtt_tls_network);
  } else {
    lwmqtt_network_disconnect(&mqtt_network);
  }
#else
  lwmqtt_network_disconnect(&mqtt_network);
#endif

  // kill mqtt task
  LOGI(MQTT_TAG, "mqtt_stop: deleting task");
  vTaskDelete(mqtt_task);

  // set flag
  mqtt_running = false;

  // release mutexes
  MQTT_UNLOCK_SELECT();
  MQTT_UNLOCK_MAIN();
}
