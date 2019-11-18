import os, threading, time, subprocess, glob, math
import re
from netlistman import *

####input parameters
driven_wire_file = './output_dir/registerWires_netlist.txt'
driving_ports_file = './output_dir/registerPorts_netlist.txt'
library = 'gscl45nm'
input_verilog_file = './input_dir/netlist.v'
std_cell_technology = './input_dir/gscl45nm.db'
pipeline_insertion = True
no_of_pipeline = 1
n_slowing = False
n = 2
comb_logic = False

driving_port_connection = ['DFFPOSX1','CLK','CLOCK']
force = False
####output parameters
dc_script = './dc/dc_script.tcl'
dc_work_dir = './dc/'
output_verilog_file = './output_dir/netlist_repipelined.v'

if not os.path.isdir(dc_work_dir):
  os.mkdir(dc_work_dir)

if pipeline_insertion and n_slowing:
  print 'FATAL:: either do nslowing or repipelining....'
  exit()

cascade_number = 0
if pipeline_insertion:
  cascade_number = no_of_pipeline
else:
  cascade_number = n-1

fp = open(driven_wire_file)
wires = [line.replace('\n','') for line in fp.readlines()]
fp.close()
fp = open(driving_ports_file)
ports = [line.replace('\n','') for line in fp.readlines()]
fp.close()

driving_cell_info = []
if ports[0].find('/Q')>=0:
  if comb_logic:
    print 'FATAL:: combinational logic having flops...'
    exit()
  else:
    verilog_of_mod = open(input_verilog_file).readlines()
    verilog_of_mod = ''.join(verilog_of_mod)
    verilog_of_mod = verilog_2_linewise_verilog(verilog_of_mod)
    for port in ports:
      temp_arr = []
      curr_gate = port.replace('/Q','')
      instantiation = get_instance_line(verilog_of_mod,curr_gate,'')
      [module_name,curr_gate] = line_2_module_prop(instantiation)
      instantiation = instantiation.replace(' '+curr_gate+' ','').replace(' ','')
      instantiation = instantiation.replace(module_name+'(','')
      instantiation = instantiation.replace(';','')
      instantiation = instantiation[:-1]
      [ports_arr,wires_arr]=connections2portsNwires(instantiation)
      if not force:
        temp_arr.append(module_name)
        for idx,port_val in enumerate(ports_arr):
          if port_val != 'Q' and port_val != 'D':
            temp_arr.append(port_val)
            temp_arr.append(wires_arr[idx])
      else:
        temp_arr = driving_port_connection
      driving_cell_info.append(temp_arr)
#print driving_cell_info

if len(driving_cell_info) == 0 and n_slowing:
  print 'FATAL:: nslowing without info about existing flops'
  exit()


if len(driving_cell_info) == 0:
  temp_arr = driving_port_connection 
  for idx0 in range(0,len(wires)):
    driving_cell_info.append(temp_arr)
    



fp = open(dc_script,'w')
fp.write('set target_library '+ '"'+std_cell_technology+'"\n')
fp.write('set link_library '+ '"'+std_cell_technology+'"\n')
fp.write('remove_design -all\n')
fp.write('read_file -format verilog '+input_verilog_file+'\n')
fp.write('link\n')
if comb_logic:
  current_driving_cell_info = driving_cell_info[0]
  current_driving_cell_info = current_driving_cell_info[1:len(current_driving_cell_info)]
  async_ports_wires = current_driving_cell_info[1:][::2]
  for wire in async_ports_wires:
    fp.write('create_net '+wire+'\n')
    fp.write('create_port '+wire+'\n')
    fp.write('connect_net '+wire+' '+wire+'\n')
count = 0
for idx0,wire in enumerate(wires):
  fp.write('######disconnecting wire: '+wire+' from port: '+ports[idx0]+' \n')
  fp.write('disconnect_net '+wire+' '+ports[idx0]+'\n')
  current_driving_cell_info = driving_cell_info[idx0]
  cell_name = current_driving_cell_info[0]
  current_driving_cell_info = current_driving_cell_info[1:len(current_driving_cell_info)]
  #print current_driving_cell_info
  async_ports = current_driving_cell_info[::2]
  async_ports_wires = current_driving_cell_info[1:][::2]
  fp.write('######creating nets and new flipflops cell for wire: '+wire+'\n')
  for idx1 in range(0,cascade_number):
    new_ff_name = 'new_reg_'+str(idx0)+'_'+str(idx1)
    new_net_name = 'new_wire_'+str(idx0)+'_'+str(idx1)
    fp.write('create_cell '+new_ff_name+' '+library+'/'+cell_name+'\n')
    fp.write('create_net '+new_net_name+'\n')
  fp.write('######connecting cascaded reg between port: '+ports[idx0]+' and wire: '+wire+' \n')
  for idx1 in range(0,cascade_number):
    ff_name = 'new_reg_'+str(idx0)+'_'+str(idx1)
    net_name = 'new_wire_'+str(idx0)+'_'+str(idx1)
    if idx1 < (cascade_number-1):
      next_net_name = wire.replace('[','').replace(']','')+'_wire'+str(idx1+1)
    else:
      next_net_name = wire
    fp.write('connect_net '+net_name+' '+ff_name+'/D'+'\n')
    fp.write('connect_net '+next_net_name+' '+ff_name+'/Q'+'\n')
    if idx1 == 0:
      fp.write('connect_net '+net_name+' '+ports[idx0]+'\n')
  fp.write('######connecting clock and reset ports to cascaded registers between port: '+ports[idx0]+' and wire: '+wire+' \n')
  for idx1 in range(0,cascade_number):
    ff_name = 'new_reg_'+str(idx0)+'_'+str(idx1)
    for idx2,async_port in enumerate(async_ports):
      fp.write('connect_net '+async_ports_wires[idx2]+' '+ff_name+'/'+async_port+'\n')
fp.write('set_fix_multiple_port_nets -all -buffer_constants -feedthroughs\n')
fp.write('change_name -hierarchy -rules verilog\n')
fp.write('write -hierarchy -format verilog -output '+output_verilog_file+'\n')
fp.write('exit\n')
fp.close()
subprocess.call(['dc_shell', '-f',dc_script], stdout=open(dc_work_dir+'/dc_shell_repipeline.log', 'w+'),cwd=dc_work_dir)





