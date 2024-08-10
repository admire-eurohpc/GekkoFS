# Quantum Espresso - GekkoFS benchmarks

## How to run
These scripts are an example how to run Nek5000 on Mogon-NHR and HPC4AI clusters using GekkoFS/BeeGFS/Lustre.

Note, all paths need to be adjusted to your environment.

### Build GekkoFS
Build and install GekkoFS following the official instructions [here](https://storage.bsc.es/gitlab/hpc/gekkofs)

### Build Nek5000
1. Download Nek5000 from the official website. See quickstart [here](https://nek5000.github.io/NekDoc/quickstart.html)
2. Download the workload `turbPipe` [here](https://seafile.rlp.net/d/7f8364ad931e4bfebfee/)

### Prepare workload
- Navigate to the `compile/` folder. Add executable rights to `./compile_script` and edit it. Change the
  `NEK_SOURCE_ROOT` to the path where you extracted the Nek5000 release.
- Run `./compile_script --all` and build everything
- Copy/symlink the `./nek5000` binary in `compile/` to `run/`. This is the executable you run later with MPI.
- Copy the `input/` directory from nek5000_input.tar to the `run/` directory.
- Go into the `run/` folder.
- `SESSION.NAME`. This file defines the arguments to Nek5000. From my example: the first line is the model and the
  second line the input directory path. There is no way to pass this explicitly to Nek. `SESSION.NAME` must be available
  in the $PWD when launching `./nek5000`.
- Workload configuration. In the `input/` directory is a `turbPipe.par` file. Here you can set various settings. For
  instance `numSteps` defines how many steps should be run, e.g., 10 steps would only need few seconds. `writeInterval`
  defines how often an output file is written, i.e., in this case every 5 steps all processes write into one output
  file. You can also say to not use the checkpoint files. You should be able to run this workloads with 16 processes.
  Therefore, we can accurately define the runtime and I/O usage of this workload.
- Now, you can simply run `./nek5000` with mpirun. (double check you are in the correct directory with SESSION.NAME).


### Run Nek5000

Presets for all file systems are in the `run_{lustre,gekkofs,beegfs}.sh` scripts on the Mogon and HPC4AI clusters. To
run, modify all scripts and paths accordingly. `sched_{lustre,gkfs,beegfs}.sh` are the corresponding top level scheduler
scripts and is your entry point.