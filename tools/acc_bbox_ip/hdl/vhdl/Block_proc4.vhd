-- ==============================================================
-- RTL generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and OpenCL
-- Version: 2019.2
-- Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
-- 
-- ===========================================================

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity Block_proc4 is
port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_continue : IN STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    args_address0 : OUT STD_LOGIC_VECTOR (2 downto 0);
    args_ce0 : OUT STD_LOGIC;
    args_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    mem_in_V : IN STD_LOGIC_VECTOR (31 downto 0);
    mem_out_V : IN STD_LOGIC_VECTOR (31 downto 0);
    mem_in_V_out_din : OUT STD_LOGIC_VECTOR (31 downto 0);
    mem_in_V_out_full_n : IN STD_LOGIC;
    mem_in_V_out_write : OUT STD_LOGIC;
    mem_out_V_out_din : OUT STD_LOGIC_VECTOR (31 downto 0);
    mem_out_V_out_full_n : IN STD_LOGIC;
    mem_out_V_out_write : OUT STD_LOGIC;
    ap_return_0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    ap_return_1 : OUT STD_LOGIC_VECTOR (31 downto 0);
    ap_return_2 : OUT STD_LOGIC_VECTOR (31 downto 0) );
end;


architecture behav of Block_proc4 is 
    constant ap_const_logic_1 : STD_LOGIC := '1';
    constant ap_const_logic_0 : STD_LOGIC := '0';
    constant ap_ST_fsm_state1 : STD_LOGIC_VECTOR (3 downto 0) := "0001";
    constant ap_ST_fsm_state2 : STD_LOGIC_VECTOR (3 downto 0) := "0010";
    constant ap_ST_fsm_state3 : STD_LOGIC_VECTOR (3 downto 0) := "0100";
    constant ap_ST_fsm_state4 : STD_LOGIC_VECTOR (3 downto 0) := "1000";
    constant ap_const_lv32_0 : STD_LOGIC_VECTOR (31 downto 0) := "00000000000000000000000000000000";
    constant ap_const_lv32_1 : STD_LOGIC_VECTOR (31 downto 0) := "00000000000000000000000000000001";
    constant ap_const_lv32_2 : STD_LOGIC_VECTOR (31 downto 0) := "00000000000000000000000000000010";
    constant ap_const_lv64_0 : STD_LOGIC_VECTOR (63 downto 0) := "0000000000000000000000000000000000000000000000000000000000000000";
    constant ap_const_lv64_1 : STD_LOGIC_VECTOR (63 downto 0) := "0000000000000000000000000000000000000000000000000000000000000001";
    constant ap_const_lv64_2 : STD_LOGIC_VECTOR (63 downto 0) := "0000000000000000000000000000000000000000000000000000000000000010";
    constant ap_const_lv32_3 : STD_LOGIC_VECTOR (31 downto 0) := "00000000000000000000000000000011";
    constant ap_const_boolean_1 : BOOLEAN := true;

    signal ap_done_reg : STD_LOGIC := '0';
    signal ap_CS_fsm : STD_LOGIC_VECTOR (3 downto 0) := "0001";
    attribute fsm_encoding : string;
    attribute fsm_encoding of ap_CS_fsm : signal is "none";
    signal ap_CS_fsm_state1 : STD_LOGIC;
    attribute fsm_encoding of ap_CS_fsm_state1 : signal is "none";
    signal mem_in_V_out_blk_n : STD_LOGIC;
    signal mem_out_V_out_blk_n : STD_LOGIC;
    signal ap_block_state1 : BOOLEAN;
    signal args_load_reg_123 : STD_LOGIC_VECTOR (31 downto 0);
    signal ap_CS_fsm_state2 : STD_LOGIC;
    attribute fsm_encoding of ap_CS_fsm_state2 : signal is "none";
    signal args_load_1_reg_133 : STD_LOGIC_VECTOR (31 downto 0);
    signal ap_CS_fsm_state3 : STD_LOGIC;
    attribute fsm_encoding of ap_CS_fsm_state3 : signal is "none";
    signal ap_CS_fsm_state4 : STD_LOGIC;
    attribute fsm_encoding of ap_CS_fsm_state4 : signal is "none";
    signal ap_NS_fsm : STD_LOGIC_VECTOR (3 downto 0);


