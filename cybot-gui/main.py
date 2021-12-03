import serial.serialutil
import pygame, math, sys, uart, serial, threading, time, numpy
from scipy.optimize import curve_fit

# Constants
BLUE = (25, 25, 200)
PURPLE = (150, 25, 200)
BLACK = (23, 23, 23)
GREY = (56, 56, 56)
WHITE = (254, 254, 254)
RED = (200, 25, 25)
GREEN = (25, 200, 25)

CM_TO_PX = 1.724
SCREEN_WIDTH = 1280
SCREEN_HEIGHT = 720

move_avg = [14]
turn_avg = [70.12]
ir_cal = [1043, 745, 610, 487, 2191, 1530, 1169, 945, 778, 672, 583, 528, 466, 406, 381, 330, 298, 268, 248, 225, 236, 235, 196, 235, 196, 168, 144, 166, 120]
pg_cal = [16.12, 22.64, 27.04, 31.84, 7.79, 11.29, 14.86, 19.04, 23.14, 26.72, 28.85, 33.23, 36.98, 40.65, 44.12, 48.08, 51.49, 26.79, 33.26, 39.66, 66.9, 1000.0, 1000.0, 1000.0, 1000.0, 64.08, 8.82, 1000.0, 69.69]
COEFF = 3082
PWR = -0.748
IR_RAW = 0
CliffData = []

def polar_to_cart(deg, amt):
    """convert polar coordinates to cartesian"""
    x = float(amt) * math.sin(math.radians(int(deg)))
    y = float(amt) * math.cos(math.radians(int(deg)))
    return x,y

def line_of_best_fit():
    """set formula for calculating ir values"""
    def objective(x, a, b):
	    return a * pow(x, b)
    global COEFF, PWR
    popt, _ = curve_fit(objective, ir_cal, pg_cal)
    COEFF, PWR = popt

def ir_to_cm(val):
    """convert ir values into centimeters"""
    return COEFF * pow(val,PWR)

def get_dist(x1,x2,y1,y2):
    return abs(math.sqrt(pow(x1-x2,2) + pow(y1-y2,2)))

