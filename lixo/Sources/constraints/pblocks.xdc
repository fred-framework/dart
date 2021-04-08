# User Generated miscellaneous constraints


set_property HD.RECONFIGURABLE true [get_cells system_i/slot_p0_s0]
create_pblock pblock_slot_0
add_cells_to_pblock [get_pblocks pblock_slot_0] [get_cells -quiet [list system_i/slot_p0_s0]]
resize_pblock [get_pblocks pblock_slot_0] -add {SLICE_X98Y0:SLICE_X107Y99}
resize_pblock [get_pblocks pblock_slot_0] -add {RAMB18_X5Y0:RAMB18_X5Y39}
resize_pblock [get_pblocks pblock_slot_0] -add {RAMB36_X5Y0:RAMB36_X5Y19}
resize_pblock [get_pblocks pblock_slot_0] -add {DSP48_X4Y0:DSP48_X4Y39}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_slot_0]
set_property SNAPPING_MODE ON [get_pblocks pblock_slot_0]


set_property HD.RECONFIGURABLE true [get_cells system_i/slot_p0_s1]
create_pblock pblock_slot_1
add_cells_to_pblock [get_pblocks pblock_slot_1] [get_cells -quiet [list system_i/slot_p0_s1]]
resize_pblock [get_pblocks pblock_slot_1] -add {SLICE_X24Y0:SLICE_X95Y149}
resize_pblock [get_pblocks pblock_slot_1] -add {RAMB18_X2Y0:RAMB18_X4Y59}
resize_pblock [get_pblocks pblock_slot_1] -add {RAMB36_X2Y0:RAMB36_X4Y29}
resize_pblock [get_pblocks pblock_slot_1] -add {DSP48_X2Y0:DSP48_X3Y59}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_slot_1]
set_property SNAPPING_MODE ON [get_pblocks pblock_slot_1]


set_property SEVERITY {Warning} [get_drc_checks NSTD-1]
set_property SEVERITY {Warning} [get_drc_checks UCIO-1]
