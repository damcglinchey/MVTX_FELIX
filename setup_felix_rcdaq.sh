#!/bin/bash
pkill -9 rcdaq_server
rcdaq_server > log 2>&1 &
sleep 2
rcdaq_client load ./librcdaqplugin_felix.so

rcdaq_client create_device device_deadtime 1 1005 1000 0 1
rcdaq_client create_device device_felix 1 2000 0 1024 0 4

rcdaq_client daq_list_readlist

rm rcdaq*.evt
rcdaq_client daq_open
rcdaq_client daq_begin
sleep 5
rcdaq_client daq_end
