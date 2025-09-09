FILE STRUCTURE:
📂 JANUS_Induction
      └── 📂 Round 3
           ├── 📂 Problem 1
           │    ├── arduino_code.ino
           ├── 📂 Problem 2
                ├── index.html
                ├── index_with3d.html
           ├── 📂 Problem 3
                ├── kicad.lck

# README – My Journey Through Problems 1, 2, and 3

This file is more of a personal log than a strict technical manual. It’s about how I approached the three avionics-related problems, what I did, what worked, what didn’t, and what I’d probably improve if I had more time.
So please dont expect any perfect explanations :)

## Problem 1 – Basic Arduino Serial Plotting
At first, the goal was simple: just get some data from the Arduino and show it on a graph in the browser. I had two sensor values (you can think of them as temperature and pressure). I wrote a small HTML+JS page that used the Web Serial API to talk to the Arduino. Plotly.js was my choice for plotting because it’s easy to get something on the screen fast.

**What worked:**
* Opening the serial port and reading values line by line.
* Splitting the incoming string by commas and using the first two numbers.
* Plotly updating in real time with shifting arrays.

**What I realized later:**
* It only works if the Arduino prints exactly `value1,value2`. If the format changes, it breaks.
* If the data is noisy or incomplete, the code throws errors.

**Possible improvements:**
* Make the parser more robust (similar to what I did in Problem 2).
* Add labels to the graph depending on what the sensors actually are.
* Maybe save the incoming data to a file.

**Improvements I did**
* Change the existing code to plot on a 3d scale with the dimensions: altitude, latitude and longitude
---

## Problem 2 – GPS Data with NMEA Sentences
This one was trickier. The Arduino was sending full NMEA `$GNGGA` sentences like:

```
$GNGGA,000115.00,2836.8093,N,07712.5418,E,1,09,1.3,1030.0,M,0.0,M,,*78
```

My original plotting code from Problem 1 didn’t make sense here because `parts[0]` was `$GNGGA`, not a number. So I had to change my approach.

**Steps I took:**
* Extracted the right fields (latitude, longitude, altitude).
* Wrote a conversion function to turn NMEA coordinates (ddmm.mmmm) into decimal degrees.
* Updated the graph to show three traces: lat, lon, alt vs time.

**Challenges:**
* The serial data often arrives in chunks, so sometimes you get half a line. That used to crash the parser.
* I added a buffer and only processed complete lines. This fixed the random errors.
* Added a test mode with some sample sentences (valid, invalid, incomplete) to check if the parser can handle them.

**Ideas for later:**
* Instead of plotting lat/lon vs time, it’d be cooler to plot the actual path on a map or at least a lat vs lon scatter.
* Maybe add a 3D scatter (lat, lon, alt) for a more visual representation.
* Add a little status display to show the last sentence and whether it was parsed or ignored.

## Problem 3 – PCB Design with KiCad
This part shifted away from coding and into hardware design. I had to take a schematic (already drawn in KiCad) and turn it into a routed PCB with an STM32 MCU, a BMP290 sensor, and a Micro USB-B connector.

**How I approached it:**

* Opened the schematic in KiCad, annotated the components, and fixed ERC errors.
* Assigned footprints for the MCU, the sensor, and the USB connector (this part took time because the footprints must match datasheets).

**What I found difficult:**
Since i dont have much experience working with kicad, i could not complete the answer to this problem in time

**If I had more time:**
I'd learn kicad from scratch and try to give a suitable answer to the problem given

## Final Thoughts
Each problem built on the last in a way:

* Problem 1 was about basic serial → graph.
* Problem 2 forced me to parse real-world messy data and think about robustness.
* Problem 3 made me shift into actual hardware, where placement and routing matter just as much as code.

I actually regret not being able to take up problem 3 and complete it fully since it seemed as a more interesting (and challenging) task than the previous 2.
Although I was able to get to a certain point, I was finding myself in a very confused state as I didnt know how to proceed.
If i could do it all over again, i would definitely learn the required stuff (fully) for problem 3 and attempt to give a complete answer.

I’ve left some “future improvements” in each section, both to remind myself what I’d like to learn next and to show that this work is more of a journey than a finished product. Hopefully it makes sense and shows my thought process rather than just a polished end result.
