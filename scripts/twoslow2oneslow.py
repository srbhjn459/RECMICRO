import os, threading, time, subprocess, glob, math
import re
import sys
sys.setrecursionlimit(4000)
def Traverse(wire,graph,reg_level,reg,level):
  if wire in graph.keys():
    children = graph[wire].keys()
    for child in children:
      prop = graph[wire][child]
      if prop.split(',')[4] == 'nv':
        graph[wire][child] = prop.replace(',nv',',v')
        reg_inst = prop.split(',')[1]
        if reg_inst in reg:
          if reg_inst not in reg_level.keys():
            reg_level[reg_inst] = level
          else:
            curr_level = reg_level[reg_inst]
            if curr_level%2 != level%2:
              print 'FATAL:: Register: '+reg_inst+' has level: '+str(curr_level)+' and also level: '+str(level)
              exit()
            reg_level[reg_inst] = level
          Traverse(child,graph,reg_level,reg,level+1)
        else:
          Traverse(child,graph,reg_level,reg,level)
        


graph_file = './output_dir/graph_netlist.txt'
reg_file = './output_dir/registers_netlist.txt'
ip_port_file = './output_dir/ip_ports_netlist.txt'           #####input ports in the netlist excluding async pins (clk & rstn generally)
replacement_node_file = './output_dir//bypass_reg.txt'
other_node_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/nbypassable_registers_netlist.txt'
level_file = '/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir//reg_level_file.txt'

fp = open(reg_file,'r')
reg = [each_reg.replace('\n','') for each_reg in fp.readlines()]
fp.close()

graph = {}
fp = open(graph_file,'r')
line = 'saurabh'
while line:
  line = fp.readline()
  if line.find('=') >= 0:
    line_arr = line.split('=')
    wire = line_arr[0]
    wire_arr = wire.split(',')
    if wire_arr[0] not in graph.keys():
      graph[wire_arr[0]] = {}
      graph[wire_arr[0]][wire_arr[1]] = line_arr[1].replace('\n','')+',nv'
    else:
      graph[wire_arr[0]][wire_arr[1]] = line_arr[1].replace('\n','')+',nv'
fp.close()
fp = open(ip_port_file,'r')
ip_ports = [each_port.replace('\n','') for each_port in fp.readlines()]

reg_level = {}
print 'now finding level'
for each_port in ip_ports:
  Traverse(each_port,graph,reg_level,reg,0)    

fp = open(replacement_node_file,'w')
fp1 = open(other_node_file,'w')
for each_reg in reg_level.keys():
  if reg_level[each_reg]%2 == 1:
    for in_wire in graph.keys():
      curr_graph = graph[in_wire]
      for out_wire in curr_graph.keys():
        prop = graph[in_wire][out_wire]
        if prop.find(each_reg+',')>=0:
          fp.write(in_wire+':'+each_reg+':'+out_wire+'\n')
  else:
    for in_wire in graph.keys():
      curr_graph = graph[in_wire]
      for out_wire in curr_graph.keys():
        prop = graph[in_wire][out_wire]
        if prop.find(each_reg+',')>=0:
          fp1.write(in_wire+':'+each_reg+':'+out_wire+'\n')

fp.close()
fp1.close()


fp = open(level_file,"w")
for key in reg_level.keys():
  fp.write (key+':'+str(reg_level[key])+'\n')
fp.close()

for each_reg in reg:
  if each_reg not in reg_level.keys():
    print 'WARNING:: flip-flop: '+ each_reg+' not found through any data path'
#print graph
#print ip_ports

