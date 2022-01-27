Semestral Work – MOTOR CONTROL

AUTHOR:
Roc Benaiges Moragrega

ABSTRACT:
The goal of the semestral work is to create a digital motor controller. The program control the position of the motor according to the set-point given by the position of another motor, moved by hand (steer-by-wire). The set-point will be transferred between the two motor controllers using UDP messages. The actual state of the controller and its history will be published as live graphs over the HTTP protocol.

HOW TO RUN THE PROGRAM:
1. Open WindRiver Workbench and connect the boards and motors. To see the board's console, start gtkterm and set it up for serial port ttyUSB0 (1st motor) and port ttyUSB1 (2nd motor) and baudrate 115200.
2. Build the project
3. Run motorClient, without arguments and with the correct IP adress of the motor Client.
4. Run motor, without arguments and with the correct IP adress of the motor Server.
5. Move the first motor to test if the other motor moves and copy the target position.
6. Open the webserver with the correct IP adress in order to see constantly the position and speed of the motor.
