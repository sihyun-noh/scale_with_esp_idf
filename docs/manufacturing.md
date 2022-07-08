# How to generate the manufacturing data for SmartFarm device

## Manufacturing data items
+ Serial Number
+ MAC Address
+ HW version
+ AWS Certification (If needed)
+ Model Name
+ Power Mode 
  + B (Battery)
  + P (Plug)

## Serial Numbering Scheme
The serial number is a 14 digits number that must following the below format:

(e.g.) : 102123STH00001

+ Supplier ID

  Digit 1-2 (##) is the unique identifier for each ODM
  
  10 : Currently ODM code (Greenlabs).

+ Year of Mfg

  Digit 3-4 (##) is the year of manufacture. For example, 21 for the year of 2021

+ Week of Mfg

  Digit 5-6 (##) is the week of manufacture. For example, 06 for the 6 weeks of manufacture year. (01 ~ 52 weeks)

+ Product(Model) Code

  Digit 7-9 (xxx) is the product code.

  + Sensor : S
    + Sub type : 
      - TH : 온습도
      - SE : 지온/지습/토지EC
      - WE : 용액EC
      - WP : 용액pH
      - WS : 기상대
      - CO : Co2
 
  + Actuator : A
    + Sub type : 
      - MT : Motor
      - SW : Switch (On/Off)

    

+ Unique Sequence Identifier

  Digit 10-14(#####) is the unique sequence identifier. It consists of hexadecimal numbers, from 0 to 9.
  Starting from 00001 and sequencing up to FFFFF.

## MAC address

## HW version

## Model Name
(e.g.) GLSTH 

+ Prefix : GL
+ Product(Model) code : STH --> sensor of Temperature and Humidity.

## ESP32 Manufacturing Utility

You can use the esp32 manufacturing utility to easily create manufacturing data for multiple devices.

### Workflow

[CSV Configuration] + [Master Value CSV file] ---> [Manufacturing Data files, ...]

### CSV Configuration File
This configuration file contains the configuration of the device to be flashed.

The data in the configuration file has the following format (the REPEAT tag is optional):
```
name1,namespace, <-- First entry should be of type "namespace"
key1,type1,encoding1
key2,type2,encoding2,REPEAT
key3,type3,encoding3
```

Each line should have three parameters: `key,type,encoding`, separated by a comma. If the `REPEAT` tag is present, the value corresponding to this key in the master value CSV file will be the same for all devices.


The configuration file below is a real manufacturing data that can be applied to the device.

NOTE : syscfg_get only can read the string format data from the cfg and mfg data section, so we need to use the string format in configuration file.

<sample_config.csv>
```
mfg,namespace, <-- First entry should be of type "namespace" and name is mfg, since syscfg_get can read manufacture data from the "mfg" namespace.
serial_number,data,string
mac_address,data,string
hw_version,data,string,REPEAT
model_name,data,string,REPEAT
power_mode,data,string,REPEAT
```

NOTE :  
```
Make sure there are no spaces:
  * before and after ","
  * at the end of each line in a CSV file
  * No blank line at the end of file
```

### Master Value CSV File
This master csv file contains details data of the devices to be flashed. Each data of line in this file corresponds to a device instance.
 
The data in the master value CSV file has the following format:
```
key1,key2,key3,...
value1,value2,value3,...
```

Below is a sample example of a master value CSV file:

You don't need to input the data that has the REPEAT tag, because this data field will be generated the same value automatically.

<sampel_master.csv>
```
id,serial_number,mac_address,hw_version,model_name,power_mode
1,102123STH00001,0EADBEEFFEE9,A0,GLSTH,B
2,102123STH00002,0EADBEEFFEEA,,,
3,102123STH00003,0EADBEEFFEEB,,,
4,102123STH00004,0EADBEEFFEEC,,,
...
```

### Running the utility

Usage : 
```bash
python3 esp-idf/tools/mass_mfg/mfg_gen.py generate sample_config.csv sample_master.csv sample 0x3000

0x3000 is a manufacturing data size what you want to generate.
This size must be multiple of 4096
```

After running the command above, it makes the two directory as following : 

```bash
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  .
drwxr-xr-x   - jeffyoon staff  8 Jun 16:58  ..
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  bin
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  csv
.rw-r--r-- 158 jeffyoon staff  9 Jun 18:22  sample_config.csv
.rw-r--r-- 202 jeffyoon staff  9 Jun 18:22  sample_master.csv
.rw-r--r-- 220 jeffyoon staff  9 Jun 18:22  sample_master_created.csv

./bin:
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  .
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  ..
.rw-r--r-- 12k jeffyoon staff  9 Jun 18:22  sample-1.bin
.rw-r--r-- 12k jeffyoon staff  9 Jun 18:22  sample-2.bin
.rw-r--r-- 12k jeffyoon staff  9 Jun 18:22  sample-3.bin
.rw-r--r-- 12k jeffyoon staff  9 Jun 18:22  sample-4.bin

./csv:
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  .
drwxr-xr-x   - jeffyoon staff  9 Jun 18:22  ..
.rw-r--r-- 198 jeffyoon staff  9 Jun 18:22  sample-1.csv
.rw-r--r-- 198 jeffyoon staff  9 Jun 18:22  sample-2.csv
.rw-r--r-- 198 jeffyoon staff  9 Jun 18:22  sample-3.csv
.rw-r--r-- 198 jeffyoon staff  9 Jun 18:22  sample-4.csv
```


As you can see, the sample-1~4.bin is the manufacturing data and it can download to the `mfg` partition area using the following command 

```bash
esptool.py -p /dev/tty.usbserial-0001 write_flash 0x144000 sample-1.bin

0x144000 is a mfg partition address that is described in partition.csv
```


Reference site link : [[https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/mass_mfg.html]]


