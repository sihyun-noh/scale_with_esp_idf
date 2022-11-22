#pragma once

#include <functional>
#include <vector>

#ifndef MQTT_MIN_FREE_MEMORY
#define MQTT_MIN_FREE_MEMORY 4096
#endif

#include "AsyncTCP.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#if ASYNC_TCP_SSL_ENABLED
#include <tcp_axtls.h>
#define SHA1_SIZE 20
#endif

#include "mqttlib/Flags.hpp"
#include "mqttlib/ParsingInformation.hpp"
#include "mqttlib/MessageProperties.hpp"
#include "mqttlib/Helpers.hpp"
#include "mqttlib/Callbacks.hpp"
#include "mqttlib/DisconnectReasons.hpp"
#include "mqttlib/Storage.hpp"

#include "mqttlib/Packets/Packet.hpp"
#include "mqttlib/Packets/ConnAckPacket.hpp"
#include "mqttlib/Packets/PingRespPacket.hpp"
#include "mqttlib/Packets/SubAckPacket.hpp"
#include "mqttlib/Packets/UnsubAckPacket.hpp"
#include "mqttlib/Packets/PublishPacket.hpp"
#include "mqttlib/Packets/PubRelPacket.hpp"
#include "mqttlib/Packets/PubAckPacket.hpp"
#include "mqttlib/Packets/PubRecPacket.hpp"
#include "mqttlib/Packets/PubCompPacket.hpp"

#include "mqttlib/Packets/Out/Connect.hpp"
#include "mqttlib/Packets/Out/PingReq.hpp"
#include "mqttlib/Packets/Out/PubAck.hpp"
#include "mqttlib/Packets/Out/Disconn.hpp"
#include "mqttlib/Packets/Out/Subscribe.hpp"
#include "mqttlib/Packets/Out/Unsubscribe.hpp"
#include "mqttlib/Packets/Out/Publish.hpp"

class AsyncMqttClient {
  public:
  AsyncMqttClient();
  ~AsyncMqttClient();

  AsyncMqttClient& setKeepAlive(uint16_t keepAlive);
  AsyncMqttClient& setClientId(const char* clientId);
  AsyncMqttClient& setCleanSession(bool cleanSession);
  AsyncMqttClient& setMaxTopicLength(uint16_t maxTopicLength);
  AsyncMqttClient& setCredentials(const char* username, const char* password = nullptr);
  AsyncMqttClient& setWill(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr,
                           size_t length = 0);
  AsyncMqttClient& setServer(IPAddress ip, uint16_t port);
  AsyncMqttClient& setServer(const char* host, uint16_t port);
#if ASYNC_TCP_SSL_ENABLED
  AsyncMqttClient& setSecure(bool secure);
  AsyncMqttClient& addServerFingerprint(const uint8_t* fingerprint);
#endif

  AsyncMqttClient& onConnect(AsyncMqttClientInternals::OnConnectUserCallback callback);
  AsyncMqttClient& onDisconnect(AsyncMqttClientInternals::OnDisconnectUserCallback callback);
  AsyncMqttClient& onSubscribe(AsyncMqttClientInternals::OnSubscribeUserCallback callback);
  AsyncMqttClient& onUnsubscribe(AsyncMqttClientInternals::OnUnsubscribeUserCallback callback);
  AsyncMqttClient& onMessage(AsyncMqttClientInternals::OnMessageUserCallback callback);
  AsyncMqttClient& onPublish(AsyncMqttClientInternals::OnPublishUserCallback callback);

  bool connected() const;
  void connect();
  void disconnect(bool force = false);
  uint16_t subscribe(const char* topic, uint8_t qos);
  uint16_t unsubscribe(const char* topic);
  uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0,
                   bool dup = false, uint16_t message_id = 0);
  bool clearQueue();  // Not MQTT compliant!

  const char* getClientId() const;

  private:
  AsyncClient _client;
  AsyncMqttClientInternals::OutPacket* _head;
  AsyncMqttClientInternals::OutPacket* _tail;
  size_t _sent;
  enum { CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED } _state;
  AsyncMqttClientDisconnectReason _disconnectReason;
  uint32_t _lastClientActivity;
  uint32_t _lastServerActivity;
  uint32_t _lastPingRequestTime;

  char _generatedClientId[18 + 1];  // esp8266-abc123 and esp32-abcdef123456
  IPAddress _ip;
  const char* _host;
  bool _useIp;
#if ASYNC_TCP_SSL_ENABLED
  bool _secure;
#endif
  uint16_t _port;
  uint16_t _keepAlive;
  bool _cleanSession;
  const char* _clientId;
  const char* _username;
  const char* _password;
  const char* _willTopic;
  const char* _willPayload;
  uint16_t _willPayloadLength;
  uint8_t _willQos;
  bool _willRetain;

#if ASYNC_TCP_SSL_ENABLED
  std::vector<std::array<uint8_t, SHA1_SIZE>> _secureServerFingerprints;
#endif

  std::vector<AsyncMqttClientInternals::OnConnectUserCallback> _onConnectUserCallbacks;
  std::vector<AsyncMqttClientInternals::OnDisconnectUserCallback> _onDisconnectUserCallbacks;
  std::vector<AsyncMqttClientInternals::OnSubscribeUserCallback> _onSubscribeUserCallbacks;
  std::vector<AsyncMqttClientInternals::OnUnsubscribeUserCallback> _onUnsubscribeUserCallbacks;
  std::vector<AsyncMqttClientInternals::OnMessageUserCallback> _onMessageUserCallbacks;
  std::vector<AsyncMqttClientInternals::OnPublishUserCallback> _onPublishUserCallbacks;

  AsyncMqttClientInternals::ParsingInformation _parsingInformation;
  AsyncMqttClientInternals::Packet* _currentParsedPacket;
  uint8_t _remainingLengthBufferPosition;
  char _remainingLengthBuffer[4];

  std::vector<AsyncMqttClientInternals::PendingPubRel> _pendingPubRels;

  SemaphoreHandle_t _xSemaphore = nullptr;

  void _clear();
  void _freeCurrentParsedPacket();

  // TCP
  void _onConnect();
  void _onDisconnect();
  // void _onError(int8_t error);
  // void _onTimeout();
  void _onAck(size_t len);
  void _onData(char* data, size_t len);
  void _onPoll();

  // QUEUE
  void _insert(AsyncMqttClientInternals::OutPacket* packet);    // for PUBREL
  void _addFront(AsyncMqttClientInternals::OutPacket* packet);  // for CONNECT
  void _addBack(AsyncMqttClientInternals::OutPacket* packet);   // all the rest
  void _handleQueue();
  void _clearQueue(bool keepSessionData);

  // MQTT
  void _onPingResp();
  void _onConnAck(bool sessionPresent, uint8_t connectReturnCode);
  void _onSubAck(uint16_t packetId, char status);
  void _onUnsubAck(uint16_t packetId);
  void _onMessage(char* topic, char* payload, uint8_t qos, bool dup, bool retain, size_t len, size_t index,
                  size_t total, uint16_t packetId);
  void _onPublish(uint16_t packetId, uint8_t qos);
  void _onPubRel(uint16_t packetId);
  void _onPubAck(uint16_t packetId);
  void _onPubRec(uint16_t packetId);
  void _onPubComp(uint16_t packetId);

  void _sendPing();
};
