import serial, time

class UartConnection:
    def __init__(self):
        """establish UART connection to cyBot and read output"""
        self.ser = serial.serial_for_url("socket://192.168.1.1:288", baudrate=115200)

    def data_stream(self, player):
        """interpret information from uart"""
        while True:
            data = self.ser.read_until().decode("utf-8").split(",")
            print(data)
            if "MOV" == data[0]:
                player.move(float(data[1]))
                self.waiting = False
            elif "TRN" == data[0]:
                player.turn(float(data[1]))
                self.waiting = False
            elif "SCN" == data[0]:
                player.scan(int(data[1]),float(data[2]),float(data[3]))
                self.waiting = False
            elif "BMP" == data[0]:
                data[1] = int(data[1])
                if data[1] > 0:
                    player.bumper = "right"
                elif data[1] < 0:
                    player.bumper = "left"
                elif data[1] == 0:
                    player.bumper = ""
            elif "MAN" == data[0]:
                player.manual = True
            elif "AUT" == data[0]:
                player.manual = False

    def send_data(self,data):
        for x in range(len(data)):
            self.ser.write(bytes(data[x], 'ascii'))
            time.sleep(0.1) # wait 1 ms

        