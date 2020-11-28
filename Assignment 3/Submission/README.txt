1. To create executable files, there are two methods
   (a) using Makefile
       run "make all" to create the executables and "make clean" to remove any previously made executables
   (b) using terminal
       gcc -Wall server.c -o server
       gcc -Wall client.c -o client

2. To run the executables,
   ./server <port_number>
   ./client <ip_address> <port_number>

3. Functionalities and Usage is explained in "Assignment 3 Report.pdf"

4. Username Password pairs are stored in "traders_auth.txt"
