make clean
make all

if [ -f ictester.hex ]
 then
   JLinkExe -device lpc1769 -if jtag -speed 4000 -jtagconf -1,-1 -autoconnect 1 flash.jlink
 fi


