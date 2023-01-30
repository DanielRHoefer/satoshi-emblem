#buck converter equations

#USER VARIABLES 
from math import *


v_in = 14 #volts
v_out = 7 #volts 
output_current = 15 #amp
freq = 1000 #KHz d
efficiency = 0.9
input_ptp = 100 #mV
series_inductance = 560 #uH 
output_transient_current = 5 #A

v_rip = 8 # mV
output_esr = 2 #mOhms

v_sw = 0.1 #V 
v_catch = 0.1 #V

#END USER VARIABLES 



delta_l = v_rip / output_esr
r = delta_l / output_current

inductance = ((v_in - v_sw - v_out) * (v_out + v_catch)) / \
             ((v_in - v_sw + v_catch) * r * freq * 1000 * output_current) * 1000000

t_on = ((v_out + v_catch) * 1000000) / ((v_in - v_sw + v_catch) * freq * 1000)
et = (v_in - v_sw - v_out) * t_on
i_peak = output_current + (delta_l / 2)
i_rms = sqrt(pow(output_current, 2) + (pow((et / inductance), 2) / 12))
e = 0.5 * inductance * pow(i_peak, 2)
i_input = (v_out / (v_in * efficiency) * output_current)

print("Inductance: %5.2fuH" % inductance)       #uH
print("Peak inductor current: %5.2fA" % i_peak) #A
print("Peak input current: %5.2fA" % i_input)   #A
print("RMS inductor current: %5.2fA" % i_rms)   #A
print("Total inductor energy: %5.2fuJ" % e)      #uJ

dc = v_out / (v_in * efficiency)
c_min = (output_current * dc * (1-dc) * 1000000) / (freq * input_ptp)
print("Minimum ceramic input capcitance: %5.2fuF" % c_min)  #uF

delta_input = (v_out / (v_in * efficiency) * output_transient_current)

input_bulk_cap = (1.21 * pow(delta_input, 2) * series_inductance) / input_ptp
print("Minimum bulk input capcitance: %5.2fuF" % input_bulk_cap)  #uF