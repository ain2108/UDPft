## UDPft
File transfer using UDP

Anton Nefedenkov
ain2108

Program compiles without errors or warnings on clic machines.

## Run example:
./receiver result.txt 20000 athens.clic.cs.columbia.edu 22000 log_receiver.txt
./sender file.txt 128.59.15.39 21000 22000 log_sender.txt 20
./newudpl -o 128.59.15.35:20000 -i 128.59.15.40/* -p 21000:23000 -L 10 -B 100 -O 20

## Runtime notes:
Do not use window_size > 2000, see the description below.
File size < 2 Gb.

## Necessary files:
Makefile
README.txt
file.txt
hostip.sh
includes/UDPsocket.h
includes/controller.h
includes/helpers.h
includes/input.h
includes/logger.h
includes/packet.h
result.txt
src/UDPsocket.c
src/controller.c
src/helpers.c
src/input.c
src/logger.c
src/packet.c
src/receiver.c
src/sender.c

## Description:
IPv6 not supported (ran out of time).
The program is implemented using pthreads. I wanted to learn mutlithreading, 
so I did build my implementation on threads. Becasue of this, please do not use windowsize > 200.
Due to my incorrect implementation of threads, every segment is sent by its own thread. If 
window size is too large, the program will run our of resources. The idea was to implement thread 
pool, but i sadly ran out of time.

In my tests, the program ran succesfully. It supports out of order packets, bit errors, duplicates
and lost packets. For tests, I have used Edgar Alan Poe's poem The Raven.

The program makes use of a bash script. In general this program was not designed with
portability in mind.  

##Notes:
MSS if defined as a macro. MSS must be divisible by two since the checksum calculation
code is sensible to this.
The size of the file that can be sent is limited by the sizeof(int) ~ 2 Gbytes
This is due to te sequencing of bytes. 
