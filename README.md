# TI_Launchpad3220_LED_Morse
<h3>Toggleable messages that play in morse code on the TI-Launchpad 3220</h3>

<h3>
Use the side buttons to change between "SOS" and "OK".
</h3>

This program utilizes the core concepts of State-Machines to control an embedded device's peripherals, written in C.

# Journal Assignment for Week 8 of CS 350 Embedded Systems Design
&emsp; This project utilizes the GPIO buttons of the TI_Launchpad 3220S to Toggle a state-machine which turns the LEDs on and off in order to create a morse-code message. This project revolved around the idea of ticking state-machines so that the next task does not execute until the current one finishes. This project could have been done better; and thinking back now that I've completed the course otherwise, I'd like to have made this more scaleable in terms of messages the device can play. As of now, the messages are hard-coded. I'd like to create an array of letters along with their corresponding Morse-code, and have users type a message into the UART and see their message played on the LEDs.
<br>
&emsp; The basic knowledge of state-machines is now engrained into my very being, haha. I actually love the idea of ticking state-machines on embedded devices, as it's very commonplace and efficient when implemented correctly. I'm taking this knoweldge with me and applying it to my future projects in embedded systems.
<br>
&emsp; This one was organized very well. Each tick function was commented in places where it was useful to be, and the code is very well-structured in terms of variables and functions each having a section to stay.
<br>
&emsp; Thanks to my professor and classmates for being there for each other and help guide one another through issues and other bugs.
