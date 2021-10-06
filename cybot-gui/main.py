import serial.serialutil
import pygame, math, sys, uart, serial, threading

# Constants
BLUE = (25, 25, 200)
PURPLE = (200, 25, 200)
BLACK = (23, 23, 23)
WHITE = (254, 254, 254)
RED = (200, 25, 25)
GREEN = (25, 200, 25)

CM_TO_PX = 1.724
SCREEN_WIDTH = 1280
SCREEN_HEIGHT = 720

def polar_to_cart(deg, amt):
    """convert polar coordinates to cartesian"""
    x = float(amt) * math.sin(math.radians(int(deg)))
    y = float(amt) * math.cos(math.radians(int(deg)))
    return x,y

def ir_to_cm(val):
    """convert ir values into centimeters"""
    return 2600000 * pow(val,-1.56)

class Player():
    """cybot player class"""

    def __init__(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.servo_pos = 0
        self.rot = 90
        self.size = 34.8 * CM_TO_PX
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)
        self.progress_bar = pygame.Rect(0,0,0,0)

    def move(self, reverse=False):
        """move forward/backwards"""
        if reverse:
            cybot_uart.send_byte('s') # drive backwards
        else:
            cybot_uart.send_byte('w') # drive forward

        amt = float(cybot_uart.read_line()) # wait for sensor distance for accuracy
        x, y = polar_to_cart(self.rot, amt * CM_TO_PX)
        self.x += x
        self.y += y
        self.rect = pygame.Rect(self.x-(self.size/2),self.y-(self.size/2),self.size,self.size)

    def turn(self, clockwise=False):
        """turn"""
        if clockwise:
            cybot_uart.send_byte('d') # driveLeft
        else:
            cybot_uart.send_byte('a') # turnRight
        angle = cybot_uart.read_line() # wait for sensor distance for accuracy
        self.rot += int(angle)

    def clear(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.rot = 0
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)
        objects.clear()

    def scan(self):
        """scan 180 degrees infront"""
        cybot_uart.send_byte('m')
        while True:
            # read from uart
            data = cybot_uart.read_line()

            # break if done scanning
            if data == "Complete\n":
                self.progress_bar = pygame.Rect(0,0,0,0)
                break
            
            self.progress_bar = pygame.Rect(SCREEN_WIDTH/4, SCREEN_HEIGHT - 100, float(int(self.servo_pos) / 180) * (SCREEN_WIDTH / 2), 50)

            # split data into readings
            theta, ir, ping = data.rstrip().split(',')
            self.servo_pos = int(theta)

            distAvg = (float(ir) + float(ping)) / 2

            # create new objects at coordinates
            if float(ir) < 100: 
                irx, iry = polar_to_cart(int(self.servo_pos) - 90 + self.rot, distAvg * CM_TO_PX)
                offsetx, offsety = polar_to_cart(int(self.rot), float(34.8 / 2) * CM_TO_PX)
                objects.append(Object(self.x+irx+offsetx, self.y+iry+offsety))    

class Object():
    """class to hold scan data"""

    def __init__(self,x,y):
        self.x = x
        self.y = y

    def compare_to(self,object):
        dist = math.sqrt(pow((self.x - object.x),2) + pow(self.y - object.y,2))
        if dist < 25:
            return True
        return False

# initalize serial connection
try:
    cybot_uart = uart.UartConnection()
except serial.serialutil.SerialException:
    print("ERR: Failed to connect to CyBot.")
    sys.exit()

# initalize pygame
pygame.init()
screen = pygame.display.set_mode([SCREEN_WIDTH, SCREEN_HEIGHT])
objects = []
player = Player()

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            try:
                sys.exit()
            finally:
                main = False

        if event.type == pygame.KEYDOWN:
            if event.key == ord('q'): # quit
                pygame.quit()
                try:
                    sys.exit()
                finally:
                    main = False
            if event.key == ord('a'): # turn left
                player.turn()
            if event.key == ord('d'): # turn right
                player.turn(True)
            if event.key == ord('w'): # move forward
                player.move()
            if event.key == ord('s'): # move backwards
                player.move(True)
            if event.key == ord('m'): # scan
                scanThread = threading.Thread(target=player.scan)
                scanThread.daemon = True
                scanThread.start()
            if event.key == ord('k'): # clear
                objects.clear()

    # fill background
    screen.fill(BLACK)

    # draw cybot
    xe, ye = polar_to_cart(player.rot + player.servo_pos - 90, player.size / 2)
    pygame.draw.circle(screen, WHITE, (player.x, player.y), player.size / 2)
    pygame.draw.arc(screen, GREEN, player.rect, math.radians(player.rot-180), math.radians(player.rot), 10)
    pygame.draw.line(screen, BLUE, (player.x, player.y), (player.x + xe, player.y + ye), 5)
    pygame.draw.rect(screen, WHITE, player.progress_bar)

    # draw sensor data
    for object in objects:
        pygame.draw.circle(screen, RED, (object.x, object.y), 2)

    

    pygame.display.flip()