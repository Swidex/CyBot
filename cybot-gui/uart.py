import serial, time

class UartConnection:
    def __init__(self):
        self.real_cybot = "socket://192.168.1.1:288"
        self.sim = "socket:10.24.85.217:50000"
        """establish UART connection to cyBot and read output"""
        self.ser = serial.serial_for_url(self.sim, baudrate=115200)
        self.waiting = False

    def data_stream(self, player):
        """interpret information from uart"""
        while True:
            data = self.ser.read_until().decode("utf-8").split(",")
            print(data)
            if "MOV" == data[0]:
                player.update(0, float(data[1]))
                self.waiting = False
            elif "TRN" == data[0]:
                player.update(float(data[1]), 0)
                self.waiting = False
            elif "SCN" == data[0]:
                player.scan(float(data[1]),float(data[2]),float(data[3]))
            elif "BMP" == data[0]:
                data[1] = int(data[1])
                if data[1] > 0:
                    player.bumper = "right"
                elif data[1] < 0:
                    player.bumper = "left"
                elif data[1] == 0:
                    player.bumper = ""

    def send_data(self,data):
        for x in range(len(data)):
            self.ser.write(bytes(data[x], 'ascii'))
            time.sleep(0.1) # wait 1 ms

        