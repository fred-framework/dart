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
set run.rmSynth        1
set run.prImpl         0
set run.prVerify       0
set run.writeBitstream 0
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
### RP Module Definitions
 ####################################################################
add_module dct
set_attribute module dct moduleName	top_fdct
set_attribute module dct prj 	$prjDir/dct.prj
set_attribute module dct synth 	${run.rmSynth}

add_module FFT_ifft
set_attribute module FFT_ifft moduleName	fft_top
set_attribute module FFT_ifft prj 	$prjDir/FFT_ifft.prj
set_attribute module FFT_ifft synth 	${run.rmSynth}

source $tclDir/run.tcl
exit
