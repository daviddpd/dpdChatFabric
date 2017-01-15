#!/bin/sh
bin/createConfig -c device.conf -w device.conf --genkeys --uuid0=00000000-0000-0000-0000-000000000000 --uuid1=d3bd5042-a073-11e5-b5a8-00a0988afcc9 --debug
bin/createConfig -c controller.conf -w controller.conf --genkeys --genuuid1
