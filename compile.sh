g++ mt19937ar.c ip.cpp show.cpp -o show.cgi -O2 -mcpu=i686 -L/usr/local/lib -L/usr/local/psa/mysql/lib/mysql -I/usr/local/include -I/usr/local/psa/mysql/include/mysql -lfcgi -lGeoIP -lmysqlclient
g++ ip.cpp go.cpp -o go.cgi -O2 -mcpu=i686 -L/usr/local/lib -L/usr/local/psa/mysql/lib/mysql -I/usr/local/include -I/usr/local/psa/mysql/include/mysql -lfcgi -lmysqlclient
g++ mtop-stat.cpp ip.cpp -o mtop-stat -O2 -mcpu=i686 -L/usr/local/lib -L/usr/local/psa/mysql/lib/mysql -I/usr/local/include -I/usr/local/psa/mysql/include/mysql -lmysqlclient
g++ echo.c -o echo.cgi -O2 -mcpu=i686 -L/usr/local/lib -I/usr/local/include -lfcgi
chown mtop:mtop show.cgi
chown mtop:mtop go.cgi
chown mtop:mtop echo.cgi
chown mtop:mtop mtop-stat
rm -f /home/mtop/pub/show.cgi
rm -f /home/mtop/pub/go.cgi
rm -f /home/mtop/pub/echo.cgi
rm -f /home/mtop/sbin/mtop-stat
mv show.cgi /home/mtop/pub/
mv go.cgi /home/mtop/pub/
mv echo.cgi /home/mtop/pub/
mv mtop-stat /home/mtop/sbin/