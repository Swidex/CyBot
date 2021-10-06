import serial

class UartConnection:
    def __init__(self):
        """establish UART connection to cyBot and read output"""
        self.ser = serial.serial_for_url("socket://192.168.1.1:288") 

    def read_line(self):
        """read until new line is detected"""
        data = ""
        last_byte = ''
        while True:
            if last_byte == "\n":
                return data
            last_byte = self.ser.read(1).decode("utf-8")
            data += last_byte
            self.ser.flushOutput()

    def send_byte(self,data):
        data = bytes(data, 'utf-8')
        self.ser.write(data)
        