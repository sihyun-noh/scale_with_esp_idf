The Modbus Protocol was originally developed by Modicon (now part of Schneider Electric).

# Modbus message structure
ADU (Application Data Unit)

<---------------------------------------><br>
| Slave ID | Function Code | Data | CRC |<br>
             <---- PDU(Protocol) ---->

PDU (Protocol Data Unit)

Modbus is a request-response protocol where:

 - The client(master) sends a request to a Modbus device(slave).
 - The Modbus device sends a response.


## Modbus Register

| Register Number  | Register Addr Hex   | Access Type  | Name                              | Type   | Function Code |
| ---------------- | ------------------- | ------------ | --------------------------------- | ------ |---------------|
| 00001 ~ 09999    | 0000 to 270E        | read-write   | Discrete Output Coils             | DO     |   1, 5, 15    |
| 10001 ~ 19999    | 0000 t0 270E        | read         | Discrete Input Contacts           | DI     |      2        |
| 30001 ~ 39999    | 0000 to 270E        | read         | Analog Input Registers            | AI     |      4        |
| 40001 ~ 49999    | 0000 t0 270E        | read-write   | Analog Output Holding Registers   | AO     |   3, 6, 16    |

Coils and Discrete Inputs = 1 bit or bool

Register = 16 bits (2 bytes)

## Modbus Data Types

The Modbus protocol supports these data types:

| Data Type         | Access       | Description               |
| ----------------- | ------------ | ------------------------- |
| Coil              | Read-write   | Signle bit outputs        |
| Discrete Input    | Read-only    | Single bit inputs         |
| Input Register    | Read-only    | 16-bit input registers    |
| Holding Register  | Read-write   | 16-bit output registers   |


e.g.) 11 03 006B 0003 7687 <br>
Modbus message use the register address to communicate between master and salve.<br>
AO Holding Register 의 첫번째 레지스터 번호는 40001 이지만 주소는 0000 입니다.<br>
이 두가지 사이의 차이점은 "오프셋" 입니다.<br>
위의 패킷의 예제는 다음과 같이 설명 할 수 있습니다.<br>
장치 17의 주소로 레지스터 40108 ~ 40110 에서 홀딩 레지스터의 AI 값을 얻기 위한 요청입니다.<br>

|   Packet | Description                                                          |
|  ------- | -------------------------------------------------------------------- |
|       11 | Slave ID (17 = 11 hex)                                               |
|       03 | Function Code                                                        |
|     006B | First register address (40108 - 40001 = 107 = 6B hex)                |
|     0003 | Register count that need to read (From 40108 to 40110) 3-Registers   |
|     7687 | CRC checksum                                                         |

## Funcion Code
| Function Code | What the Function Does                            | Value Type | Access Type |
| ------------- | ------------------------------------------------- | ---------- | ----------- |
| 01 (0x01)     | Read DO / Read Coil status                        | Discrete   | Read        |
| 02 (0x02)     | Read DI / Read Input status                       | Discrete   | Read        |
| 03 (0x03)     | Read AO / Read Holding Registers                  | 16 bit     | Read        |
| 04 (0x04)     | Read AI / Read Input Registers                    | 16 bit     | Read        |
| 05 (0x05)     | Write one DO / Force Single Coil                  | Discrete   | Write       |
| 06 (0x06)     | Write one AO / Preset Single Register             | 16 bit     | Write       |
| 15 (0x0F)     | Multiple DO recodring / Force Multiple Coils      | Discrete   | Write       |
| 16 (0x10)     | Multiple AO recodring / Preset Multiple Registers | 16 bit     | Write       |


## Packet Structure
### Request frame
| Slave ID | Function Code | Address (High/Low byte) | Request Data Len(High/Low byte) | CRC |
|----------|---------------|-------------------------|---------------------------------|-----|
|       01 |            03 | 00 , 6B                 | 00, 03                          | CRC |

+ 01: 슬레이브 1번에게(1번 장치에게)
+ 03 : 3번 함수를 이용하여 40001~49999사이에 있는 레지스터 중
+ 00 6B : 6B번 주소부터
+ 00 03 : 3개 레지스터 값을 요구

