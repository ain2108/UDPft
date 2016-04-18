# UDPft
File transfer using UDP

Anton Nefedenkov
ain2108




Notes:

MSS if defined as a macro. MSS must be divisible by two since the checksum calculation
code is sensible to this.

The size of the file that can be sent is limited by the sizeof(int) ~ 2 Gbytes
This is due to te sequencing of bytes. 
