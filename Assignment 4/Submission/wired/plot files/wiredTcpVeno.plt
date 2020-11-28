set terminal png
set output "wiredTcpVeno.png"
set title "Throughput plot for TCP Veno"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Veno" with linespoints
40 685.326
44 672.964
48 645.949
52 623.031
60 586.382
552 777.779
576 825.609
628 996.375
1420 3983.4
1500 4107.1
e
