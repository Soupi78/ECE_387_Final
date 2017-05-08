# ECE_387_Final
This will serve as the wiki for my ECE 387 final project.
To start off, here is a video quickly demoing the functionality:
https://www.youtube.com/watch?v=xCgmdyfzFtA

To provide some context, this final is a continuation of my ECE 387 midterm project, an SD logging GPS system to mounted on a drone:

https://github.com/Soupi78/GPS_Logging_For_Drone-Automated-Google-Maps-Verification

To explain what this system does in brief, it sends GPS data to a Bluetooth connected and Uart enabled device. 
My Midtern project allowed me to store GPS data to an SD card and my final can do the same, however due to the size of the Bluetooth, SD and GPS libraries, the arduino uno only has the capability to import 2 of the 3 libraries at a time. For this reason I have left my code in it's bluetooth state to highlight the diffrence. In my code iteslf I've commented specefically in the sections I've added to my midterm as well as what each section of my new code does. Basically what I've done is used the same interupt I used to log GPS data to an SD, and after writting to the SD I used the same data and sent it over bluetooth. Because of the limitations of bluetooth, this data had to broken down into 20 bytes and sent as a packet. What it does it it takes the NMEA GPS data and breaks it down and transmits it to the uART of whatever Bluetooth device you've connected; if no GPS data is available, it simply transimits "No Fix". Below is a pictre of the Data after being transmitted:
http://imgur.com/a/4bzTD

The first step in setting up this project is wiring it together. Fortunately for me, the ADAFruit Ultimate GPS Logger Pro stacks perfcetly into the UNO and also leaves an extra port on each pin. This means all I had to do was connect the ADAFruit Bluetooth chip directly to my GPS shield. Below is the pin layout used:
http://imgur.com/a/8txw3

Because the GPS Shield's pins didn't prove to be reliable, I had to solder female connector pins onto my shield to connect the Bluetooth so be warned if you can't find your BLE device while testing. Unfortunately the NRF8001 doesn't have an LED to indicate power like the GPS chip does, so for degugging using an LED to verify proved helpful because a lot of the problems I ran into were hardware issues. Below is my final setup used:
http://imgur.com/a/OcHAM
