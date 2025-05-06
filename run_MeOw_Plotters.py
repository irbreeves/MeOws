from MeOw_PlotData import plot_data

meow = 2  # Station number [1,2,3,4,5,6]
sensor_elevation = 5.064  # Start elevation of bed sensor [m NAVD88], DUNEX_100821 = [4.788, 5.064, 5.207, 5.457, 4.175, 4.345]
filter_min = 300  # Minimum allowable sonar distance reading
filter_max = 5000  # Maximum allowable sonar distance reading

# meow_file = 'Data/DATALOG5_22122021.TXT'
meow_file = 'Data/DATALOG' + str(meow) + '_100821-102421' + '.TXT'
# meow_file = 'Data/DATALOG' + str(meow) + '_100821-102421_v2' + '.TXT'  # MeOw2 DUNEX
# file = 'Data/091321_BucketTest_DATALOG' + str(meow) + '.TXT'
# file = 'Data/091921_PoleTest_DATALOG' + str(meow) + '.TXT'
# file = 'Data/DATALOG' + str(meow) + '_092021_092721' + '.TXT'
# file = 'Data/DATALOG' + str(meow) + '_092821_100821' + '.TXT'
# file = 'Data/111521_MeOw3_NewBattery.TXT'
# file = 'Data/DATALOG1_111821_112321_REALDUNE.TXT'

tide_file = 'Data/WachapreagueHourly-21Aug-22Nov19.xlsx'

plot_data(meow_file, sensor_elevation, filter_min, filter_max)