### Response frame (OK)
| Slave ID | Function Code | Read Len(byte) | Read Data              | CRC |
|----------|---------------|----------------|------------------------|-----|
|       01 |            03 |             06 | 13, 12, 3D, 12, 40, 4F | CRC |

### Response frame (Error)
| Slave ID | Function Code | Exception Code | CRC |
|----------|---------------|----------------|-----|
|       01 |            83 |             04 | CRC |

## 풍향 RK110-02.pdf
Communication Protocol (MODBUS)

Transmission mode : MODBUS-RTU, Baud Rate: 9600bps, Data bits: 8, Stop bit: 1, Check bit: No

Slave address : the factory default is 01H (set according to the need, 00H to FFH)

### The 03H Function code example : Read the Wind direction
Host Scan Order (Slave address: 0x01)<br>
| Slave ID | Function Code                   | Address | Data Len | CRC   |
|----------|---------------------------------|---------|----------|-------|
|       01 | 03 (Read AO, Holding Registers) | 00 00   | 00 01    | 84 0A |


Slave Response<br>
| Slave ID | Function Code                  | Read Len | Data  | CRC   |
|----------|--------------------------------|----------|-------|-------|
|       01 | 03(Read AO, Holding Registers) |       02 | 00 B4 | B8 33 |

Wind direction : (00B4)H = (180)D = 180

### The 10H Function code example : Modify the slave address
Host Scan Order (Changed to 01H, read and write address must be 00H)
| Slave ID | Function Code | Slave Address | CRC   |
|----------|---------------|---------------|-------|
|       00 |            10 |            01 | DB C0 |

Slave Response
| Slave ID | Funcion Code | CRC   |
|----------|--------------|-------|
|       00 |           10 | 00 7C |

### The 20H Function code example : Read the slave address (fixed command, ensure that no other devices on the bus)
Host Scan Order
| Slave ID | Funcion Code | CRC   |
|----------|--------------|-------|
|       00 |           20 | 00 68 |

Slave Response (addr = 01H)
| Slave ID | Funcion Code | Slave addr | CRC   |
|----------|--------------|------------|-------|
|       00 |           20 |         01 | A9 C0 |

## PH 용액 RK500-02.pdf
Communication Protocol (MODBUS)

Transmission mode : MODBUS-RTU, Baud Rate: 9600bps, Data bits: 8, Stop bit: 1, Check bit: No

Slave address : the factory default is 01H (set according to need, 00H to FFH)

### The 03H Function code example : Read the PH
Host Scan Order (Slave address: 0x01)
| Slave ID | Function Code | Address | Data | CRC |
|----------|---------------|---------|------|-----|
|       01 |           03  | 00 00   | 00 01| 840A|

Slave Respose
| Slave ID | Function Code | Read Len | Data | CRC |
|----------|---------------|----------|------|-----|
|       01 |           03  |       02 | 02 A0| B95C|

PH: (0240)H = (576)D , 576 / 100 = 5.76


### ESP32 Modbus master data dictionary description
| Field         | Description           | Deatailed Information                                            |
| --------------|-----------------------|------------------------------------------------------------------|
| cid           | Charcteristic ID      | The identifier of characteristic (must be unique)                |
| param_key     | Charcteristic Name    | String description of the characteristic                         |
| param_units   | Charcteristic Units   | Physical Units of the characteristic                             |
| mb_slave_addr | Modbus Slave Address  | The short address of the device with correspond parameter UID    |
| mb_param_type | Modbus Register Type  | MB_PARAM_INPUT,MB_PARAM_HOLDING,MB_PARAM_COIL,MB_PARAM_DISCRETE  |
| mb_reg_start  | Modbus Register Start | Relative register address of characteristic in the register area |
| mb_size       | Modbus Register Size  | Length of characteristic in registers                            |
| param_offset  | Instance Offset       |                                                                  |
| param_type    | Data Type             |                                                                  |
| param_size    | Data Size             |                                                                  |
| param_opts    | Parameter Options     |                                                                  |
| access        | Parameter access type |                                                                  |

