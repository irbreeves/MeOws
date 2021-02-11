# MeOws_PlotData

# Script reads MeOw data texfile, converts distances to elevations, and plots results

# Ian R.B. Reeves
# Updated: 11 Feb 21

import pandas as pd
import matplotlib.pyplot as plt

#%%

# Input Data Info
MeOw = 6                    # Station number
SensorElev_Bed = 3.339      # Start elevation of bed sensor
SensorElev_Water = 3.490    # Start elevation of water sensor
Filter_min = 500            # Minimum allowable sonar distance reading
Filter_max = 3000           # Maximum allowable sonar distance reading

# File Name
file = 'Data/DATALOG' + str(MeOw) + '.TXT'

# Add Optional Hourly NOAA Tide Data
Tides = True
if Tides:
    tidefile = 'Data/WachapreagueHourly-21Aug-22Nov19.xlsx'
    tidesheet = 0

# Add Optional Offset (set to 0 if none)
Off_bed = 0
Off_water = 0

#%%

# Load Data
data = pd.read_csv(file,header=3,infer_datetime_format=True)
dt = data['DateTime']
Temp = data['BoardTemp_C']
Batt = data['Battery_V']
Range_bed = data['SonarRange_Bed_mm']
Range_water = data['SonarRange_Water_mm']

if Tides:
    tidedata = pd.read_excel(tidefile, sheet_name=tidesheet, parse_dates=[['Date','Time (LST/LDT)']])
    tide_dt = tidedata['Date_Time (LST/LDT)']
    tide_waterlevel = tidedata['Verified (m)']

#%%

# Filter Data
data_raw = data
data = data[data['SonarRange_Bed_mm'] > Filter_min] 
data = data[data['SonarRange_Bed_mm'] < Filter_max]                 
data = data[data['SonarRange_Water_mm'] > Filter_min]
data = data[data['SonarRange_Water_mm'] < Filter_max]

# Convert Distance Readings to Elevations
data['BedElev'] = SensorElev_Bed - data.SonarRange_Bed_mm / 1000
data['WaterElev'] = SensorElev_Water - data.SonarRange_Water_mm / 1000

# Apply Optional Offsets
data['BedElev'] += Off_bed
data['WaterElev'] += Off_water

#%%
 
# Plot Bed and Water Elevations
plt.figure(figsize=(20, 12))
plt.rcParams.update({'font.size':12})
if Tides: plt.plot(tide_dt, tide_waterlevel, c='gray', ls='-', alpha=0.15)
data.plot.scatter('DateTime', 'WaterElev', c='blue', s=1, ax=plt.gca())
data.plot.scatter('DateTime', 'BedElev', c='peru', s=1, ax=plt.gca())
plt.xlabel('Date-Time')
plt.ylabel('Elevation (m NAVD88)')
if Tides: plt.legend(['Tide Gauge - Wachapreague', 'Bed', 'Water'], markerscale=10, loc='upper right')
else: plt.legend(['Bed', 'Water'], markerscale=10, loc='upper right')
plt.show()

# Plot Only Bed Elevations
plt.figure(figsize=(20, 12))
plt.rcParams.update({'font.size':12})
if Tides: plt.plot(tide_dt, tide_waterlevel, c='gray', ls='-', alpha=0.15)
data.plot.scatter('DateTime', 'BedElev', c='peru', s=1, ax=plt.gca())
plt.xlabel('Date-Time')
plt.ylabel('Elevation (m NAVD88)')
if Tides: plt.legend(['Tide Gauge - Wachapreague', 'Bed', 'Water'], markerscale=10, loc='upper right')
else: plt.legend(['Bed', 'Water'], markerscale=10, loc='upper right')
plt.show()

# Plot Temperature and Battery
plt.figure(figsize=(20, 12))
plt.subplot(2,1,1)
data_raw.plot('DateTime', 'BoardTemp_C', c='red', ax=plt.gca())
plt.xlabel('Date-Time')
plt.ylabel('Board Temperature (C)')
plt.subplot(2,1,2)
data_raw.plot('DateTime', 'Battery_V', c='forestgreen', ax=plt.gca())
plt.xlabel('Date-Time')
plt.ylabel('Battery Voltage')
plt.show()