import serial.serialutil
import pygame, math, sys, uart, serial, threading, time

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

HEADER_CODES = ["BRK\n","CMP\n","SCN\n","MOV\n","TRN\n","CAL\n","SRV\n"]

# ERROR MSGS
NO_HEADER_RECEIVED = "ERR: Did not receive header response."
PREMATURE_FUNC_END = "ERR: Ended prematurely due to interruption to command."

def polar_to_cart(deg, amt):
    """convert polar coordinates to cartesian"""
    x = float(amt) * math.sin(math.radians(int(deg)))
    y = float(amt) * math.cos(math.radians(int(deg)))
    return x,y

def ir_to_cm(val):
    """convert ir values into centimeters"""
    return 2600000 * pow(float(val),-1.56)

def get_dist(x1,x2,y1,y2):
    return abs(math.sqrt(pow(x1-x2,2) + pow(y1-y2,2)))

class Player():
    """cybot player class"""

    def __init__(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.servo_pos = 0
        self.rot = 90.0
        self.size = 34.8 * CM_TO_PX
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)
        self.manual = True
        self.bumper = ""

    def move(self, amt):
        """move forward/backwards"""
        
        x, y = polar_to_cart(self.rot, amt * CM_TO_PX)
        self.x += x
        self.y += y
        self.rect = pygame.Rect(self.x-(self.size/2),self.y-(self.size/2),self.size,self.size)

    def turn(self, angle):
        """turn right/left"""
        self.rot += angle

    def clear(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.rot = 0
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)
        ScanData.clear()

    def scan(self,theta,ir,ping):
        """scan 180 degrees infront"""

        self.servo_pos = theta
        ir = ir_to_cm(ir)
        irx, iry = polar_to_cart(int(self.servo_pos) - 90 + self.rot,ir * CM_TO_PX)
        pgx, pgy = polar_to_cart(int(self.servo_pos) - 90 + self.rot, ping * CM_TO_PX)
        offsetx, offsety = polar_to_cart(int(self.rot), float(34.8 / 2) * CM_TO_PX)
        ScanData.append(Point(self.x+irx+offsetx, self.y+iry+offsety, self.x+pgx+offsetx, self.y+pgy+offsety,theta,ir,ping))  

class Point():
    """class to hold scan data"""

    def __init__(self,irx,iry,pgx,pgy,theta,ir,pg):
        self.ir = [ir,[irx,iry]]
        self.pg = [pg,[pgx,pgy]]
        self.theta = theta

# initalize serial connection
try:
    cybot_uart = uart.UartConnection()
    player = Player()
    stream = threading.Thread(target=cybot_uart.data_stream, args=[player])
    stream.daemon = True
    stream.start()
except serial.serialutil.SerialException:
    print("ERR: Failed to connect to CyBot.")
    sys.exit()

# initalize pygame
pygame.init()
font = pygame.font.SysFont('Segoe UI', 30)
screen = pygame.display.set_mode([SCREEN_WIDTH, SCREEN_HEIGHT])
ScanData = []
RenderedScan = []

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
                cybot_uart.send_data("a")
                cybot_uart.send_data("d15x")
            if event.key == ord('d'): # turn right
                cybot_uart.send_data("d-15x")
            if event.key == ord('w'): # move forward
                cybot_uart.send_data("w20x")
            if event.key == ord('s'): # move backwards
                cybot_uart.send_data("w-20x")
            if event.key == ord('m'): # scan
                cybot_uart.send_data("m")
            if event.key == ord('t'): # toggle autonomous
                cybot_uart.send_data("t")
            if event.key == ord('k'): # clear
                ScanData.clear()

    # fill background
    screen.fill(BLACK)

    # draw cybot
    xe, ye = polar_to_cart(player.rot + player.servo_pos - 90, player.size / 2)
    pygame.draw.circle(screen, WHITE, (player.x, player.y), player.size / 2)
    pygame.draw.arc(screen, GREEN, player.rect, math.radians(player.rot-180), math.radians(player.rot), 10)
    pygame.draw.line(screen, BLUE, (player.x, player.y), (player.x + xe, player.y + ye), 5)
    bump_text = font.render("bumper: " + player.bumper, False, WHITE)
    screen.blit(bump_text,(0,40))
    pos_text = font.render(str(round(player.x - (SCREEN_WIDTH / 2), 2)) + ", " + str(round(player.y - (SCREEN_HEIGHT / 2),2)) + ", " + str(round(player.rot,2)), False, WHITE)
    screen.blit(pos_text,(0,80))

    if player.manual:
        mode_text = font.render("mode: manual", False, WHITE)
    else:
        mode_text = font.render("mode: auto", False, WHITE)
    screen.blit(mode_text,(0,0))

    RenderedScan.clear()
    start = 0
    end = 0
    for x in range(len(ScanData)):
        pygame.draw.circle(screen, RED, ScanData[x].ir[1], 1)
        if ScanData[x].ir[0] < 180 and ScanData[x].pg[0] < 300:
            end = x
        elif (abs(start - end) >= 3):
            RenderedScan.append([ScanData[start].pg[1],ScanData[int((start+end)/2)].pg[1],ScanData[end].pg[1]])
            start = x + 1
            end = x + 1
        else:
            start = x + 1
            end = x + 1
    
    for points in RenderedScan:
        pygame.draw.polygon(screen, WHITE, points, 5)


    pygame.display.flip()