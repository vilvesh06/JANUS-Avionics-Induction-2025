# JANUS-Avionics-Induction-2025
Repo containing all the necessary files to evaluate my answers and solutions to the given problems as part of the induction process

Hi this is my first time writing a README file so i have tried to include as much info as i can without crossing the given regulations so here you go:

(note that the file hierarchy is only properly visible when you view this readme file as code)
File Structure:
📂 JANUS_Induction
      └── 📂 Round 1
           ├── 📂 codes
           │    ├── vilvesh_2025b5ps0562h_arduino_code.ino
           │    └── vilvesh_2025b5ps0562h_python_data_analysis.py
           │
           ├── 📂 screenshots
           │    ├── LED_apogee.png
           │    ├── LED_ascending.png
           │    ├── LED_descending.png
           │    └── wiring_schematic.png

Problem 1 – Data Analysis (Python)
1. For this part, I had to take raw pressure data and turn it into something more useful like altitude and velocity.
2. First, I read the pressure values from the given Excel sheet.
3. I used the barometric formula to convert pressure into altitude. The starting altitude was normalized to zero since the launch begins from ground level.
4. I then calculated velocity as the change in altitude per second.
5. Because the data was a bit noisy, I applied a moving average filter to smooth out the values.
6. Finally, I plotted both static and animated graphs for altitude vs time and velocity vs time using matplotlib. The animation updates point by point every second, which made the graph look more realistic and less cluttered.
7. This helped me get a clear idea of the prototype’s flight profile while still keeping the graphs readable.

Problem 2 – State Detection (Arduino in Tinkercad)
1. For the second part, the idea was to simulate how the system would know whether the device is going up, at the peak, or coming down.
2. I used a force sensor as input, and then converted that force into pressure (by dividing by the surface area).
3. Just like with the Python part, I added a moving average filter here as well to reduce random fluctuations.
4. From the pressure, I estimated the altitude and then calculated velocity.
5. Based on the velocity value, I set conditions:
If velocity is positive → the system is ascending (green LED ON).
If velocity is around zero → the system is at apogee (blue LED ON + buzzer ON).
If velocity is negative → the system is descending (red LED ON).
6. I also printed out values to the Serial monitor so I could check what was happening during the simulation.
7. This way, the state of the system is always indicated clearly through LEDs and the buzzer at the peak.
8. I aslo felt like deviating from the main point a little bit but didnt do so and i will list the changes i wanted to make below:
With small noise, velocity might hover around 0, causing buzzer to ring repeatedly, so i wanted to make the change that it should require velocity to stay near zero for a few cycles before declaring apogee.
I also wanted to add clearer axis labels and maybe polish the visuals  to make it more 'fantastic' but i didnt want to go too deep into that.

Anyways, that was a *short* summary of my journey
