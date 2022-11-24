set(srcs
  "AsyncTCP.cpp"
  "AsyncMqttClient.cpp"
  "IPAddress.cpp"
  "mqttlib/Packets/ConnAckPacket.cpp"
  "mqttlib/Packets/PingRespPacket.cpp"
  "mqttlib/Packets/PubAckPacket.cpp"
  "mqttlib/Packets/PubCompPacket.cpp"
  "mqttlib/Packets/PublishPacket.cpp"
  "mqttlib/Packets/PubRecPacket.cpp"
  "mqttlib/Packets/PubRelPacket.cpp"
  "mqttlib/Packets/SubAckPacket.cpp"
  "mqttlib/Packets/UnsubAckPacket.cpp"
  "mqttlib/Packets/Out/Connect.cpp"
  "mqttlib/Packets/Out/Disconn.cpp"
  "mqttlib/Packets/Out/OutPacket.cpp"
  "mqttlib/Packets/Out/PingReq.cpp"
  "mqttlib/Packets/Out/PubAck.cpp"
  "mqttlib/Packets/Out/Publish.cpp"
  "mqttlib/Packets/Out/Subscribe.cpp"
  "mqttlib/Packets/Out/Unsubscribe.cpp"
  )

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS "." "mqttlib" "mqttlib/Packets" "mqttlib/Packets/Out"
  REQUIRES freertos lwip syslog esp_timer asio esp_hw_support
)
