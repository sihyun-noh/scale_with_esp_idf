#ifndef _IP_ADDRESS_H_
#define _IP_ADDRESS_H_

class IPAddress {
  private:
  union {
    uint8_t bytes[4];
    uint32_t dword;
  } _address;

  uint8_t* raw_address() { return _address.bytes; }

  public:
  // Constructor
  IPAddress();

  IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
  IPAddress(uint32_t address);
  IPAddress(const uint8_t* address);
  virtual ~IPAddress() {}

  bool fromString(const char* address);

  // Overloaded cast operator to allow IPAddress objects to be used where a pointer
  // to a four-byte uint8_t array is expected
  operator uint32_t() const { return _address.dword; }
  bool operator==(const IPAddress& addr) const { return _address.dword == addr._address.dword; }
  bool operator==(const uint8_t* addr) const;

  // Overloaded index operator to allow getting and setting individual octets of the address
  uint8_t operator[](int index) const { return _address.bytes[index]; }
  uint8_t& operator[](int index) { return _address.bytes[index]; }

  // Overloaded copy operators to allow initialisation of IPAddress objects from other types
  IPAddress& operator=(const uint8_t* address);
  IPAddress& operator=(uint32_t address);
};

// changed to extern because const declaration creates copies in BSS of INADDR_NONE for each CPP unit that includes it
extern IPAddress INADDR_NONE;

#endif /* _IP_ADDRESS_H_ */
