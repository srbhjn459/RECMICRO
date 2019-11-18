##############################################################
##############################################################
###Author(s): Saurabh Jain                                 ###
###Usage: 1. Converts a netlist to graph (dump to file)    ###
###       2. Creates a file of all the flip-flops          ###
###       3. Creates a file of all the inputs in design    ###
###       4. Creates a file of all the outputs in design   ###
###       5. creates a file of all the inputs of std-cells ###
###       6. Creates a file of all the outputs of std-cells###
###       7. Inputs:                                       ###
###            a. Flattened verilog netlist of module      ###
###            b. Standard cell verilog path               ###
###            c. List of asynchronous ports in design     ###
###            d. List of asynchronous ports in std-cells  ###
###            e. Identification pin of flip-flop          ###
###       8. Outputs:                                      ###
###            a.file of all the flip-flops                ###
###            b.file of all the inputs in design          ###
###            c.file of all the outputs in design         ###
###            d.file of all the inputs of std-cells       ###
###            e.file of all the outputs of std-cells      ###
###            f.file containing graph representation of   ###
###              netlist                                   ###
##############################################################
##############################################################


import os, threading, time, subprocess, glob, math
import re


def get_connection_port (connection_arr):
  ports = []
  for connection in connection_arr:
    connection = connection.split('(')
    connection = connection[0].replace('.','')
    ports.append(connection)
  return ports


def get_connection_net (connection_arr):
  nets = []
  for connection in connection_arr:
    connection = connection.split('(')
    connection = connection[1].replace(')','')
    nets.append(connection)
  return nets

def get_prop_for_instance(graph,instance):
  prop = []
  for key in graph.keys():
    prop = graph[key]
    if prop[1] == instance:
      prop.append(key)
      return prop
  return prop

    


#### input files/parameters
#ip_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/reconf_tool/FIR_lvt/FIR_lvt.v'     #####flattened verilog netlist
ip_file = './input_dir/netlist.v'     #####flattened verilog netlist
ip_tech_file = './input_dir/gscl45nm.v'       #####standard cell verilog file
#async_ports = ['clk_i','rst_ni','clock_en_i','test_en_i']   #####asynchronous ports in module
async_ports = ['CLOCK']   #####asynchronous ports in module
async_ports_cell = ['CLK','S','R']     #####asynchronous pins in all the standard cells
register_iden_pin = 'CLK'                  #####pin to be used for identifying flip flop


#### output files/parameters
op_graph_file = './output_dir/graph_netlist.txt'          #####output graph file wire = node & gate = edge
cell_ip_port_info_file = './output_dir/cellIp.txt'   #####input info of each standard cell 
cell_op_port_info_file = './output_dir/cellOp.txt'   #####output info of each standard cell
ip_port_file = './output_dir/ip_ports_netlist.txt'           #####input ports in the netlist excluding async pins (clk & rstn generally)
op_port_file = './output_dir/op_ports_netlist.txt'           #####output ports in the netlist  
register_file = './output_dir/registers_netlist.txt'         #####all the flip-flops in the design  
driven_port_file = './output_dir/registerPorts_netlist.txt'         #####all the flip-flops in the design  
driven_wire_file = './output_dir/registerWires_netlist.txt'         #####all the flip-flops in the design  




it_fp = open(ip_tech_file)
line = it_fp.readline() 
##extract ip and op ports/pins of std cells
cell_dict = {}
while line:
  if line.find('module')>=0:
    name = line.replace('module','').replace(' ','').replace('\n','').replace(';','')
    regex = re.compile(".*?\((.*?)\)")
    result = re.findall(regex,name)
    if len(result) > 0:
      name = name.replace(result[0],'')
    name = name.replace('()','')
    cell_dict[name] = {}
    cell_dict[name]['input'] = []
    cell_dict[name]['output'] = []
    while line.find('endmodule') < 0:
      if line.find('input')>=0:
        line = line.replace('input','').replace(';','').replace('\n','').replace(' ','')
        arr = line.split(',')
        cell_dict[name]['input'] =cell_dict[name]['input']+arr
      elif line.find('output')>=0:
        line = line.replace('output','').replace(';','').replace('\n','').replace(' ','')
        arr = line.split(',')
        cell_dict[name]['output'] =cell_dict[name]['output'] + arr
      line = it_fp.readline()
  line = it_fp.readline()
it_fp.close()
####dump the ip ports of std cell to a file
o_fp = open(cell_ip_port_info_file,'w')
for key in cell_dict.keys():
  if 'input' in cell_dict[key].keys():
    ip_pins = cell_dict[key]['input']
  else: 
    ip_pins = []
  ip_string = ",".join(ip_pins)
  o_fp.write(key+'='+ip_string+'\n')
o_fp.close()
#
#
####dump the op ports of std cell to a file
o_fp = open(cell_op_port_info_file,'w')
for key in cell_dict.keys():
  if 'output' in cell_dict[key].keys():
    op_pins = cell_dict[key]['output']
  else:
    op_pins = []
  op_string = ",".join(op_pins)
  o_fp.write(key+'='+op_string+'\n')
o_fp.close()


