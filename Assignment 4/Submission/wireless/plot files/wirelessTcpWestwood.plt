set terminal png
set output "wirelessTcpWestwood.png"
set title "Throughput plot for TCP Westwood"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP Westwood" with linespoints
40 71.0097
44 75.2756
48 82.0865
52 88.8975
60 102.279
552 206.471
576 206.277
628 205.975
1420 205.792
1500 205.563
e