def avg(list):
    return sum(list) / len(list)

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
        self.lBump = ""
        self.rBump = ""
        self.estimating = False

    def update(self, angle, dist, lBump, rBump):
        """update position info"""
        self.rot += angle

        # movement handling
        x, y = polar_to_cart(self.rot, dist * CM_TO_PX)
        self.x += x
        self.y += y
        self.rect = pygame.Rect(self.x-(self.size/2),self.y-(self.size/2),self.size,self.size)

        # bumper handling
        self.lBump = "l" if lBump == 1 else ""
        self.rBump = "r" if rBump == 1 else ""

    def cliff(self, cliffVal) :
        # cliff handling
        lx, ly = polar_to_cart(self.rot - 45, self.size / 1.5)
        flx, fly = polar_to_cart(self.rot - 15, self.size / 1.5)
        rx, ry = polar_to_cart(self.rot + 45, self.size / 1.5)
        frx, fry = polar_to_cart(self.rot + 15, self.size / 1.5)

        if cliffVal & 0b1:
            # l high
            CliffData.append(Cliff(True,self.x + lx,self.y + ly,self.x,self.y))
        if cliffVal & 0b10:
            # l low
            CliffData.append(Cliff(False,self.x + lx,self.y + ly,self.x,self.y))
        if cliffVal >> 2 & 0b1:
            # lf high
            CliffData.append(Cliff(True,self.x + flx,self.y + fly,self.x,self.y))
        if cliffVal >> 2 & 0b10:
            # lf low
            CliffData.append(Cliff(False,self.x + flx,self.y + fly,self.x,self.y))
        if cliffVal >> 4 & 0b1:
            # r high
            CliffData.append(Cliff(True,self.x + rx,self.y + ry,self.x,self.y))
        if cliffVal >> 4 & 0b10:
            # r low
            CliffData.append(Cliff(False,self.x + rx,self.y + ry,self.x,self.y))
        if cliffVal >> 8 & 0b1:
            # rf high
            CliffData.append(Cliff(True,self.x + frx,self.y + fry,self.x,self.y))
        if cliffVal >> 8 & 0b10:
            # rf low
            CliffData.append(Cliff(False,self.x + frx,self.y + fry,self.x,self.y))

    def calibrate_ir(self):
        """calibrate ir sensor w/ ping sensor"""
        print("Calibrating IR sensors...")
        while self.bumper == "":
            cybot_uart.send_data('w')
            time.sleep(0.25)
            cybot_uart.send_data('w') # stop moving

        time.sleep(0.1)
        
        for _ in range(20):
            cybot_uart.send_data('s')
            time.sleep(0.25)
            cybot_uart.send_data('s')
            cybot_uart.send_data('m')
            time.sleep(0.1)
            if ScanData[len(ScanData) - 1].pg[0] >= 500: continue
            ir_cal.append(IR_RAW)
            pg_cal.append(ScanData[len(ScanData) - 1].pg[0])
        print("Complete!")
        print("Recommend: Restart client or clear scan data (k).")
        self.calibrate_move_estimation()

    def forward(self):
        """move forward until not estimating"""
        self.estimating = True
        cybot_uart.send_data('w')
        while self.estimating: continue
        cybot_uart.send_data('w')

    def back(self):
        """move backward until not estimating"""
        self.estimating = True
        cybot_uart.send_data('s')
        while self.estimating: continue
        cybot_uart.send_data('s')

    def left(self):
        """turn left until not estimating"""
        self.estimating = True
        cybot_uart.send_data('a')
        while self.estimating: continue
        cybot_uart.send_data('a')

    def right(self):
        """turn right until not estimating"""
        self.estimating = True
        cybot_uart.send_data('d')
        while self.estimating: continue
        cybot_uart.send_data('d')

    def clear(self):
        self.x = SCREEN_WIDTH / 2
        self.y = SCREEN_HEIGHT / 2
        self.rot = 0
        self.rect = pygame.Rect(self.x-30,self.y-30,60,60)
        ScanData.clear()


    def scan(self,theta,ir,pg):
        """scan 180 degrees infront"""
        global IR_RAW

        self.servo_pos = theta
        IR_RAW = int(ir)  # for calibration
        ir = ir_to_cm(ir)
        irx, iry = polar_to_cart(int(self.servo_pos) - 90 + self.rot, ir * CM_TO_PX)
        pgx, pgy = polar_to_cart(int(self.servo_pos) - 90 + self.rot, pg * CM_TO_PX)
        offsetx, offsety = polar_to_cart(self.rot, float(34.8 / 2) * CM_TO_PX)
        
        if IR_RAW > 500:

            #Add a new obstacle in the grid at calculated coordinates
            obstacle_grid[self.x+pgx+offsetx][self.y+pgy+offsety] = Obstacle(self.x+irx+offsetx, self.y+iry+offsety, self.x+pgx+offsetx, self.y+pgy+offsety)

class Obstacle():
    '''
    This represents an obstacle in the field.

    TODO: Add width of object, whether large or small
    '''
    def __init__(self, ix, iy, px, py):
        self.ix = ix
        self.iy = iy
        self.px = px
        self.py = py
        self.points = []
    def __str__(self):
        return "Obstacle(" + str(self.px) + ", " + str(self.py) + ", irx: " + str(self.ix) + ", iry: " + str(self.iy) + ")"

    def __repr__(self):
        return self.__str__()


class Grid():
    '''
    Class for transparently working on the grid.

    Acts similar to 2d list. If grid[a] does not exist, the inner list is automatically created
    Index into the grid like so: grid[x][y].
    If no obstacle exists at (x,y), returns None.
    '''

    def __init__(self, near_threshold=5, outer=True, container=None):
        #super().__init__(self)
        self.grid_dict = {}
        self.near_threshold = near_threshold
        self.outer = outer
        self.container = container
        self.points = []

    def get_obstacles(self):
        obstacles = []
        for row in self.grid_dict.values():
            for obst in row.grid_dict.values():
                obstacles.append(obst)
        return obstacles

    def clear(self):
        del self.grid_dict
        self.grid_dict = {}

    def __getitem__(self, key):
        key = int(key)
        try:
            return self.grid_dict[key]
        except KeyError:
            if(self.outer):
                self.grid_dict[key] = Grid(near_threshold=self.near_threshold,outer=False, container = self)
                return self.grid_dict[key]
            return None

    def __setitem__(self, key, newval):
        #print("Setting item, self.outer=" + str(self.outer))
        if(not self.outer):
            x = int(newval.px)
            y = int(newval.py)
            for i in range(int(x - self.near_threshold), int(x + self.near_threshold)):
                for j in range(int(y - self.near_threshold), int(y + self.near_threshold)):
                    if self.container[i][j] != None:
                        print("Found near obstacle")
                        self.container[i][j].points.append((newval.px, newval.py))
                        return

            #If no near obstacle found, add to grid
            self.grid_dict[int(key)] = newval
    
    def __str__(self):
        return str(self.grid_dict)

    def __repr__(self):
        return self.__str__()

