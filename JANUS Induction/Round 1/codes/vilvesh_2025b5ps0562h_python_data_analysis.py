#importing required libraries
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

#loading pressure data CSV file
#pressure_df = pd.read_csv('pressure_data.csv')
pressure_df = pd.read_excel('Raw_Test_Flight_Data_25.xlsx') #directly reading from exel file

#filtering out invalid pressure values
pressure_df = pressure_df[(pressure_df['Pressure'] > 0) & (pressure_df['Pressure'] < 1400)].reset_index(drop=True)

#constants from problem statement
P0 = 1013
T0 = 298
L = 0.0065
R = 8.31432
M = 0.0289644
g = 9.80665

#calculating altitude 'h' using barometric formula
pressure_df['Altitude'] = (T0 / L) * (1 - (pressure_df['Pressure'] / P0) ** ((R * L) / (g * M)))

#normalizin altitude so launch starts at ground level
pressure_df['Altitude'] -= pressure_df['Altitude'].iloc[0]

#calculating velocity as difference in altitude over time (assuming time interval = 1sec)
pressure_df['Velocity'] = pressure_df['Altitude'].diff().fillna(0)

#smooth altitude and velocity
pressure_df['Altitude_Smoothed'] = pressure_df['Altitude'].rolling(window=4, min_periods=1).mean()
pressure_df['Velocity_Smoothed'] = pressure_df['Velocity'].rolling(window=4, min_periods=1).mean()

#ploting static graphs for visual check
plt.figure(figsize=(12, 6))
plt.subplot(2,1,1)
plt.plot(pressure_df['Altitude'], label='Altitude Raw', alpha=0.5)
plt.plot(pressure_df['Altitude_Smoothed'], label='Altitude Smoothed', linewidth=2)
plt.legend()
plt.title('Altitude vs Time')

plt.subplot(2,1,2)
plt.plot(pressure_df['Velocity'], label='Velocity Raw', alpha=0.5)
plt.plot(pressure_df['Velocity_Smoothed'], label='Velocity Smoothed', linewidth=2)
plt.legend()
plt.title('Velocity vs Time')
plt.tight_layout()
plt.show()

#animation function for altitude
fig1, ax1 = plt.subplots()
line1, = ax1.plot([], [], 'b-', linewidth=2)
ax1.set_xlim(0, len(pressure_df))
ax1.set_ylim(pressure_df['Altitude_Smoothed'].min() - 2, pressure_df['Altitude_Smoothed'].max() + 2)
ax1.set_title('Altitude vs Time (Smoothed & Animated)')
ax1.set_xlabel('Time (seconds)')
ax1.set_ylabel('Altitude (m)')
ax1.grid(True)
xdata, ydata = [], []

def update_altitude(frame):
    xdata.append(frame)
    ydata.append(pressure_df['Altitude_Smoothed'].iloc[frame])
    line1.set_data(xdata, ydata)
    return line1,

ani1 = FuncAnimation(fig1, update_altitude, frames=range(len(pressure_df)), interval=1000, blit=True)

#animation function for velocity
fig2, ax2 = plt.subplots()
line2, = ax2.plot([], [], 'g-', linewidth=2)
ax2.set_xlim(0, len(pressure_df))
ax2.set_ylim(pressure_df['Velocity_Smoothed'].min() - 0.5, pressure_df['Velocity_Smoothed'].max() + 0.5)
ax2.set_title('Velocity vs Time (Smoothed & Animated)')
ax2.set_xlabel('Time (seconds)')
ax2.set_ylabel('Velocity (m/s)')
ax2.grid(True)
xdata_v, ydata_v = [], []

def update_velocity(frame):
    xdata_v.append(frame)
    ydata_v.append(pressure_df['Velocity_Smoothed'].iloc[frame])
    line2.set_data(xdata_v, ydata_v)
    return line2,

ani2 = FuncAnimation(fig2, update_velocity, frames=range(len(pressure_df)), interval=1000, blit=True)

plt.show()
