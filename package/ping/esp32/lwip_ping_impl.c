#include <lwip/sockets.h>
#include <lwip/raw.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/netdb.h>
#include "freertos/portable.h"
#include "lwip/prot/icmp.h"
#include "lwip/prot/ip.h"

#include "const.h"

#include <stdio.h>
#include <string.h>

int do_ping_lwip_impl(char *host, int seq) {
  int ret = PING_OK;
  int packet_size = 0;
  unsigned char *ping_buf = NULL;
  unsigned char *reply_buf = NULL;
  int i = 0, ping_socket = -1;

  struct sockaddr_in to_addr, from_addr;
  struct hostent *server_host;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *pecho;

  unsigned int ping_time, reply_time;

  int ping_timeout = 1000;
  int from_addr_len = sizeof(struct sockaddr);
  int data_size = 120;
  int ping_id = 0xABCD;

  packet_size = sizeof(struct icmp_echo_hdr) + data_size;
  printf("packet_size = %d\n", packet_size);

  ping_buf = pvPortMalloc(packet_size);
  if (ping_buf == NULL) {
    printf("Failed to allocate ping buffer\n");
    ret = PING_FAIL;
    goto exit;
  }
  reply_buf = pvPortMalloc(packet_size);
  if (reply_buf == NULL) {
    printf("Failed to allocate relay buffer\n");
    ret = PING_FAIL;
    goto exit;
  }

  ping_socket = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);

  /* lwip 1.5.0 */
#if defined(LWIP_SO_SNDRCVTIMEO_NONSTANDARD) && (LWIP_SO_SNDRCVTIMEO_NONSTANDARD == 0)
  struct timeval timeout;
  timeout.tv_sec = ping_timeout / 1000;
  timeout.tv_usec = ping_timeout % 1000 * 1000;
  setsockopt(ping_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#else
  /* lwip 1.4.1 */
  setsockopt(ping_socket, SOL_SOCKET, SO_RCVTIMEO, &ping_timeout, sizeof(ping_timeout));
#endif

  to_addr.sin_len = sizeof(to_addr);
  to_addr.sin_family = AF_INET;

  if (inet_aton(host, &to_addr.sin_addr) == 0) {
    server_host = gethostbyname(host);
    if (server_host == NULL) {
      ret = PING_ERR_UNKNOWN_HOST;
      goto exit;
    }
    memcpy((void *)&to_addr.sin_addr, (void *)server_host->h_addr, server_host->h_length);
  } else {
    to_addr.sin_addr.s_addr = inet_addr(host);
  }

  /* Make ping echo packet */
  for (i = 0; i < data_size; i++) {
    ping_buf[sizeof(struct icmp_echo_hdr) + 1] = (unsigned char)i;
  }

  pecho = (struct icmp_echo_hdr *)ping_buf;
  ICMPH_TYPE_SET(pecho, ICMP_ECHO);
  ICMPH_CODE_SET(pecho, 0);
  pecho->chksum = 0;
  pecho->id = ping_id;
  pecho->seqno = htons(seq + 1);
  /* Checksum includes icmp header and data, Need to calculate after fill up icmp header */
  pecho->chksum = inet_chksum(pecho, sizeof(struct icmp_echo_hdr) + data_size);

  /* Send ping packet */
  sendto(ping_socket, ping_buf, packet_size, 0, (struct sockaddr *)&to_addr, sizeof(to_addr));

  ping_time = xTaskGetTickCount();

  /* Recv ping reply packet */
  if ((recvfrom(ping_socket, reply_buf, packet_size, 0, (struct sockaddr *)&from_addr, (socklen_t *)&from_addr_len)) >=
      ((int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)) &&
       (from_addr.sin_addr.s_addr == to_addr.sin_addr.s_addr))) {
    reply_time = xTaskGetTickCount();
    iphdr = (struct ip_hdr *)reply_buf;
    pecho = (struct icmp_echo_hdr *)(reply_buf + (IPH_HL(iphdr) * 4));

    if ((pecho->id == ping_id) && (pecho->seqno == htons(seq + 1))) {
      printf("[%s] %d bytes from %s: icmp_seq=%d time=%d ms\n", __FUNCTION__, sizeof(struct icmp_echo_hdr),
             (char *)inet_ntoa(from_addr.sin_addr), htons(pecho->seqno), (reply_time - ping_time) * portTICK_RATE_MS);
    } else {
      ret = PING_FAIL;
      goto exit;
    }
  } else {
    ret = PING_FAIL;
  }

exit:
  if (ping_socket != -1) {
    close(ping_socket);
  }
  if (ping_buf) {
    vPortFree(ping_buf);
  }
  if (reply_buf) {
    vPortFree(reply_buf);
  }

  if (ret == PING_FAIL) {
    printf("ping fail = [%d]\n", seq);
  }

  return ret;
}