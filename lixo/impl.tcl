set tclParams [list hd.visual 1]
set tclHome "/home/lsa/repos/ampere-git/fred/dart/pr_tool-fork/lixo/Tcl"
set tclDir $tclHome
set projDir "/home/lsa/repos/ampere-git/fred/dart/pr_tool-fork/lixo"
 source $tclDir/design_utils.tcl
 source $tclDir/log_utils.tcl
 source $tclDir/synth_utils.tcl
 source $tclDir/impl_utils.tcl
 source $tclDir/pr_utils.tcl
 source $tclDir/log_utils.tcl
 source $tclDir/hd_floorplan_utils.tcl
############################################################### 
### Define Part, Package, Speedgrade 
###############################################################
set part xc7z020clg400-1
check_part $part
####flow control
set run.topSynth       0
set run.rmSynth        0
set run.prImpl         1
set run.prVerify       1
set run.writeBitstream 1
####Report and DCP controls - values: 0-required min; 1-few extra; 2-all
set verbose      1
set dcpLevel     1
 ####Output Directories
 set synthDir  $projDir/Synth
 set implDir   $projDir/Implement
 set dcpDir    $projDir/Checkpoint
 set bitDir    $projDir/Bitstreams
 ####Input Directories 
 set srcDir     $projDir/Sources
 set rtlDir     $srcDir/hdl
 set prjDir     $srcDir/project
 set xdcDir     $srcDir/xdc
 set coreDir    $srcDir/cores
 set netlistDir $srcDir/netlist
####################################################################
### Top Module Definitions
 ####################################################################
set top "system_wrapper_1_slots"
set static "Static" 
add_module $static
set_attribute module $static moduleName    $top
set_attribute module $static top_level     1
set_attribute module $static synth         ${run.topSynth}
####################################################################
### RP Module Definitions
 ####################################################################
add_module dct
set_attribute module dct moduleName	top_fdct

add_module FFT_ifft
set_attribute module FFT_ifft moduleName	fft_top

############################################################### 
###Implemenetation configuration 0
###############################################################
add_implementation config_0
set_attribute impl config_0 top 	   $top
set_attribute impl config_0 pr.impl 	 1
set_attribute impl config_0 implXDC 	 [list /home/lsa/repos/ampere-git/fred/dart/pr_tool-fork/lixo/Sources/constraints/pblocks.xdc]
set_attribute impl config_0 partitions 	[list [list $static           $top 	implement   ] \
	 	 	 	 	[list FFT_ifft	 system_i/slot_p0_s0 implement] \
	 	 	 	 	[list dct	 system_i/slot_p0_s1 implement] \
]

set_attribute impl config_0 impl 	    ${run.prImpl} 
set_attribute impl config_0 verify 	   ${run.prVerify} 
set_attribute impl config_0 bitstream 	 ${run.writeBitstream} 
set_attribute impl config_0 bitstream_options    "-bin_file"
source $tclDir/run.tcl
exit
