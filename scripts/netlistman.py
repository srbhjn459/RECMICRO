import os, threading, time, subprocess, glob, math
import re
def get_instance_line(verilog,instance_name,module_name):
  verilog_str_arr = verilog.split('\n')
  for line in verilog_str_arr:
    if line.find(' '+instance_name+' ')>=0 and line.find(module_name)>=0 and line.find(';')>=0:
      return line
  return ''

def verilog_2_linewise_verilog(verilog):
  verilog_str_arr = verilog.split('\n')
  count = 0
  line_wise_ver_arr = []
  while count < len(verilog_str_arr):
    curr_line = ''
    new_line = verilog_str_arr[count]
    if new_line.find(';')>=0 or new_line.find('endmodule')>=0 or new_line.isspace() or new_line == '' or new_line == '\n':
      curr_line = new_line
      count = count+1
    else:
      while new_line.find(';')<0:
        curr_line = curr_line+new_line
        count = count+1
        new_line = verilog_str_arr[count]
      curr_line = curr_line+new_line
      count=count+1
    line_wise_ver_arr.append(curr_line)
  ret_str = '\n'.join(line_wise_ver_arr)
  return ret_str

def connections2portsNwires(line):
  ports = []
  wires = []
  line_arr = line.replace(';','').split('.')
  line_arr = line_arr[1:len(line_arr)]
  for each_el in line_arr:
    port = each_el.split('(')[0]
    ports.append(port)
  for each_el in line_arr:
    regex = re.compile(".*?\((.*?)\)")
    result = re.findall(regex,each_el)
    if len(result)>1:
      print 'FATAL:: tool cannot handle instantiation of type other than .<port name>(<wire name>)'
      exit()
    wires.append(result[0])
  return [ports,wires]

def line_2_module_prop(line):
  ret_list = []
  if line.find('(')>=0 and line.find(')')>=0 and line.find('module')<0 and line.find('input ')<0 and line.find('output ')<0 and line.find('wire ')<0:
    line = line.lstrip()
    line = line.replace(' ',',')
    line_arr = line.split(',')
    ret_list.append(line_arr[0])
    ret_list.append(line_arr[1].replace('(',''))
  else:
    ret_list.append('')
    ret_list.append('')
  return ret_list
    
class module_node(object):
  def __init__(self,module_name):
    self.module_name = module_name
    self.instance_name = ''
    self.absolute_instance_name = ''
    self.verilog = ''
    self.location = ''
    self.hetrogenous = False
    self.children = []
    self.async_ports = []
    self.ip_ports ={}
    self.op_ports ={}
    self.wires ={}
    self.parent = None

  def set_absolute_instance_name(self,name):
    self.absolute_instance_name = name
  
  def set_location(self,name):
    self.location = name

  def set_verilog(self,verilog):
    self.verilog=verilog
  def set_instance_name(self,name):
    self.instance_name = name
  
  def attach_child(self,obj):
    self.children.append(obj)
  
  def attach_async_ports(self,ports):
    self.async_ports = ports

  def attach_async_port(self,port):
    self.async_ports.append(port)
  def attach_parent(self,obj):
    self.parent = obj

  def get_absolute_instance_name(self):
    return self.absolute_instance_name

  def get_children(self):
    return self.children
  
  def get_parent(self):
    return self.parent

  def get_module_name(self):
    return self.module_name

  def get_instance_name(self):
    return self.instance_name

  def get_verilog(self):
    return self.verilog
  
  def get_het(self):
    return self.hetrogenous
  
  def reset_het(self):
    self.hetrogenous = False

  def set_het(self):
    self.hetrogenous = True
  
  def get_async_ports(self):
    return self.async_ports
  
  def attach_ip_port(self,key,val):
    self.ip_ports[key] = val
  
  def attach_op_port(self,key,val):
    self.op_ports[key] = val
  
  def attach_wire(self,key,val):
    self.wires[key] = val

  def get_wires(self):
    return self.wires

  def get_ip_ports(self):
    return self.ip_ports
  
  def get_op_ports(self):
    return self.op_ports
  
  def get_location(self):
    return self.location
  
  def get_sibblings(self):
    sibblings = []
    parent = self.parent
    if parent != None:
      children = parent.get_children()
      for child in children:
        if child.get_instance_name() != self.instance_name and child.get_module_name() != self.module_name:
          sibblings.append(child)
    return sibblings
  
  def if_leaf(self):
    if len(self.children) == 0:
      return True
    else:
      return False
