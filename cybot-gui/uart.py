import serial, time

class UartConnection:
    def __init__(self):
        """establish UART connection to cyBot and read output"""
        self.ser = serial.serial_for_url("socket://192.168.1.1:288", baudrate=115200)
        self.waiting = False

    def data_stream(self, player):
        """interpret information from uart"""
        while True:
            data = self.ser.read_until().decode("utf-8").split(",")
            print(data)
            if "UPD" == data[0]:
                player.update(float(data[1]), float(data[2]))
                data[3] = int(data[3])
                if data[3] > 0:
                    player.bumper = "right"
                elif data[3] < 0:
                    player.bumper = "left"
                elif data[3] == 0:
                    player.bumper = ""
            elif "SCN" == data[0]:
                player.scan(float(data[1]),float(data[2]),float(data[3]))

    def send_data(self,data):
        for x in range(len(data)):
            self.ser.write(bytes(data[x], 'ascii'))

        