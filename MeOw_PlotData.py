# MeOws_PlotData

# Script reads MeOw data texfile, converts distances to elevations, and plots results

# Ian R.B. Reeves, modified by K. Anarde for the DUNEX deployment

import pandas as pd
import matplotlib.pyplot as plt

# %%

# Input Data Info
MeOw = 5  # Station number [1,2,3,4,5,6]
SensorElev = 0  # Start elevation of bed sensor
Filter_min = 300  # Minimum allowable sonar distance reading
Filter_max = 5000  # Maximum allowable sonar distance reading

# File Name
# file = 'Data/091321_BucketTest_DATALOG' + str(MeOw) + '.TXT'
# file = 'Data/091921_PoleTest_DATALOG' + str(MeOw) + '.TXT'
# file = 'Data/DATALOG' + str(MeOw) + '_092021_092721' + '.TXT'
# file = 'Data/DATALOG' + str(MeOw) + '_092821_100821' + '.TXT'
file = 'Data/DATALOG' + str(MeOw) + '_100821-102421' + '.TXT'
# file = 'Data/111521_MeOw3_NewBattery.TXT'
# file = 'Data/DATALOG1_111821_112321_REALDUNE.TXT'

# Add Optional Hourly NOAA Tide Data
Tides = False
if Tides:
    tidefile = 'Data/WachapreagueHourly-21Aug-22Nov19.xlsx'
    tidesheet = 0

# Add Optional Offset (set to 0 if none)
Off_bed = 0

# %%

# Load Data
data = pd.read_csv(file, header=3, index_col=0, parse_dates=True)

if Tides:
    tidedata = pd.read_excel(tidefile, sheet_name=tidesheet, parse_dates=[['Date', 'Time (LST/LDT)']])
    tide_dt = tidedata['Date_Time (LST/LDT)']
    tide_waterlevel = tidedata['Verified (m)']

# Filter Data
data_raw = data
data = data[data['SonarRange_mm'] > Filter_min]
data = data[data['SonarRange_mm'] < Filter_max]

# Convert Distance Readings to Elevations
# data['BedElev'] = SensorElev - data.SonarRange_mm / 1000
data['BedElev'] = data.SonarRange_mm / 1000
data_raw['BedElev'] = data_raw.SonarRange_mm / 1000

# # Apply Optional Offsets
data['BedElev'] += Off_bed

# %%

# Plot Only Bed Elevations
plt.figure(figsize=(10, 6))
data['BedElev'].plot(marker='.', alpha=0.5, linestyle='None')
# if Tides: plt.plot(tide_dt, tide_waterlevel, c='gray', ls='-', alpha=0.15)
plt.xlabel('Date-Time')
# plt.ylabel('Elevation (m)')
plt.ylabel('Distance (m)')
# if Tides: plt.legend(['Tide', 'Bed'], markerscale=10, loc='upper right')
plt.legend(['Bed'], markerscale=2, loc='upper right')
plt.show()

# Plot raw data
plt.figure(figsize=(10, 6))
data_raw['BedElev'].plot(marker='.', alpha=0.5, linestyle='None')
plt.xlabel('Date-Time')
# plt.ylabel('Elevation (m)')
plt.ylabel('Distance (m)')
plt.legend(['Raw-bed'], markerscale=2, loc='upper right')
plt.show()

# Plot Battery
plt.figure(figsize=(10, 6))
data_raw['Battery_V'].plot(color='forestgreen')
plt.xlabel('Date-Time')
plt.ylabel('Battery Voltage')
plt.show()
