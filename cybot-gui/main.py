import pygame, math, sys, uart, serial

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
    x = amt * math.sin(math.radians(deg))
    y = amt * math.cos(math.radians(deg))
    return x,y

def ir_to_cm(val):
    """convert ir values into centimeters"""
    return 6410000 * pow(val,-1.69)

class Player():
    """cybot player class"""

    def __init__(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.rot = 90
        self.size = 34.8 * CM_TO_PX
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)

    def move(self, reverse=False):
        """move forward/backwards"""
        if reverse:
            cybot_uart.send_byte('s') # drive backwards
        else:
            cybot_uart.send_byte('w') # drive forward

        amt = cybot_uart.read_line() # wait for sensor distance for accuracy
        if reverse: amt * (-1) # make negative if going backwards
        x, y = polar_to_cart(self.rot, amt)
        self.x += x
        self.y += y
        self.rect = pygame.Rect(self.x-(self.size/2),self.y-(self.size/2),self.size,self.size)

    def turn(self, clockwise):
        """turn"""
        if clockwise:
            cybot_uart.send_byte('d') # driveLeft
        else:
            cybot_uart.send_byte('a') # turnRight
        angle = cybot_uart.read_line() # wait for sensor distance for accuracy
        self.rot = angle

    def scan(self):
        """scan 180 degrees infront"""
        while True:
            # read from uart
            data = cybot_uart.read_line()

            # break if done scanning
            if data == "Complete\n": break

            # split data into readings
            theta, ir, ping = data.rstrip().split(',')

            # progress bar
            print("Scan Progress:",int(theta/180)*100,"%")

            # convert to cartesian coordinates
            irx, iry = polar_to_cart(int(theta) - 90 + self.rot, ir_to_cm(int(ir)) * CM_TO_PX)
            pgx, pgy = polar_to_cart(int(theta) - 90 + self.rot, int(ping) * CM_TO_PX)

            # create new objects at coordinates
            objects.append(Object(self.x+irx, self.y+iry,self.x+pgx,self.y+pgy))

class Object():
    """class to hold scan data"""
    def __init__(self,irx,iry,pgx,pgy):
        self.irx = irx
        self.iry = iry
        self.pgx = pgx
        self.pgy = pgy

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
                player.scan()

    # fill background
    screen.fill(BLACK)

    # draw cybot
    xe, ye = polar_to_cart(player.rot, player.size / 2)
    pygame.draw.circle(screen, WHITE, (player.x, player.y), player.size / 2)
    pygame.draw.arc(screen, GREEN, player.rect, math.radians(player.rot-180), math.radians(player.rot), 10)
    pygame.draw.line(screen, BLUE, (player.x, player.y), (player.x + xe, player.y + ye), 5)

    # draw sensor data
    for object in objects:
        pygame.draw.circle(screen, RED, (object.irx, object.iry), 3)
        pygame.draw.circle(screen, PURPLE, (object.pgx, object.pgy), 3)
        pygame.draw.line(screen, WHITE, (object.irx, object.iry), (object.pgx, object.pgy), 1)

    pygame.display.flip()