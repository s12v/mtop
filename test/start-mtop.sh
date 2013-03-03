kill -9 `cat /home/mtop/tmp/mtop-stat.pid`
sleep 1
rm -f ../tmp/mtop-stat.pid
rm -f ../tmp/mtop-stat.fifo
rm -f ../sbin/mtop-stat
rm -f ../sbin/mtop-stat.core
mv mtop-stat ../sbin/
/home/mtop/sbin/mtop-stat &