begin




    ap_CS_fsm_assign_proc : process(ap_clk)
    begin
        if (ap_clk'event and ap_clk =  '1') then
            if (ap_rst = '1') then
                ap_CS_fsm <= ap_ST_fsm_state1;
            else
                ap_CS_fsm <= ap_NS_fsm;
            end if;
        end if;
    end process;


    ap_done_reg_assign_proc : process(ap_clk)
    begin
        if (ap_clk'event and ap_clk =  '1') then
            if (ap_rst = '1') then
                ap_done_reg <= ap_const_logic_0;
            else
                if ((ap_continue = ap_const_logic_1)) then 
                    ap_done_reg <= ap_const_logic_0;
                elsif ((ap_const_logic_1 = ap_CS_fsm_state4)) then 
                    ap_done_reg <= ap_const_logic_1;
                end if; 
            end if;
        end if;
    end process;

    process (ap_clk)
    begin
        if (ap_clk'event and ap_clk = '1') then
            if ((ap_const_logic_1 = ap_CS_fsm_state3)) then
                args_load_1_reg_133 <= args_q0;
            end if;
        end if;
    end process;
    process (ap_clk)
    begin
        if (ap_clk'event and ap_clk = '1') then
            if ((ap_const_logic_1 = ap_CS_fsm_state2)) then
                args_load_reg_123 <= args_q0;
            end if;
        end if;
    end process;

    ap_NS_fsm_assign_proc : process (ap_start, ap_done_reg, ap_CS_fsm, ap_CS_fsm_state1, mem_in_V_out_full_n, mem_out_V_out_full_n)
    begin
        case ap_CS_fsm is
            when ap_ST_fsm_state1 => 
                if ((not(((ap_start = ap_const_logic_0) or (mem_out_V_out_full_n = ap_const_logic_0) or (mem_in_V_out_full_n = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1))) then
                    ap_NS_fsm <= ap_ST_fsm_state2;
                else
                    ap_NS_fsm <= ap_ST_fsm_state1;
                end if;
            when ap_ST_fsm_state2 => 
                ap_NS_fsm <= ap_ST_fsm_state3;
            when ap_ST_fsm_state3 => 
                ap_NS_fsm <= ap_ST_fsm_state4;
            when ap_ST_fsm_state4 => 
                ap_NS_fsm <= ap_ST_fsm_state1;
            when others =>  
                ap_NS_fsm <= "XXXX";
        end case;
    end process;
    ap_CS_fsm_state1 <= ap_CS_fsm(0);
    ap_CS_fsm_state2 <= ap_CS_fsm(1);
    ap_CS_fsm_state3 <= ap_CS_fsm(2);
    ap_CS_fsm_state4 <= ap_CS_fsm(3);

    ap_block_state1_assign_proc : process(ap_start, ap_done_reg, mem_in_V_out_full_n, mem_out_V_out_full_n)
    begin
                ap_block_state1 <= ((ap_start = ap_const_logic_0) or (mem_out_V_out_full_n = ap_const_logic_0) or (mem_in_V_out_full_n = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1));
    end process;


    ap_done_assign_proc : process(ap_done_reg, ap_CS_fsm_state4)
    begin
        if ((ap_const_logic_1 = ap_CS_fsm_state4)) then 
            ap_done <= ap_const_logic_1;
        else 
            ap_done <= ap_done_reg;
        end if; 
    end process;


    ap_idle_assign_proc : process(ap_start, ap_CS_fsm_state1)
    begin
        if (((ap_start = ap_const_logic_0) and (ap_const_logic_1 = ap_CS_fsm_state1))) then 
            ap_idle <= ap_const_logic_1;
        else 
            ap_idle <= ap_const_logic_0;
        end if; 
    end process;


    ap_ready_assign_proc : process(ap_CS_fsm_state4)
    begin
        if ((ap_const_logic_1 = ap_CS_fsm_state4)) then 
            ap_ready <= ap_const_logic_1;
        else 
            ap_ready <= ap_const_logic_0;
        end if; 
    end process;

    ap_return_0 <= args_load_reg_123;
    ap_return_1 <= args_load_1_reg_133;
    ap_return_2 <= args_q0;

    args_address0_assign_proc : process(ap_CS_fsm_state1, ap_CS_fsm_state2, ap_CS_fsm_state3)
    begin
        if ((ap_const_logic_1 = ap_CS_fsm_state3)) then 
            args_address0 <= ap_const_lv64_2(3 - 1 downto 0);
        elsif ((ap_const_logic_1 = ap_CS_fsm_state2)) then 
            args_address0 <= ap_const_lv64_1(3 - 1 downto 0);
        elsif ((ap_const_logic_1 = ap_CS_fsm_state1)) then 
            args_address0 <= ap_const_lv64_0(3 - 1 downto 0);
        else 
            args_address0 <= "XXX";
        end if; 
    end process;


    args_ce0_assign_proc : process(ap_start, ap_done_reg, ap_CS_fsm_state1, mem_in_V_out_full_n, mem_out_V_out_full_n, ap_CS_fsm_state2, ap_CS_fsm_state3)
    begin
        if (((ap_const_logic_1 = ap_CS_fsm_state3) or (ap_const_logic_1 = ap_CS_fsm_state2) or (not(((ap_start = ap_const_logic_0) or (mem_out_V_out_full_n = ap_const_logic_0) or (mem_in_V_out_full_n = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1)))) then 
            args_ce0 <= ap_const_logic_1;
        else 
            args_ce0 <= ap_const_logic_0;
        end if; 
    end process;


    mem_in_V_out_blk_n_assign_proc : process(ap_start, ap_done_reg, ap_CS_fsm_state1, mem_in_V_out_full_n)
    begin
        if ((not(((ap_start = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1))) then 
            mem_in_V_out_blk_n <= mem_in_V_out_full_n;
        else 
            mem_in_V_out_blk_n <= ap_const_logic_1;
        end if; 
    end process;

    mem_in_V_out_din <= mem_in_V;

    mem_in_V_out_write_assign_proc : process(ap_start, ap_done_reg, ap_CS_fsm_state1, mem_in_V_out_full_n, mem_out_V_out_full_n)
    begin
        if ((not(((ap_start = ap_const_logic_0) or (mem_out_V_out_full_n = ap_const_logic_0) or (mem_in_V_out_full_n = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1))) then 
            mem_in_V_out_write <= ap_const_logic_1;
        else 
            mem_in_V_out_write <= ap_const_logic_0;
        end if; 
    end process;


    mem_out_V_out_blk_n_assign_proc : process(ap_start, ap_done_reg, ap_CS_fsm_state1, mem_out_V_out_full_n)
    begin
        if ((not(((ap_start = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1))) then 
            mem_out_V_out_blk_n <= mem_out_V_out_full_n;
        else 
            mem_out_V_out_blk_n <= ap_const_logic_1;
        end if; 
    end process;

    mem_out_V_out_din <= mem_out_V;

    mem_out_V_out_write_assign_proc : process(ap_start, ap_done_reg, ap_CS_fsm_state1, mem_in_V_out_full_n, mem_out_V_out_full_n)
    begin
        if ((not(((ap_start = ap_const_logic_0) or (mem_out_V_out_full_n = ap_const_logic_0) or (mem_in_V_out_full_n = ap_const_logic_0) or (ap_done_reg = ap_const_logic_1))) and (ap_const_logic_1 = ap_CS_fsm_state1))) then 
            mem_out_V_out_write <= ap_const_logic_1;
        else 
            mem_out_V_out_write <= ap_const_logic_0;
        end if; 
    end process;

end behav;
