import matplotlib.pyplot as plt
import numpy as np
import serial

DEBUG = True

theta_data = []
ir_data = []
ping_data = []

def uart_stream():
    """establish UART connection to cyBot and read output"""

    #initialize strings for data
    data = ""
    last_byte = ''

    #open serial socket for UART
    ser = serial.serial_for_url("socket://192.168.1.1:288")

    while True:
        #   example output:
        #   theta,ir_val,ping_val
        #   30,3515,10.0
        if data == "Complete\n":
            #break if done scanning
            break
        if last_byte == '\n':
            #append data when newline detected
            theta, ir, ping = data.rstrip().split(',')
            print(theta, ir, ping)

            #append data to arrays
            theta_data.append(int(theta))
            ping_data.append(float(ping))
            #convert raw IR value to distance in centimeters
            ir_data.append(float(6410000 * pow(ir, -1.69)))

            #reset data string
            data = ""
        
        #read latest byte, convert to utf-8, and append to data string
        last_byte = ser.read(1).decode("utf-8")
        data += last_byte
        ser.flushOutput()

def plot_distance_field():
    """use matplotlib to plot distance values radially"""
    if DEBUG:
        # read values from file if debugging
        f = open("example_field.txt", "r")
        for line in f.readlines():
            theta,ir,ping = line.split(',')
            theta_data.append((float(theta) / 180) * np.pi)
            ping_data.append(float(ping))
            ir_data.append(float(6410000 * pow(int(ir), -1.69)))
    else:
        # attempt to connect to UART
        try:
            uart_stream()
        except serial.serialutil.SerialException:
            print("ERR: Failed to connect to CyBot.")
            return

    # graph the sensor data
    fig, axs = plt.subplots(2, subplot_kw=dict(projection="polar"))
    axs[0].title.set_text("IR Sensor")
    axs[1].title.set_text("Ping Sensor")
    axs[0].scatter(theta_data,ir_data, marker='o', c='r', s=30)
    axs[1].scatter(theta_data,ping_data, marker='o', c='r', s=30)
    axs[0].set_thetamax(180)
    axs[1].set_thetamax(180)
    plt.show()
    return

def main():
    while True:
        print("Options:\n1) Calibrate Sensors\n2) Scan Infront\n3) Exit")
        user_input = input("Enter your option: ")
        if user_input == "1":
            print("")
        if user_input == "2":
            plot_distance_field()
        if user_input == "3":
            break
        else:
            print("Invalid input.")

main()