####create a graph file, all the flip-flop database 
i_fp = open(ip_file)
o_fp = open(op_graph_file,'w')
line = i_fp.readline() 
registers_instance = []
count = 0
nl_graph = {}
while line:
  appline = ''
  while line.find(';')<0 and line.find('endmodule')<0 and line != '':
    appline = appline +line
    line = i_fp.readline() 
    appline = appline.replace('\n','')
  appline = appline+line
  appline = appline.replace(';','').replace('\n','')
  if appline.find('(')>=0 and appline.find(')')>=0 and appline.find('module ')<0:
    appline = appline.strip()
    module_name = appline.split(' ')[0]
    instance_name = appline.split(' ')[1]
    count = count+1
    appline = appline.replace(module_name,'').replace(instance_name,'').replace(' ','')
    connection_arr = appline.split(',')
    connection_arr[0] = connection_arr[0].lstrip('(')
    connection_arr[len(connection_arr)-1] = connection_arr[len(connection_arr)-1].rstrip(')')
    connection_port_arr = get_connection_port(connection_arr)
    connection_net_arr = get_connection_net(connection_arr)
    ip_array = []
    op_array = []
    if 'input' in cell_dict[module_name].keys():
      ip_array = cell_dict[module_name]['input']
    if register_iden_pin in ip_array:
      registers_instance.append(instance_name)
    if 'output' in cell_dict[module_name].keys():
      op_array = cell_dict[module_name]['output']
    for ipin in ip_array:
      ipin_idx = connection_port_arr.index(ipin) if ipin in connection_port_arr else -1
      if ipin_idx != -1:
        ip_net = connection_net_arr[ipin_idx]
      else:
        print 'WARNING::No connection to input port '+ipin+' in module: '+module_name+' instantiated as '+instance_name
      for opin in op_array:
        #in cells if input pins/ports are asynchronous they do not go into edges
        if ipin not in async_ports_cell:
          opin_idx = connection_port_arr.index(opin) if opin in connection_port_arr else -1
          if opin_idx != -1:
            op_net = connection_net_arr[opin_idx]
          else:
            print 'WARNING::No connection to output port '+opin+' in module: '+module_name+' instantiated as '+instance_name
          o_fp.write(ip_net+','+op_net+'='+ipin+','+instance_name+','+module_name+','+opin+'\n')
          temp_arr = []
          temp_arr.append(ipin)
          temp_arr.append(instance_name)
          temp_arr.append(module_name)
          temp_arr.append(opin)
          nl_graph[ip_net+','+op_net] = temp_arr
  line = i_fp.readline() 
i_fp.close()
o_fp.close()

#### extract out input and output ports/pins in a design
i_fp = open(ip_file)
line = i_fp.readline()
input_ports = []
output_ports = []
while line:
  if line.find('input ')>=0:
    appline = ''
    while line.find(';')<0:
      appline = appline+line.replace('\n','')
      line = i_fp.readline()
    appline = appline+line.replace('\n','')
    appline = appline.strip().replace('input','').replace(';','')
    ip_array = appline.split(',')
    for ip in ip_array:
      if ip.find('[') >=0 and ip.find(']')>=0:
        ip_pin = ip.split(']')[1]
        regex = re.compile(".*?\[(.*?)\]")
        result = re.findall(regex,ip)
        result_arr = result[0].split(':')
        for i in range(int(result_arr[1]),int(result_arr[0])+1):
          curr_pin = ip_pin+'['+str(i)+']'
          curr_pin = curr_pin.replace(' ','')
          input_ports.append(curr_pin)
      else:
        ip = ip.replace(' ','')
        input_ports.append(ip)
  elif line.find('output ')>=0:
    appline = ''
    while line.find(';')<0:
      appline = appline+line.replace('\n','')
      line = i_fp.readline()
    appline = appline+line.replace('\n','')
    appline = appline.strip().replace('output','').replace(';','')
    op_array = appline.split(',')
    for op in op_array:
      if op.find('[') >=0 and op.find(']')>=0:
        op_pin = op.split(']')[1]
        regex = re.compile(".*?\[(.*?)\]")
        result = re.findall(regex,op)
        result_arr = result[0].split(':')
        for i in range(int(result_arr[1]),int(result_arr[0])+1):
          curr_pin = op_pin+'['+str(i)+']'
          curr_pin = curr_pin.replace(' ','')
          output_ports.append(curr_pin)
      else:
        op = op.replace(' ','')
        output_ports.append(op)

  line = i_fp.readline()
i_fp.close()


#### dump extracted input pins/ports of design into a text file (excluding all the asynchronous ports)
o_fp = open(ip_port_file,'w')


for pin in async_ports: 
  if pin in input_ports:
    input_ports.remove(pin)

for ip_port in input_ports:
  o_fp.write(ip_port+'\n')
o_fp.close()

#### dump extracted output pins/ports of design into a text file
o_fp = open(op_port_file,'w')
for op_port in output_ports:
  o_fp.write(op_port+'\n')
o_fp.close()

#### dump all the flip-flops in the design into a file
o_fp = open(register_file,'w')
for register in registers_instance:
  o_fp.write(register+'\n')
o_fp.close()

o_fp = open(driven_port_file,'w')
o_fp1 = open(driven_wire_file,'w')

for register in registers_instance:
  properties = get_prop_for_instance(nl_graph,register)
  if len(properties) > 0:
    o_fp.write(register+'/'+properties[3]+'\n')
    o_fp1.write(properties[4].split(',')[1]+'\n')
o_fp.close()
o_fp1.close()

print 'files formed are: '
print '1)'+op_graph_file+' (graph file)'
print '2)'+ip_port_file+' (input ports in design,excluding async ports)'
print '3)'+op_port_file+' (output ports in design)'
print '4)'+register_file+' (all the flip-flops in the design)'  
print '5)'+cell_ip_port_info_file+' (input ports/pins of std cells)'
print '6)'+cell_op_port_info_file+' (output port/pins of std cells)'
print '7)'+driven_port_file+' (Flop driving ports)'
print '8)'+driven_wire_file+' (driven net from register ports)'

print 'file stats:'
print '1)No of gates: '+str(count)
print '1)No of seq gates: '+str(len(registers_instance))
