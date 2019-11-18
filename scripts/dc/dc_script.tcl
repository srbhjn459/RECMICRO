set target_library "/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db"
set link_library "/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db"
remove_design -all
analyze -format verilog /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/bypassable_flop.v
elaborate bypassable_flop
analyze -format verilog /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/bypassable_flop_CDN.v
elaborate bypassable_flop_CDN
analyze -format verilog /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/bypassable_flop_SDN.v
elaborate bypassable_flop_SDN
read_file -format verilog /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/netlist.v
link
create_port dont_bypass
create_net dont_bypass
connect_net dont_bypass dont_bypass
create_cell tiel_inst gscl45nm/TIEL
create_net ground
connect_net ground tiel_inst/ZN
######creating new clock gate cell : clk_gate_0 and net : clk_gate_wire_0
create_cell clk_gate_0 gscl45nm/CLKGATE
create_net clk_gate_wire_0
connect_net clk_gate_wire_0 clk_gate_0/Q
connect_net CLOCK clk_gate_0/CPN
connect_net dont_bypass clk_gate_0/E
connect_net ground clk_gate_0/TE
######disconnecting wires from existing gate: FF3
disconnect_net IN FF3/D
disconnect_net wout_3 FF3/Q
disconnect_net CLOCK FF3/CLK
######creating new bypassable_ff cells to replace gate: FF3
create_cell by_FF3 bypassable_flop
######deleting the existing gate: FF3
remove_cell FF3
######connecting pins of new gate: by_FF3
connect_net IN by_FF3/D
connect_net IN by_FF3/DB
connect_net wout_3 by_FF3/Q
connect_net clk_gate_wire_0 by_FF3/CP
connect_net CLOCK by_FF3/CLK
connect_net dont_bypass by_FF3/BBAR
######disconnecting wires from existing gate: FF10
disconnect_net win_10 FF10/D
disconnect_net wout_10 FF10/Q
disconnect_net CLOCK FF10/CLK
######creating new bypassable_ff cells to replace gate: FF10
create_cell by_FF10 bypassable_flop
######deleting the existing gate: FF10
remove_cell FF10
######connecting pins of new gate: by_FF10
connect_net win_10 by_FF10/D
connect_net win_10 by_FF10/DB
connect_net wout_10 by_FF10/Q
connect_net clk_gate_wire_0 by_FF10/CP
connect_net CLOCK by_FF10/CLK
connect_net dont_bypass by_FF10/BBAR
uniquify -force
set_fix_multiple_port_nets -all -buffer_constants -feedthroughs
change_name -hierarchy -rules verilog
write -hierarchy -format verilog -output /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/output_dir/netlist_rec.v
exit
