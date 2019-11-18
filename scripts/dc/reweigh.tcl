remove_design -all
set target_library "/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db"
set link_library "/home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/gscl45nm.db"
read_file -format verilog /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO/input_dir/netlist.v
link
source /home/a0117963/sandbox19082015/intel_work/perl_cpp_env/RECMICRO//dc/timing.tcl > timing.txt
exit
