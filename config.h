#ifndef CONFIG_H
#define CONFIG_H

//#define DEBUG

extern void init_genrand(unsigned long);
extern unsigned long genrand_int32(void);
extern unsigned int ipaton (const char*);
extern void ipntoa (const unsigned int, char*);

#define CODE_IFRAME		1
#define CODE_TEXT		2
#define CODE_WAP		3
#define CODE_TEXT_PHP		4

#define MTOP_GEOIP_FILE		"/home/mtop/geoip/city.dat"
#define MTOP_FIFO_FILE		"/home/mtop/tmp/mtop-stat.fifo"
#define MTOP_PID_FILE		"/home/mtop/tmp/mtop-stat.pid"

#define MTOP_MYSQL_DB		"mtop"		// База данных
#define MTOP_MYSQL_USER		"mtop"		// Пользователь
#define MTOP_MYSQL_PASSWD	"*****"
#define MTOP_MYSQL_RO_USER	"mtop"    // Пользователь с правами read-only
#define MTOP_MYSQL_RO_PASSWD	"*****"

#define MTOP_DATA_TIMEOUT	300				// время, через которое нужно перезагрузить данные
#define MTOP_STAT_DATA_TIMEOUT	120				// время, через которое нужно перезагрузить данные
                                // в программе mtop-stat
#define MTOP_STACK_SIZE		500000	// размер стека статистики

#define MTOP_ADMIN_USER_ID	4	// Id администратора, на балансы которого будет
                                // сбрасываться комиссия

const int mtop_signature_show	= 1;	// Признак записи показа
const int mtop_signature_click  = 2;	// Признак записи показа
const char* mtop_stat_user = 		"mtop";		// Пользователь, от имени которого выполняется mtop-stat

#define MTOP_NET_OWNSITES_NO	0
#define MTOP_NET_OWNSITES_CHECK	1
#define MTOP_NET_OWNSITES_YES	2

#define MTOP_MAX_BANNER_NUM	10				// Максимальное число баннеров, выводимых за раз
#define MTOP_SERVER_IP		"81.176.64.50"

#endif
