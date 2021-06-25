# check if the IP has a compatible interface with DART,
# if some of the required ports are missing in the IP, then it returns 0, 
# otherwise, it returns 1
#proc check_ip_interface { ip_dcp_filename } {

	# get the list of ports from the IP
	#open_checkpoint  $ip_dcp_filename
	open_checkpoint "/home/lsa/repos/ampere-git/fred/dart/pr_tool-fork/designs/sum_vec/wo_part/Synth/sum_vec/acc_0_synth.dcp"
	set port_list [get_ports *]
	set $port_list [lsort -dictionary $port_list]
	puts "$port_list"
	# 
	set ip_unique_ports {}

	foreach port $port_list {
		# get the position of the character [
		set char_pos [string last "\[" $port ]
		#puts "$char_pos != -2"
		if {$char_pos != -1} {
			# remove the [x] of the port name
			set port [string range $port 0 $char_pos-1]
		}
		puts $port
		# only add the 2nd list if the port name is new
		if {[lsearch -exact $ip_unique_ports $port] < 0} {
			puts "new port $port"
			lappend ip_unique_ports $port
		}
	}
	puts $ip_unique_ports

	# this is the list of required ports to the IP
	# if some of these ports are missing in the IP, then it returns 0
	set required_ports "
s_axi_ctrl_bus_AWVALID
m_axi_mem_bus_ARADDR
s_axi_ctrl_bus_AWREADY
s_axi_ctrl_bus_AWADDR
s_axi_ctrl_bus_WVALID
s_axi_ctrl_bus_WREADY
s_axi_ctrl_bus_WDATA
s_axi_ctrl_bus_WSTRB
s_axi_ctrl_bus_ARVALID
s_axi_ctrl_bus_ARREADY
s_axi_ctrl_bus_ARADDR
s_axi_ctrl_bus_RVALID
s_axi_ctrl_bus_RREADY
s_axi_ctrl_bus_RDATA
s_axi_ctrl_bus_RRESP
s_axi_ctrl_bus_BVALID
s_axi_ctrl_bus_BREADY
s_axi_ctrl_bus_BRESP
ap_clk
ap_rst_n
interrupt
m_axi_mem_bus_AWVALID
m_axi_mem_bus_AWREADY
m_axi_mem_bus_AWADDR
m_axi_mem_bus_AWID
m_axi_mem_bus_AWLEN
m_axi_mem_bus_AWSIZE
m_axi_mem_bus_AWBURST
m_axi_mem_bus_AWLOCK
m_axi_mem_bus_AWCACHE
m_axi_mem_bus_AWPROT
m_axi_mem_bus_AWQOS
m_axi_mem_bus_AWREGION
m_axi_mem_bus_AWUSER
m_axi_mem_bus_WVALID
m_axi_mem_bus_WREADY
m_axi_mem_bus_WDATA
m_axi_mem_bus_WSTRB
m_axi_mem_bus_WLAST
m_axi_mem_bus_WID
m_axi_mem_bus_WUSER
m_axi_mem_bus_ARVALID
m_axi_mem_bus_ARREADY
m_axi_mem_bus_ARID
m_axi_mem_bus_ARLEN
m_axi_mem_bus_ARSIZE
m_axi_mem_bus_ARBURST
m_axi_mem_bus_ARLOCK
m_axi_mem_bus_ARCACHE
m_axi_mem_bus_ARPROT
m_axi_mem_bus_ARQOS
m_axi_mem_bus_ARREGION
m_axi_mem_bus_ARUSER
m_axi_mem_bus_RVALID
m_axi_mem_bus_RREADY
m_axi_mem_bus_RDATA
m_axi_mem_bus_RLAST
m_axi_mem_bus_RID
m_axi_mem_bus_RUSER
m_axi_mem_bus_RRESP
m_axi_mem_bus_BVALID
m_axi_mem_bus_BREADY
m_axi_mem_bus_BRESP
m_axi_mem_bus_BID
m_axi_mem_bus_BUSER
"
	set $required_ports [lsort $required_ports]

	# check if all IP ports are required
	foreach port $ip_unique_ports {
		# ni operator is 'not in'
		#if {[get_property NAME [get_ports $port]] ni  $required_ports} {
		if {[lsearch -exact $required_ports $port] < 0} {
			puts "nao achei $port"
			return 0
		} else {
			puts "achei  $port"
		}
	}

	# check if all required ports are in the IP
	foreach port $required_ports {
		if {[get_property NAME [get_ports $port]] ni  $ip_unique_ports} {
			puts "nao achei $port"
			return 0
		} 
		else {
			puts "achei  $port"
			return 0
		}
	}


	#if it got to this point, than the IP has a compatiple interface
#	return 1;
#}

# https://forums.xilinx.com/t5/Vivado-TCL-Community/Using-Tcl-to-set-a-verilog-hard-coded-value-pre-synthesis/td-p/654116
# https://forums.xilinx.com/t5/Vivado-TCL-Community/Get-generic-value-from-Tcl/td-p/780458
# get_property my_generic [get_cells my_mod_i0]
# report_property [get_ports m_axi_mem_bus_RDATA[0]]

#check_ip_interface "/home/lsa/repos/ampere-git/fred/dart/pr_tool-fork/designs/sum_vec/wo_part/Synth/sum_vec/acc_0_synth.dcp"