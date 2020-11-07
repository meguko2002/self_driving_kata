import serial,os
import time

ser = serial.Serial()
ser.baudrate = 9600
dir = '/dev'
for file in os.listdir(dir):
    if "tty.usbmodem" in file:
        ser.port = os.path.join(dir, file)
        ser.open()


val = 0
while True:
    valByte = val.to_bytes(1, 'big')
    # valByte = val.to_bytes(2, 'little')
    ser.write(valByte)
    print(val)
    time.sleep(1)
    val += 1
print ( "Close Port" )