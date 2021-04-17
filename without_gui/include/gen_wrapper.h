#include <fstream>

#define create_acc_wrapper(top, rm_tag, partition_id)\
{\
    ofstream acc_wrapper;  \
    std::string wrapper_new_top =  Src_path + "/cores/" + rm_tag + "/hdl/verilog/wrapper_top.v"; \
    acc_wrapper.open(wrapper_new_top);\
    \
    acc_wrapper << "// Copyright (c) 2020-2021 Scoula Superiore Sant'Anna, Retis lab" <<endl;\
    acc_wrapper << "// This is the hdl top wrapper for reconfigurable accelerators" <<endl <<endl;\
    acc_wrapper << "// Author -- Biruk Seyoum" <<endl <<endl;\
    \
    acc_wrapper << "module " << wrapper_top_name <<"_"<< partition_id<<" (" <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_AWVALID," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_AWREADY," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_AWADDR," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_WVALID," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_WREADY," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_WDATA," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_WSTRB," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_ARVALID," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_ARREADY," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_ARADDR," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_RVALID," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_RREADY," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_RDATA," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_RRESP," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_BVALID," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_BREADY," <<endl;\
    acc_wrapper << "\t s_axi_ctrl_bus_BRESP," <<endl;\
    acc_wrapper << "\t ap_clk," <<endl;\
    acc_wrapper << "\t ap_rst_n," <<endl;\
    acc_wrapper << "\t interrupt," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWVALID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWREADY," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWADDR," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWLEN," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWSIZE," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWBURST," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWLOCK," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWCACHE," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWPROT," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWQOS," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWREGION," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_AWUSER," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WVALID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WREADY," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WDATA," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WSTRB," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WLAST," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_WUSER," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARVALID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARREADY," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARADDR," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARLEN," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARSIZE," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARBURST," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARLOCK," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARCACHE," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARPROT," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARQOS," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARREGION," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_ARUSER," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RVALID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RREADY," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RDATA," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RLAST," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RUSER," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_RRESP," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_BVALID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_BREADY," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_BRESP," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_BID," <<endl;\
    acc_wrapper << "\t m_axi_mem_bus_BUSER" <<endl;\
    acc_wrapper << ");" <<endl <<endl <<endl;\
    \
    acc_wrapper << "parameter    C_S_AXI_CTRL_BUS_DATA_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_S_AXI_CTRL_BUS_ADDR_WIDTH = 7;" <<endl;\
    acc_wrapper << "parameter    C_S_AXI_DATA_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_S_AXI_ADDR_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_ID_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_ADDR_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_DATA_WIDTH = 64;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_AWUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_ARUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_WUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_RUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_BUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_USER_VALUE = 0;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_PROT_VALUE = 0;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_MEM_BUS_CACHE_VALUE = 3;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_ID_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_ADDR_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_DATA_WIDTH = 32;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_AWUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_ARUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_WUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_RUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter    C_M_AXI_BUSER_WIDTH = 1;" <<endl;\
    acc_wrapper << "parameter C_S_AXI_CTRL_BUS_WSTRB_WIDTH = (32 / 8);" <<endl;\
    acc_wrapper << "parameter C_S_AXI_WSTRB_WIDTH = (32 / 8);" <<endl;\
    acc_wrapper << "parameter C_M_AXI_MEM_BUS_WSTRB_WIDTH = (64 / 8);" <<endl;\
    acc_wrapper << "parameter C_M_AXI_WSTRB_WIDTH = (32 / 8);" <<endl;\
    \
    acc_wrapper << "input   s_axi_ctrl_bus_AWVALID;"<<endl;\
    acc_wrapper << "output   s_axi_ctrl_bus_AWREADY;"<<endl;\
    acc_wrapper << "input  [C_S_AXI_CTRL_BUS_ADDR_WIDTH - 1:0] s_axi_ctrl_bus_AWADDR;"<<endl;\
    acc_wrapper << "input   s_axi_ctrl_bus_WVALID;"<<endl;\
    acc_wrapper << "output   s_axi_ctrl_bus_WREADY;"<<endl;\
    acc_wrapper << "input  [C_S_AXI_CTRL_BUS_DATA_WIDTH - 1:0] s_axi_ctrl_bus_WDATA;"<<endl;\
    acc_wrapper << "input  [C_S_AXI_CTRL_BUS_WSTRB_WIDTH - 1:0] s_axi_ctrl_bus_WSTRB;"<<endl;\
    acc_wrapper << "input   s_axi_ctrl_bus_ARVALID;"<<endl;\
    acc_wrapper << "output   s_axi_ctrl_bus_ARREADY;"<<endl;\
    acc_wrapper << "input  [C_S_AXI_CTRL_BUS_ADDR_WIDTH - 1:0] s_axi_ctrl_bus_ARADDR;"<<endl;\
    acc_wrapper << "output   s_axi_ctrl_bus_RVALID;"<<endl;\
    acc_wrapper << "input   s_axi_ctrl_bus_RREADY;"<<endl;\
    acc_wrapper << "output  [C_S_AXI_CTRL_BUS_DATA_WIDTH - 1:0] s_axi_ctrl_bus_RDATA;"<<endl;\
    acc_wrapper << "output  [1:0] s_axi_ctrl_bus_RRESP;"<<endl;\
    acc_wrapper << "output   s_axi_ctrl_bus_BVALID;"<<endl;\
    acc_wrapper << "input   s_axi_ctrl_bus_BREADY;"<<endl;\
    acc_wrapper << "output  [1:0] s_axi_ctrl_bus_BRESP;"<<endl;\
    acc_wrapper << "input   ap_clk;"<<endl;\
    acc_wrapper << "input   ap_rst_n;"<<endl;\
    acc_wrapper << "output   interrupt;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_AWVALID;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_AWREADY;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ADDR_WIDTH - 1:0] m_axi_mem_bus_AWADDR;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ID_WIDTH - 1:0] m_axi_mem_bus_AWID;"<<endl;\
    acc_wrapper << "output  [7:0] m_axi_mem_bus_AWLEN;"<<endl;\
    acc_wrapper << "output  [2:0] m_axi_mem_bus_AWSIZE;"<<endl;\
    acc_wrapper << "output  [1:0] m_axi_mem_bus_AWBURST;"<<endl;\
    acc_wrapper << "output  [1:0] m_axi_mem_bus_AWLOCK;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_AWCACHE;"<<endl;\
    acc_wrapper << "output  [2:0] m_axi_mem_bus_AWPROT;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_AWQOS;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_AWREGION;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_AWUSER_WIDTH - 1:0] m_axi_mem_bus_AWUSER;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_WVALID;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_WREADY;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_DATA_WIDTH - 1:0] m_axi_mem_bus_WDATA;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_WSTRB_WIDTH - 1:0] m_axi_mem_bus_WSTRB;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_WLAST;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ID_WIDTH - 1:0] m_axi_mem_bus_WID;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_WUSER_WIDTH - 1:0] m_axi_mem_bus_WUSER;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_ARVALID;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_ARREADY;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ADDR_WIDTH - 1:0] m_axi_mem_bus_ARADDR;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ID_WIDTH - 1:0] m_axi_mem_bus_ARID;"<<endl;\
    acc_wrapper << "output  [7:0] m_axi_mem_bus_ARLEN;"<<endl;\
    acc_wrapper << "output  [2:0] m_axi_mem_bus_ARSIZE;"<<endl;\
    acc_wrapper << "output  [1:0] m_axi_mem_bus_ARBURST;"<<endl;\
    acc_wrapper << "output  [1:0] m_axi_mem_bus_ARLOCK;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_ARCACHE;"<<endl;\
    acc_wrapper << "output  [2:0] m_axi_mem_bus_ARPROT;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_ARQOS;"<<endl;\
    acc_wrapper << "output  [3:0] m_axi_mem_bus_ARREGION;"<<endl;\
    acc_wrapper << "output  [C_M_AXI_MEM_BUS_ARUSER_WIDTH - 1:0] m_axi_mem_bus_ARUSER;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_RVALID;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_RREADY;"<<endl;\
    acc_wrapper << "input  [C_M_AXI_MEM_BUS_DATA_WIDTH - 1:0] m_axi_mem_bus_RDATA;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_RLAST;"<<endl;\
    acc_wrapper << "input  [C_M_AXI_MEM_BUS_ID_WIDTH - 1:0] m_axi_mem_bus_RID;"<<endl;\
    acc_wrapper << "input  [C_M_AXI_MEM_BUS_RUSER_WIDTH - 1:0] m_axi_mem_bus_RUSER;"<<endl;\
    acc_wrapper << "input  [1:0] m_axi_mem_bus_RRESP;"<<endl;\
    acc_wrapper << "input   m_axi_mem_bus_BVALID;"<<endl;\
    acc_wrapper << "output   m_axi_mem_bus_BREADY;"<<endl;\
    acc_wrapper << "input  [1:0] m_axi_mem_bus_BRESP;"<<endl;\
    acc_wrapper << "input  [C_M_AXI_MEM_BUS_ID_WIDTH - 1:0] m_axi_mem_bus_BID;"<<endl;\
    acc_wrapper << "input  [C_M_AXI_MEM_BUS_BUSER_WIDTH - 1:0] m_axi_mem_bus_BUSER;" <<endl <<endl;\
    \
    acc_wrapper << "\t"<< top<<" top_inst("<<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_AWVALID(s_axi_ctrl_bus_AWVALID)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_AWREADY(s_axi_ctrl_bus_AWREADY)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_AWADDR(s_axi_ctrl_bus_AWADDR)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_WVALID(s_axi_ctrl_bus_WVALID)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_WREADY(s_axi_ctrl_bus_WREADY)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_WDATA(s_axi_ctrl_bus_WDATA)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_WSTRB(s_axi_ctrl_bus_WSTRB)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_ARVALID(s_axi_ctrl_bus_ARVALID)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_ARREADY(s_axi_ctrl_bus_ARREADY)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_ARADDR(s_axi_ctrl_bus_ARADDR)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_RVALID(s_axi_ctrl_bus_RVALID)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_RREADY(s_axi_ctrl_bus_RREADY)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_RDATA(s_axi_ctrl_bus_RDATA)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_RRESP(s_axi_ctrl_bus_RRESP)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_BVALID(s_axi_ctrl_bus_BVALID)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_BREADY(s_axi_ctrl_bus_BREADY)," <<endl;\
    acc_wrapper << "\t .s_axi_ctrl_bus_BRESP(s_axi_ctrl_bus_BRESP)," <<endl;\
    acc_wrapper << "\t .ap_clk(ap_clk)," <<endl;\
    acc_wrapper << "\t .ap_rst_n(ap_rst_n)," <<endl;\
    acc_wrapper << "\t .interrupt(interrupt)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWVALID(m_axi_mem_bus_AWVALID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWREADY(m_axi_mem_bus_AWREADY)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWADDR(m_axi_mem_bus_AWADDR)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWID(m_axi_mem_bus_AWID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWLEN(m_axi_mem_bus_AWLEN)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWSIZE(m_axi_mem_bus_AWSIZE)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWBURST(m_axi_mem_bus_AWBURST)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWLOCK(m_axi_mem_bus_AWLOCK)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWCACHE(m_axi_mem_bus_AWCACHE)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWPROT(m_axi_mem_bus_AWPROT)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWQOS(m_axi_mem_bus_AWQOS)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWREGION(m_axi_mem_bus_AWREGION)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_AWUSER(m_axi_mem_bus_AWUSER)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WVALID(m_axi_mem_bus_WVALID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WREADY(m_axi_mem_bus_WREADY)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WDATA(m_axi_mem_bus_WDATA)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WSTRB(m_axi_mem_bus_WSTRB)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WLAST(m_axi_mem_bus_WLAST)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WID(m_axi_mem_bus_WID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_WUSER(m_axi_mem_bus_WUSER)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARVALID(m_axi_mem_bus_ARVALID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARREADY(m_axi_mem_bus_ARREADY)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARADDR(m_axi_mem_bus_ARADDR)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARID(m_axi_mem_bus_ARID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARLEN(m_axi_mem_bus_ARLEN)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARSIZE(m_axi_mem_bus_ARSIZE)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARBURST(m_axi_mem_bus_ARBURST)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARLOCK(m_axi_mem_bus_ARLOCK)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARCACHE(m_axi_mem_bus_ARCACHE)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARPROT(m_axi_mem_bus_ARPROT)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARQOS(m_axi_mem_bus_ARQOS)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARREGION(m_axi_mem_bus_ARREGION)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_ARUSER(m_axi_mem_bus_ARUSER)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RVALID(m_axi_mem_bus_RVALID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RREADY(m_axi_mem_bus_RREADY)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RDATA(m_axi_mem_bus_RDATA)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RLAST(m_axi_mem_bus_RLAST)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RID(m_axi_mem_bus_RID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RUSER(m_axi_mem_bus_RUSER)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_RRESP(m_axi_mem_bus_RRESP)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_BVALID(m_axi_mem_bus_BVALID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_BREADY(m_axi_mem_bus_BREADY)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_BRESP(m_axi_mem_bus_BRESP)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_BID(m_axi_mem_bus_BID)," <<endl;\
    acc_wrapper << "\t .m_axi_mem_bus_BUSER(m_axi_mem_bus_BID));" <<endl;\
    acc_wrapper << "endmodule " <<endl <<endl;\
}
