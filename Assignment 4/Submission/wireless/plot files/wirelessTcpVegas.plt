set terminal png
set output "wirelessTcpVegas.png"
set title "Throughput plot for TCP Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Vegas" with linespoints
40 71.0097
44 75.229
48 82.037
52 85.6787
60 98.5668
552 165.342
576 165.196
628 160.191
1420 180.327
1500 180.115
e
