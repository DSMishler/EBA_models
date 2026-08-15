# EBA Models

## Daniel Mishler


A collection of models run on NERSC that showcase the capabilities of EBA,
complete with visualization. Also for Daniel's own purposes of understanding
the architecture better.

EBA stands for "Exposed Buffer Architecture"


2024-09-16 began development of PyEBA2

2025-04-09 began development of PyEBA3

Major changes from PyEBA2
- Processes are axed. EBA is more primitive and should not have a linux-like
  processor with a scheduler, etc.
- The key-tag system is going to be done away with. We may have to
  revisit this for security
- In order to accentuate the EBA at play, separate nodes will no longer
  share a Python environment.
- Visualization will include timestamps to allow for more granular control
