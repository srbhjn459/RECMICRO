import os, threading, time, subprocess, glob, math
import re
from netlistman import *



####input parameters
replacement_node_file = './output_dir/bypassable_registers_netlist.txt'
input_verilog_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/netlist.v'
std_cell_technology = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db'
bypassable_flops_file = ['/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/bypassable_flop.v','/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/bypassable_flop_CDN.v','/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/bypassable_flop_SDN.v']
no_of_flops_per_gate = 8 ##will make 1 clock gate for all flops 
library = 'gscl45nm'
clk_gate_cell_name = 'CLKGATE' ### name of the clock gating cell in the library
cg_in_all = False   #clock gate in non bypassable register for tree balancing
other_node_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/nbypassable_registers_netlist.txt'
tieh_cell_name = 'TIEH'
tiel_cell_name = 'TIEL'
clk_port = 'CLOCK'
####output parameters
dc_script = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/dc/dc_script.tcl'
dc_work_dir = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/dc/'
output_verilog_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/netlist_rec.v'
new_port_name = 'dont_bypass' ##something equivalent to "dont bypass"

fp = open(replacement_node_file)
flip_flop_instNwires = [line.replace('\n','') for line in fp.readlines()]
fp.close()
nbflip_flop_instNwires = []
if cg_in_all:
  fp = open(other_node_file)
  nbflip_flop_instNwires = [line.replace('\n','') for line in fp.readlines()]
  fp.close()

###get reset and clock connections
cell_info = []
verilog_of_mod = open(input_verilog_file).readlines()
verilog_of_mod = ''.join(verilog_of_mod)
verilog_of_mod = verilog_2_linewise_verilog(verilog_of_mod)
for connection in flip_flop_instNwires:
  temp_arr = []
  curr_gate = connection.split(':')[1]
  instantiation = get_instance_line(verilog_of_mod,curr_gate,'')
  [module_name,curr_gate] = line_2_module_prop(instantiation)
  instantiation = instantiation.replace(' '+curr_gate+' ','').replace(' ','')
  instantiation = instantiation.replace(module_name+'(','')
  instantiation = instantiation.replace(';','')
  instantiation = instantiation[:-1]
  [ports_arr,wires_arr]=connections2portsNwires(instantiation)
  temp_arr.append(module_name)
  for idx,port_val in enumerate(ports_arr):
    if port_val != 'Q' and port_val != 'D':
      temp_arr.append(port_val)
      temp_arr.append(wires_arr[idx])
  cell_info.append(temp_arr)
fp = open(dc_script,'w')
fp.write('set target_library '+ '"'+std_cell_technology+'"\n')
fp.write('set link_library '+ '"'+std_cell_technology+'"\n')
fp.write('remove_design -all\n')
for i in range(0,len(bypassable_flops_file)):
  fp.write('analyze -format verilog '+bypassable_flops_file[i]+'\n')
  if i == 0:
    fp.write('elaborate bypassable_flop\n')
  elif i == 1:
    fp.write('elaborate bypassable_flop_CDN\n')
  else:
    fp.write('elaborate bypassable_flop_SDN\n')
