# MeOw_PlotData

# Script reads MeOw data texfile, converts distances to elevations, and plots results

# Ian R.B. Reeves, modified by K. Anarde to make more general

import pandas as pd
import matplotlib.pyplot as plt


def plot_data(
        meow_file,
        sensor_elevation,
        filter_min,
        filter_max,
        tide_file=None
):

    # import tide data
    if tide_file is not None:

        tidesheet = 0
        tidedata = pd.read_excel(tide_file, sheet_name=tidesheet, parse_dates=[['Date', 'Time (LST/LDT)']])
        tide_dt = tidedata['Date_Time (LST/LDT)']
        tide_waterlevel = tidedata['Verified (m)']

    # load and filter data
    data = pd.read_csv(meow_file, header=3, index_col=0, parse_dates=True)
    data_raw = data
    data = data[data['SonarRange_mm'] > filter_min]
    data = data[data['SonarRange_mm'] < filter_max]

    # convert distance readings to elevations
    if sensor_elevation == 0:
        data['BedElev'] = data.SonarRange_mm / 1000
        data_raw['BedElev'] = data_raw.SonarRange_mm / 1000
    else:
        data['BedElev'] = sensor_elevation - data.SonarRange_mm / 1000
        data_raw['BedElev'] = sensor_elevation - data_raw.SonarRange_mm / 1000

    # plot bed elevations
    plt.figure(figsize=(10, 6))
    data['BedElev'].plot(marker='.', alpha=0.5, linestyle='None')
    if tide_file is not None:
        plt.plot(tide_dt, tide_waterlevel, c='gray', ls='-', alpha=0.15)
    plt.xlabel('Date-Time')
    if sensor_elevation == 0:
        plt.ylabel('Distance (m)')
    else:
        plt.ylabel('Elevation (m NAVD88)')
    if tide_file is not None:
        plt.legend(['Tide', 'Bed'], markerscale=10, loc='upper right')
    else:
        plt.legend(['Bed'], markerscale=2, loc='upper right')
    plt.show()

    # Plot raw data
    plt.figure(figsize=(10, 6))
    data_raw['BedElev'].plot(marker='.', alpha=0.5, linestyle='None')
    plt.xlabel('Date-Time')
    if sensor_elevation == 0:
        plt.ylabel('Distance (m)')
    else:
        plt.ylabel('Elevation (m NAVD88)')
    plt.legend(['Raw-bed'], markerscale=2, loc='upper right')
    plt.show()

    # Plot Battery
    plt.figure(figsize=(10, 6))
    data_raw['Battery_V'].plot(color='forestgreen')
    plt.xlabel('Date-Time')
    plt.ylabel('Battery Voltage')
    plt.show()
