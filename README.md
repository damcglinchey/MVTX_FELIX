# to build
On the FELIX server, source the setup scripts for the FELIX software and the rcdaq software:
```
source /home/maps/meeg/felix/setup.sh
source /home/maps/meeg/rcdaq/setup.sh
```
If on your own system: felix/setup.sh is what you make when following the software instructions in felix_firmware, rcdaq/setup.sh should look like the following:
```
export ONLINE_MAIN=/home/maps/meeg/rcdaq/install
export RCDAQ_ROOT=/home/maps/meeg/rcdaq
export PATH=$PATH:$ONLINE_MAIN/bin
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$ONLINE_MAIN/lib"
source $ONLINE_MAIN/bin/aliases.sh
```

In this directory:
```
mkdir build
cd build
cmake ..
make
```

# to run
Must be root.
Source the two `setup.sh` files as above.
In the `build` directory:
```
../setup_felix_rcdaq.sh

```

Various things to look at data:
```
dlist -i rcdaq-00000001-0000.evt -n 10
ddump -i rcdaq-00000001-0000.evt -e 2
tweak -w 4 rcdaq-00000001-0000.evt
ddump -i rcdaq-00000001-0000.evt -n 30|grep -v 6000 |grep -v "         0        0        0        0"|less
```

# todo
figure out padding in `daq_device_felix.cc` line 111-114