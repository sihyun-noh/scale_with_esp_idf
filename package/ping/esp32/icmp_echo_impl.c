#include "icmp_echo_impl.h"

#include <stdio.h>
#include <string.h>

typedef enum {
  EV_PING_ANY = 0,
  EV_PING_SUCCESS = 1,
  EV_PING_FAILURE = 2,
} ping_event_t;

typedef enum {
  EVENT_OK = 0,
  EVENT_CREATE_FAILED,
  EVENT_SEND_FAILED,
  EVENT_RECV_FAILED,
} event_err_t;

typedef struct {
  uint8_t *data;
  uint32_t data_len;
  ping_event_t event_id;
} ping_event_msg_t;

#define MAX_PING_MESSAGES 5
#define MAX_PING_MSG_SIZE sizeof(ping_event_msg_t)

static SemaphoreHandle_t s_ping_mutex = NULL;  // Mutex lock for ping command

/* Array containing pointer to the event structures used to send events to the easy_setup task */
static ping_event_msg_t queue_data[MAX_PING_MESSAGES * MAX_PING_MSG_SIZE];

/* Queue control structure */
static StaticQueue_t s_queue;

/* Queue control handle */
static QueueHandle_t event_queue;

static int init_ping_event(void) {
  int ret = EVENT_OK;

  event_queue = xQueueCreateStatic((UBaseType_t)MAX_PING_MESSAGES, (UBaseType_t)MAX_PING_MSG_SIZE,
                                   (uint8_t *)queue_data, &s_queue);

  if (event_queue == NULL) {
    ret = EVENT_CREATE_FAILED;
    printf("Failed to create Event Queue !!!\n");
  } else {
    printf("Event Queue created !!!\n");
  }
  return ret;
}

static int send_ping_event(ping_event_t event_id) {
  int ret = EVENT_OK;
  ping_event_msg_t event_msg = { 0 };

  event_msg.event_id = event_id;

  BaseType_t rc = xQueueSendToBack(event_queue, &event_msg, (TickType_t)0);
  if (rc == pdTRUE) {
    printf("Event Sent : [%d] !!!\n", event_id);
  } else {
    ret = EVENT_SEND_FAILED;
    printf("Failed to send Event !!!\n");
  }
  return ret;
}

static int recv_ping_event(ping_event_t *event_id) {
  int ret = EVENT_OK;
  ping_event_msg_t event_msg;

  BaseType_t rc = xQueueReceive(event_queue, &event_msg, portMAX_DELAY);
  if (rc == pdTRUE) {
    *event_id = event_msg.event_id;
    if (event_msg.data) {
      free(event_msg.data);
    }
    printf("Event received : [%d] !!!\n", *event_id);
  } else {
    ret = EVENT_RECV_FAILED;
    printf("Failed to receive Event !!!\n");
  }

  return ret;
}

static void deinit_ping_event(void) {
  if (event_queue != NULL) {
    vQueueDelete(event_queue);
    printf("Event Queue deleted!!!\n");
  }
}

static void lock_ping_cmd(void) {
  if (s_ping_mutex == NULL) {
    s_ping_mutex = xSemaphoreCreateMutex();
  }
  xSemaphoreTake(s_ping_mutex, portMAX_DELAY);
}

static void unlock_ping_cmd(void) {
  if (s_ping_mutex) {
    xSemaphoreGive(s_ping_mutex);
  }
}

int do_ping_impl(char *host, uint8_t ping_count) {
  int rc = PING_OK;
  ping_event_t event_id = EV_PING_ANY;

  lock_ping_cmd();

  init_ping_event();

  if (ping_count == 0) {
    rc = PING_ERR_NOT_COUNT;
    goto exit;
  }
  // parse IP address
  struct sockaddr_in6 sock_addr6;
  ip_addr_t target_addr;
  memset(&target_addr, 0, sizeof(target_addr));
  if (inet_pton(AF_INET6, host, &sock_addr6.sin6_addr) == 1) {
    /* convert ip6 string to ip6 address */
    ipaddr_aton(host, &target_addr);
  } else {
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    /* convert ip4 string or hostname to ip4 or ip6 address */
    if (getaddrinfo(host, NULL, &hint, &res) != 0) {
      printf("ping: unknown host %s\n", host);
      rc = PING_ERR_UNKNOWN_HOST;
      goto exit;
    }
    if (res->ai_family == AF_INET) {
      struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
      inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    } else {
      struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
      inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
    }
    freeaddrinfo(res);
  }
  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ping_config.target_addr = target_addr;
  ping_config.count = ping_count;

  /* set callback functions */
  esp_ping_callbacks_t cbs = { .cb_args = NULL,
                               .on_ping_success = cmd_ping_on_ping_success,
                               .on_ping_timeout = cmd_ping_on_ping_timeout,
                               .on_ping_end = cmd_ping_on_ping_end };
  esp_ping_handle_t ping;
  esp_ping_new_session(&ping_config, &cbs, &ping);
  esp_ping_start(ping);

  if (recv_ping_event(&event_id) == EVENT_OK) {
    if (event_id == EV_PING_SUCCESS) {
      printf("ping command success...\n");
      rc = PING_OK;
    } else {
      printf("ping command failure...\n");
      rc = PING_FAIL;
    }
  }

exit:
  deinit_ping_event();
  unlock_ping_cmd();
  return rc;
}

void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args) {
  uint8_t ttl;
  uint16_t seqno;
  uint32_t elapsed_time, recv_len;
  ip_addr_t target_addr;

  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));

  printf("%ld bytes from %s icmp_seq=%d ttl=%d time=%ld ms\n", recv_len, ipaddr_ntoa((ip_addr_t *)&target_addr), seqno,
         ttl, elapsed_time);
}

void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args) {
  uint16_t seqno;
  ip_addr_t target_addr;

  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));

  printf("From %s icmp_seq=%d timeout\n", ipaddr_ntoa((ip_addr_t *)&target_addr), seqno);
}

void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args) {
  ip_addr_t target_addr;
  uint32_t transmitted;
  uint32_t received;
  uint32_t total_time_ms;

  esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
  esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));

  uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
  if (IP_IS_V4(&target_addr)) {
    printf("\n--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
  } else {
    printf("\n--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
  }

  printf("%ld packets transmitted, %ld received, %ld%% packet loss, time %ldms\n", transmitted, received, loss,
         total_time_ms);

  esp_ping_stop(hdl);
  // delete the ping sessions, so that we clean up all resources and can create a new ping session
  // we don't have to call delete function in the callback, instead we can call delete function from other tasks
  esp_ping_delete_session(hdl);

  // send event message which ping command is done to the do_ping command api, do_ping api should be blocking function
  if (loss == 100) {
    // 100% packet loss, the ping command is failed.
    send_ping_event(EV_PING_FAILURE);
  } else {
    // Otherwise, ping command is success.
    send_ping_event(EV_PING_SUCCESS);
  }
}
