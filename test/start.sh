./compile.sh
./stop.sh
mv show.cgi ../pub/
mv go.cgi ../pub/go.html
sleep 1
lighttpd -f /usr/local/etc/lighttpd.conf