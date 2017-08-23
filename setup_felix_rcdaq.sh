#!/bin/sh

D=`dirname "$0"`
B=`basename "$0"`
MYSELF="`cd \"$D\" 2>/dev/null && pwd || echo \"$D\"`/$B"

rcdaq_client daq_clear_readlist
rcdaq_client load librcdaqplugin_felix.so


rcdaq_client create_device device_file 9 900 "$MYSELF"
rcdaq_client create_device device_deadtime 1 1005 1000 0 1
rcdaq_client create_device device_felix 1 2000 0 1024 0 1000
