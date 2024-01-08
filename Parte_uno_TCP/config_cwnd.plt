set terminal png
set output "dumbell-tp2-cwnd.png"
set xlabel "Time(seconds)"
set ylabel "cwnd"
set title "Congestion Window"
plot "tcp-2-0-cwnd.data" using 1:2 title "cwnd" with lines,"tcp-2-0-ssth.data" using 1:2 title "ssthresh" with lines



