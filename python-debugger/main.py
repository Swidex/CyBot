import matplotlib.pyplot as plt
import numpy as np
import serial

def init_uart():
    """establish UART connection to cyBot and read output"""
    ser = serial.serial_for_url("socket://192.168.1.1:288")
    return ser

def scan(ser):
    """Read UART Stream"""

    #initialize variables for data
    data = ""
    last_byte = ''
    theta_data = []
    ir_data = []
    ping_data = []

    while True:
        #   example output:
        #   theta,ir_val,ping_val
        #   30,3515,10.0
        if data == "Complete\n":
            #break if done scanning
            return theta_data, ir_data, ping_data
        if last_byte == '\n':
            #append data when newline detected
            theta, ir, ping = data.rstrip().split(',')

            #append data to arrays
            theta_data.append(int(theta))
            ping_data.append(float(ping))
            ir_data.append(float(ir))

            #reset data string
            data = ""
        
        #read latest byte, convert to utf-8, and append to data string
        last_byte = ser.read(1).decode("utf-8")
        data += last_byte
        ser.flushOutput()

def plot_distance_field(theta,ir,ping):
    """use matplotlib to plot distance values radially"""
    fig, axs = plt.subplots(2, subplot_kw=dict(projection="polar"))

    # add titles
    axs[0].title.set_text("IR Sensor")
    axs[1].title.set_text("Ping Sensor")

    # create polar scatter plots
    axs[0].scatter(theta,ir, marker='o', c='r', s=30)
    axs[1].scatter(theta,ping, marker='o', c='r', s=30)

    # only display 180 degrees
    axs[0].set_thetamax(180)
    axs[1].set_thetamax(180)

    plt.show()

def plot_calibration(ir,ping):
    """use matplotlib to plot distance values"""
    from scipy.optimize import curve_fit
    x = np.array(ir)
    y = np.array(ping)

    # calculate power fit
    popt, _ = curve_fit(lambda fx,a,b: a*fx**-b,  x,  y)
    power_y = popt[0]*x**-popt[1]

    print("Distance Formula: y =",popt[0]," * x^-",popt[1])

    plt.scatter(x, y, label='Sensor Data')
    plt.plot(x, power_y, label='Power Fit')
    plt.legend()
    plt.title("IR Value vs. Ping Distance")
    plt.ylabel("IR Value")
    plt.xlabel("Ping Distance (cm.)")
    plt.show()

def main():
    # connect to CyBot, exit if failed.
    try:
        ser = init_uart()
    except serial.serialutil.SerialException:
        print("ERR: Failed to connect to CyBot.")
        exit()

    # loop for input
    while True:
        print("Options\nwasd (movement)\nm) Scan\nc) Calibrate")
        user_option = input("Option: ")
        
        if user_option == "w":
            # move forward
            ser.write('w')
        if user_option == "a":
            # turn left
            ser.write('a')
        if user_option == "s":
            # go back
            ser.write('s')
        if user_option == "d":
            # turn right
            ser.write('d')
        if user_option == "m":
            # scan infront of cybot
            ser.write('m')
            theta,ir,ping = scan(ser)
            plot_distance_field(theta,ir,ping)
        if user_option == "c":
            # calibrate ir sensor
            ser.write('c')
            _,ir,ping = scan(ser)
            plot_calibration(ir,ping)
        else:
            print("Invalid input.")

main()
