
#!/bin/bash

rm -rf argobots/ cereal/ json-c/ libfabric/ mercury/ mochi-margo/ mochi-thallium/ yaml-cpp/

# Download Cereal v1.3.2
git clone https://github.com/USCiLab/cereal.git


# Download YAML-CPP yaml-cpp-0.7.0
git clone https://github.com/jbeder/yaml-cpp.git

# Download Margo v0.9.8+
git clone https://github.com/mochi-hpc/mochi-margo.git

# Download Argobots v1.1+
git clone https://github.com/pmodels/argobots.git

# Download Mercury v2.1.0+
git clone https://github.com/mercury-hpc/mercury.git
cd mercury
git submodule update --init --recursive
cd ..


# Download Thallium v0.10.1+
git clone https://github.com/mochi-hpc/mochi-thallium.git

# Download libfabric v0.10.1+
git clone https://github.com/ofiwg/libfabric.git

#Download json-c 
git clone https://github.com/json-c/json-c.git
