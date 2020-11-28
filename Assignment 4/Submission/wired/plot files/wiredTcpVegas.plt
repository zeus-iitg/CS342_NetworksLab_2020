set terminal png
set output "wiredTcpVegas.png"
set title "Throughput plot for TCP Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Vegas" with linespoints
40 69.7113
44 72.7384
48 75.7654
52 78.7922
60 84.8454
552 360.884
576 378.777
628 408.787
1420 961.387
1500 1020.84
e
