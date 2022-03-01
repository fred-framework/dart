# DART

DART is a tool that fully automates the FPGA design flow for a real-time, dynamic partially reconfigurable (DPR) co-designed system that comprises both software and hardware components. DART fully automates the partitioning, floorplanning, and implementation (routing and bitstream generation) phases of the DPR flow. Besides, DART supports the Zynq 7-series and Ultrascale+ FPGA-based SoCs by Xilinx. 

DART must be used with [FRED](https://fred-framework-docs.readthedocs.io/en/latest/docs/03_runtime/index.html) runtime for a complete real-time FPGA offloading solution. While DART automates the hardware design,
FRED manages the hw/sw interface, easing the Linux-based application design. 

## Table of Contents

[DART Introduction](https://fred-framework-docs.readthedocs.io/en/latest/docs/02_dart/index.html)

[DART Installation](./docs/install.md)

[Running DART](./docs/running.md)


## Authors

- Biruk Belay Seyoum (June 2021), ReTiS Lab, Scuola Sant'Anna, Pisa, Italy. Main developer.
- Alexandre Amory (June 2021), ReTiS Lab, Scuola Sant'Anna, Pisa, Italy. Tester, documentation, DART IPs, and minor contributions to DART source code.


## Papers

Please cite this paper if you are using DART:

```bibtex
@inproceedings{seyoum2021automating,
  title={Automating the design flow under dynamic partial reconfiguration for hardware-software co-design in {FPGA SoC}},
  author={Seyoum, Biruk and Pagani, Marco and Biondi, Alessandro and Buttazzo, Giorgio},
  booktitle={Proceedings of the ACM Symposium on Applied Computing (SAC)},
  pages={481--490},
  year={2021}
}
```

## References

- [Dynamic Function eXchange - UG909 (v2020.2)](https://www.xilinx.com/support/documentation/sw_manuals/xilinx2020_2/ug909-vivado-partial-reconfiguration.pdf)
- [XAPP1231 - Partial Reconfiguration of a Hardware Accelerator with Vivado Design Suite](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841851/XAPP1231+-+Partial+Reconfiguration+of+a+Hardware+Accelerator+with+Vivado+Design+Suite)

## Acknoledgments

This tool was partially developed in the context of the [AMPERE project](https://ampere-euproject.eu/). This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 871669.