### Examle Register mapping table of Modbus slave
| CID | Register | Length | Range    | Type  | Units       | Description                                 |
| --- |----------|--------|----------|-------|-------------|---------------------------------------------|
|   0 |    30000 |      4 | MAX_UINT | U32   | Not defined | Serial number of device (4bytes), read-only |
|   1 |    30002 |      2 | MAX_UINT | U16   | Not defined | Software version (4bytes) read-only         |
|   2 |    40000 |      4 | -20..40  | FLOAT | DegC        | Room temperature in DegC                    |

During initialization of the Modbus stack, a pointer to the Data Dictionary (called descriptor) must be provided as the parameter of the function below.<br>
mbc_master_set_descriptor(): Initialization of master descriptor.

```c
// Enumeration of modbus slave addresses accessed by master device
enum {
    MB_DEVICE_ADDR1 = 1,
    MB_DEVICE_ADDR2,
    MB_SLAVE_COUNT
};

// Enumeration of all supported CIDs for device
enum {
    CID_SER_NUM1 = 0,
    CID_SW_VER1,
    CID_TEMP_DATA_1,
    CID_SER_NUM2,
    CID_SW_VER2,
    CID_TEMP_DATA_2
};

// Example Data Dictionary for Modbus parameters in 2 slaves in the segment
mb_parameter_descriptor_t device_parameters[] = {
    // CID, Name, Units, Modbus addr, register type, Modbus Reg Start Addr, Modbus Reg read length,
    // Instance offset (NA), Instance type, Instance length (bytes), Options (NA), Permissions
    { CID_SER_NUM1, STR("Serial_number_1"), STR("--"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 0, 2,
                    0, PARAM_TYPE_U32, 4, OPTS( 0,0,0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_SW_VER1, STR("Software_version_1"), STR("--"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 2, 1,
                    0, PARAM_TYPE_U16, 2, OPTS( 0,0,0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_TEMP_DATA_1, STR("Temperature_1"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 0, 2,
                    0, PARAM_TYPE_FLOAT, 4, OPTS( 16, 30, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_SER_NUM2, STR("Serial_number_2"), STR("--"), MB_DEVICE_ADDR2, MB_PARAM_INPUT, 0, 2,
                    0, PARAM_TYPE_U32, 4, OPTS( 0,0,0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_SW_VER2, STR("Software_version_2"), STR("--"), MB_DEVICE_ADDR2, MB_PARAM_INPUT, 2, 1,
                    0, PARAM_TYPE_U16, 2, OPTS( 0,0,0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_TEMP_DATA_2, STR("Temperature_2"), STR("C"), MB_DEVICE_ADDR2, MB_PARAM_HOLDING, 0, 2,
                    0, PARAM_TYPE_FLOAT, 4, OPTS( 20, 30, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
};
// Calculate number of parameters in the table
uint16_t num_device_parameters = (sizeof(device_parameters) / sizeof(device_parameters[0]));
ESP_ERROR_CHECK(mbc_master_set_descriptor(&device_parameters[0], num_device_parameters));
```

+ UART APIs for implementation of RS-485
```c

uart_config_t uart_config = {
.baud_rate = baudrate,
.data_bits = databits,
.parity = parity,
.stop_bits = UART_STOP_BITS_1,
.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
.rx_flow_ctrl_thread = 2,
.use_ref_tick = UART_SCLK_APB,
};

// Set UART config
uart_param_config(port_num, &uart_config);

// Install UART driver, and get the queue.
uart_driver_install(port_num, MB_SERIAL_BUF_SIZE, MB_SERIAL_BUF_SIZE, MB_QUEUE_LENGTH, &uart_queue, MB_PORT_SERIAL_ISR_FLAG);

// Set timeout for TOUT intereupt (T3.5 modbus time)
uart_set_rx_timeout(port_num, MB_SERIAL_TOUT);

// Set always timeout flag to trigger timeout intereupt event after rx fifo full.
uart_set_always_rx_timeout(port_num, true);


uart_write_bytes();

uart_read_bytes();

serial_rx_poll() {
    while (read_state && (cnt++ <= MB_SERIAL_BUF_SIZE)) {
       read_state = mb_frame_receive();
    }

    uart_flush_input(port_num);
}
```
