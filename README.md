# linux_modbus
libmodbus linux demo, registers are per "KOSTAL Interface MODBUS (TCP) &amp; SunSpec PIKO IQ / PLENTICORE plus"  

# install libmodbus
`sudo apt-get install libmodbus-dev`  

# compile

`gcc -o client client_mock.c -lmodbus`  
`gcc -o server kostal_mock.c -lmodbus`  

in one terminal run
`sudo ./server`  

in other terminal
`./client`  

client will read registers from server and print them:  
Read registers from mock (iteration 0):  
Battery consumption (0x6A): 50.00  
Grid power (0x6C): 100.00  
PV power (0x74): 800.00  
Battery temperature (0xD6): 60.00  

