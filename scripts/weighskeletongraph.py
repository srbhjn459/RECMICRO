import os, threading, time, subprocess, glob, math, sys
import re

#if len(sys.argv)<4:
#  print "Usage: python weighskeletongraph.py <skeleton_graph_file> <input_verilog_file> <std_cell_library>"
#  print "eg: python weighskeletongraph.py skl_graph_file.txt ./processed_rtl/FIR_lvt.v tcbn40lpbwplvtwc.db"
#  exit()
#skeleton_graph_file = sys.argv[1];
#input_verilog_file = sys.argv[2];
#std_cell_technology = sys.argv[3];

design_dir = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/'
#skeleton_graph_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/skl_graph.txt'
skeleton_graph_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/skl_graph_netlist.txt'
#input_verilog_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/FIR_lvt.v';
input_verilog_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/netlist.v'; #give absolute path
#input_port_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/ip_ports.txt'
input_port_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/ip_ports_netlist.txt'
#output_port_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/op_ports.txt'
output_port_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/op_ports_netlist.txt'
#registers_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/registers.txt'
registers_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/registers_netlist.txt'
std_cell_technology = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db';
nl_graph = {}

if not os.path.isdir(design_dir):
  os.mkdir(design_dir)
if not os.path.isdir(design_dir+'/dc'):
  os.mkdir(design_dir+'/dc')

output_dc_file = design_dir+'/dc'+'/dc_log.log'
ifp = open(skeleton_graph_file)
reg_assoc = []
line = ifp.readline()
while line:
  arr = []
  line = line.split('=')[0]
  line = line.split('->')
  launching_flop = line[0].split(':')[1]
  launching_flop_ip_net = line[0].split(':')[0]
  launching_flop_op_net = line[0].split(':')[2]
  launching_pin = 'Q'
  capturing_flop = line[1].split(':')[1]
  capturing_flop_ip_net = line[1].split(':')[0]
  capturing_flop_op_net = line[1].split(':')[2]
  capturing_pin = 'D'
  arr.append(launching_flop)
  arr.append(launching_flop_ip_net)
  arr.append(launching_flop_op_net)
  arr.append(launching_pin)
  arr.append(capturing_flop)
  arr.append(capturing_flop_ip_net)
  arr.append(capturing_flop_op_net)
  arr.append(capturing_pin)
  if (launching_flop == 'dummy' or capturing_flop == 'dummy_out'):
    arr.append('0')
  else:
    arr.append('1')
  reg_assoc.append(arr)
  line = ifp.readline()
ifp.close()
ip_ports = open(input_port_file).read().split('\n')
ip_ports = ip_ports[0:len(ip_ports)-1]
op_ports = open(output_port_file).read().split('\n')
op_ports = op_ports[0:len(op_ports)-1]
registers = open(registers_file).read().split('\n')
registers = registers[0:len(registers)-1]

ofp = open(design_dir+'/dc/timing.tcl',"w")
count = 0
for element in reg_assoc:
  launching_flop = element[0]
  launching_flop_ip_net = element[1]
  launching_flop_op_net = element[2]
  launching_pin = element[3]
  capturing_flop = element[4]
  capturing_flop_ip_net = element[5]
  capturing_flop_op_net = element[6]
  capturing_pin = element[7]
  validity = element[8]
  if validity == '1':
    if launching_flop in ip_ports and capturing_flop in registers: ##in2reg
      ofp.write(' report_timing -from '+launching_flop+' -to ')
      ofp.write(capturing_flop+'/'+capturing_pin+'\n')
    elif launching_flop in registers and capturing_flop in registers: ##reg2reg
      ofp.write(' report_timing -from '+launching_flop+'/'+launching_pin+' -to ')
      ofp.write(capturing_flop+'/'+capturing_pin+'\n')
    elif launching_flop in registers and capturing_flop_op_net in op_ports:  ##reg2out
      ofp.write(' report_timing -from '+launching_flop+'/'+launching_pin+' -to ')
      ofp.write(capturing_flop_op_net+'\n')
ofp.close()
ofp = open(design_dir+'/dc/reweigh.tcl',"w")
ofp.write('remove_design -all\n')
ofp.write('set target_library '+ '"'+std_cell_technology+'"\n')
ofp.write('set link_library '+ '"'+std_cell_technology+'"\n')
ofp.write('read_file -format verilog '+input_verilog_file+'\n')
ofp.write('link\n')
ofp.write('source '+design_dir+'/dc/timing.tcl > timing.txt\n')
ofp.write('exit\n')
ofp.close()
subprocess.call(['dc_shell', '-f', design_dir+'/dc/reweigh.tcl'], stdout=open(output_dc_file, 'w+'),cwd = design_dir+'/dc')
if not os.path.isfile(design_dir+'/dc/timing.txt'):
  print 'FATAL:: file '+design_dir+'/dc/timing.txt not found'
  exit()
ifp = open(design_dir+'/dc/timing.txt')
line = ifp.readline()
timing = []
while line:
  if line.find('data arrival time')>=0:
    line = line.replace('data','').replace('arrival','').replace('time','').replace(' ','').replace('\n','')
    timing.append(float(line))
  elif line.find('Error')>=0:
    print 'FATAL:: Error found in finding weights of timing arcs with design compiler'
    print 'see file '+output_dc_file
    exit()
  line = ifp.readline()
ifp.close()
ifp = open(skeleton_graph_file)
lines = []
idx = 0
for element in reg_assoc:
  line = ifp.readline()
  if element[8] == '1':
    line = line.split("=")[0]
    weight = timing[idx]
    line = line+'='+str(weight)+'\n'
    idx=idx+1
  lines.append(line)
ifp.close()
skeleton_graph_file_v2 = skeleton_graph_file
ofp = open(skeleton_graph_file_v2,"w")
for line in lines:
  ofp.write(line)
ofp.close()

print "files modified are:"
print "1)"+skeleton_graph_file_v2