class Cliff():
    """class to hold cliff data"""

    def __init__(self,cliff,x,y,dX,dY):
        #TODO: add pit visualization to gui
        if cliff:
            self.color = BLACK
        else:
            self.color = WHITE

        self.x = x
        self.y = y
        self.dx = dX
        self.dy = dY

def main():
    '''
    Main function, will be called if __name__ == "__main__"
    '''

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
                if event.key == ord('a'): # turn left
                    threading.Thread(target=player.left).start()
                if event.key == ord('d'): # turn right
                    threading.Thread(target=player.right).start()
                if event.key == ord('w'): # move forward
                    threading.Thread(target=player.forward).start()
                if event.key == ord('s'): # move backwards
                    threading.Thread(target=player.back).start()
                if event.key == ord('m'): # scan once
                    cybot_uart.send_data("m")
                if event.key == ord('n'): # radial scan
                    cybot_uart.send_data("n")
                if event.key == ord('c'): # calibrate
                    threading.Thread(target=player.calibrate_ir, daemon=True).start()
                if event.key == ord('f'): # apply calibration settings
                    line_of_best_fit()
                if event.key == ord('k'): # clear scan data
                    ScanData.clear()
                if event.key == ord('r'): # reset cybot location
                    pass
            elif event.type == pygame.KEYUP:
                player.estimating = False

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
        for obstacle in obstacle_grid.get_obstacles():
            pygame.draw.circle(screen, PURPLE, [obstacle.x, obstacle.y], 1)

        pygame.display.flip()

#main()

# initalize pygame
pygame.init()
font = pygame.font.SysFont('Segoe UI', 30)
screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
ScanData = []
obstacle_grid = Grid(near_threshold=5)

try:
    cybot_uart = uart.UartConnection()
    player = Player()
    stream = threading.Thread(target=cybot_uart.data_stream, args=[player])
    stream.daemon = True
    stream.start()
except serial.serialutil.SerialException:
    print("No serial connection found")
    sys.exit()

#PLEASE PUT IN A MAIN FUNCTION LIKE ABOVE
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
            if event.key == ord('a'): # turn left
                threading.Thread(target=player.left).start()
            if event.key == ord('d'): # turn right
                threading.Thread(target=player.right).start()
            if event.key == ord('w'): # move forward
                threading.Thread(target=player.forward).start()
            if event.key == ord('s'): # move backwards
                threading.Thread(target=player.back).start()
            if event.key == ord('m'): # scan once
                cybot_uart.send_data("m")
            if event.key == ord('n'): # radial scan
                cybot_uart.send_data("n")
            if event.key == ord('c'): # calibrate
                threading.Thread(target=player.calibrate_ir).start()
            if event.key == ord('f'): # apply calibration settings
                line_of_best_fit()
            if event.key == ord('k'): # clear scan data
                ScanData.clear()
            if event.key == ord('r'): # reset cybot location
                pass
        elif event.type == pygame.KEYUP:
            player.estimating = False

    # fill background
    screen.fill(GREY)

    for cliff in CliffData:
        pygame.draw.circle(screen, cliff.color, (cliff.x, cliff.y), 10)

    # draw cybot
    xe, ye = polar_to_cart(player.rot + player.servo_pos - 90, player.size / 2)
    pygame.draw.circle(screen, WHITE, (player.x, player.y), player.size / 2)
    pygame.draw.arc(screen, GREEN, player.rect, math.radians(player.rot-180), math.radians(player.rot), 10)
    pygame.draw.line(screen, BLUE, (player.x, player.y), (player.x + xe, player.y + ye), 5)
    bump_text = font.render("bumper: " + player.lBump + player.rBump, False, WHITE)
    screen.blit(bump_text,(0,40))
    pos_text = font.render(str(round(player.x - (SCREEN_WIDTH / 2), 2)) + ", " + str(round(player.y - (SCREEN_HEIGHT / 2),2)) + ", " + str(round(player.rot,2)), False, WHITE)
    screen.blit(pos_text,(0,80))

    if player.manual:
        mode_text = font.render("mode: manual", False, WHITE)
    else:
        mode_text = font.render("mode: auto", False, WHITE)
    screen.blit(mode_text,(0,0))

    for obstacle in obstacle_grid.get_obstacles():
            pygame.draw.circle(screen, RED, [obstacle.px, obstacle.py], 1)
    
   # print(str(obstacle_grid) + "\n")

    pygame.display.flip()
