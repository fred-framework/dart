import os
from os import listdir
from os.path import isfile, join

#slots = [f for f in listdir("hdl") if isfile(join("hdl", f))]
#slots.sort()
files = os.listdir("cores")
for name in files:
	file = open("project/" + name + ".prj", "w")
	mypath = "cores/" + name + "/hdl/vhdl"
	onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]
	for f in onlyfiles:
		file.write("vhdl xil_defaultlib ./Sources/cores/" + name + "/hdl/vhdl/" + f +"\n")
print("just finished copying load prj")
#	a = input(name+" "+ str(slots))
#	file.write("vhdl xil_defaultlib ./Sources/hdl/" + slots[int(a)])