fp.write('read_file -format verilog '+input_verilog_file+'\n')
fp.write('link\n')
fp.write('create_port '+new_port_name+'\n')
fp.write('create_net '+new_port_name+'\n')
fp.write('connect_net '+new_port_name+' '+new_port_name+'\n')      
fp.write('create_cell tiel_inst '+library+'/'+tiel_cell_name+'\n')
fp.write('create_net ground\n')
fp.write('connect_net ground '+'tiel_inst/ZN\n')
for idx,connection in enumerate(flip_flop_instNwires):
  current_cell_info = cell_info[idx]
  gate_type = current_cell_info[0]
  input_wire = connection.split(':')[0]
  gate_name = connection.split(':')[1]
  output_wire = connection.split(':')[2]
  current_cell_info = current_cell_info[1:]
  async_ports = current_cell_info[::2]
  async_ports_wires = current_cell_info[1:][::2]
  new_gate_name = 'by_'+gate_name
  if idx%no_of_flops_per_gate == 0:
    new_clock_gate_cell = 'clk_gate_'+str(idx)
    new_clock_gate_cell_out = 'clk_gate_wire_'+str(idx)
    fp.write('######creating new clock gate cell : '+new_clock_gate_cell+' and net : '+new_clock_gate_cell_out+'\n')
    fp.write('create_cell '+new_clock_gate_cell+' '+library+'/'+clk_gate_cell_name+'\n')
    fp.write('create_net '+new_clock_gate_cell_out+'\n')
    fp.write('connect_net '+new_clock_gate_cell_out+' '+new_clock_gate_cell+'/Q\n')
    fp.write('connect_net '+clk_port+' '+new_clock_gate_cell+'/CPN\n')
    fp.write('connect_net '+new_port_name+' '+new_clock_gate_cell+'/E\n')
    fp.write('connect_net ground'+' '+new_clock_gate_cell+'/TE\n')
  fp.write('######disconnecting wires from existing gate: '+gate_name+'\n')
  fp.write('disconnect_net '+input_wire+' '+gate_name+'/D\n')
  fp.write('disconnect_net '+output_wire+' '+gate_name+'/Q\n')
  flop_type = 'bypassable_flop'
  for idx0,async_port in enumerate(async_ports):
    fp.write('disconnect_net '+async_ports_wires[idx0]+' '+gate_name+'/'+async_port+'\n')
    if async_port == 'CDN':
      flop_type = 'bypassable_flop_CDN'
    if async_port == 'SDN':
      flop_type = 'bypassable_flop_SDN'
  fp.write('######creating new bypassable_ff cells to replace gate: '+gate_name+'\n')
  fp.write('create_cell '+new_gate_name+' '+flop_type+'\n')
  fp.write('######deleting the existing gate: '+gate_name+'\n')
  fp.write('remove_cell '+gate_name+'\n')
  fp.write('######connecting pins of new gate: '+new_gate_name+'\n')
  fp.write('connect_net '+input_wire+' '+new_gate_name+'/D'+'\n')
  fp.write('connect_net '+input_wire+' '+new_gate_name+'/DB'+'\n')
  fp.write('connect_net '+output_wire+' '+new_gate_name+'/Q'+'\n')
  fp.write('connect_net '+new_clock_gate_cell_out+' '+new_gate_name+'/CP\n')
  for idx0,async_port in enumerate(async_ports):
    if async_port != 'CP':
      fp.write('connect_net '+async_ports_wires[idx0]+' '+new_gate_name+'/'+async_port+'\n')
  fp.write('connect_net '+new_port_name+' '+new_gate_name+'/BBAR'+'\n')
#
#
if cg_in_all:
  fp.write('create_cell tieh_inst '+library+'/'+tieh_cell_name+'\n')
  fp.write('create_net supply\n')
  fp.write('connect_net supply '+'tieh_inst/Z\n')
  fp.write('######creating clock gates for non bypassable flops ##############\n')
  for idx,connection in enumerate(nbflip_flop_instNwires):
    gate_name = connection.split(':')[1]
    fp.write('disconnect_net '+clk_port+' '+gate_name+'/CP'+'\n')
    if idx%no_of_flops_per_gate == 0:
      new_clock_gate_cell = 'clk_gate_nb_'+str(idx)
      new_clock_gate_cell_out = 'clk_gate_nb_wire_'+str(idx)
      fp.write('######creating new clock gate cell : '+new_clock_gate_cell+' and net : '+new_clock_gate_cell_out+'\n')
      fp.write('create_cell '+new_clock_gate_cell+' '+library+'/'+clk_gate_cell_name+'\n')
      fp.write('create_net '+new_clock_gate_cell_out+'\n')
      fp.write('connect_net '+new_clock_gate_cell_out+' '+new_clock_gate_cell+'/Q\n')
      fp.write('connect_net '+clk_port+' '+new_clock_gate_cell+'/CPN\n')
      fp.write('connect_net supply'+' '+new_clock_gate_cell+'/E\n')
      fp.write('connect_net ground'+' '+new_clock_gate_cell+'/TE\n')
    fp.write('######connecting clock gate cell : '+new_clock_gate_cell+' output : '+new_clock_gate_cell_out+' to CP pin of flop : '+gate_name+'\n')
    fp.write('connect_net '+new_clock_gate_cell_out+' '+gate_name+'/CP\n')
 

fp.write('uniquify -force\n')
fp.write('set_fix_multiple_port_nets -all -buffer_constants -feedthroughs\n')
fp.write('change_name -hierarchy -rules verilog\n')
fp.write('write -hierarchy -format verilog -output '+output_verilog_file+'\n')
fp.write('exit\n')
fp.close()
#subprocess.call(['dc_shell', '-f',dc_script], stdout=open(dc_work_dir+'/dc_log', 'w+'),cwd=dc_work_dir)
