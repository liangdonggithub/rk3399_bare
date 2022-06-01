# rk3399_bare
rk3399 bare metal debug and development environment
# software
aarch64-linux-gnu, openocd, rkbin
# hardware
firefly-rk3399 eval board, jlink v11, usb type-c cable.
# how to use
## 1.compile your own miniloader
make loader<br>
and you can get a new binary file which was named as rk3399_loader.bin<br>
## 2.burn loader to emmc
Connect type-c cable between pc and eval board<br>
Booting eval board with maskrom mode.<br>
Run ./burn.sh to burn loader to emmc.<br>
Loader is a routine with swj port init and dead loop. Swd endpoint can be recognized by jlink when you repower eval board.
## 3.load image by jlink-v11
Compile your own project. In this project you can samply use make to do it.<br>
Then you can get a binary file named rk3399_bare.bin .<br>
Check you jlink device connection state.<br>
Open a terminal to run 'sudo ./openocd.sh' to control jlink device.<br>
Open another terminal to run './telnet.sh' to open telnet client.<br>
You can send command from telnet dialog such as 'halt','load_image','step' and so on.<br>
In this case. 'halt' to stop core. 'load_image rk3399_bare.bin 0x0' to write code to memory 0. 'step 0' to set pc to 0x0.<br>
## 4.debug
Run './dbg.sh' to call gdb-multiarch<br>
Note that openocd script file gdb port maybe not equal with this case(.gdbinit target remote configuration). You need change it for your own environment.
## enjoy it!
