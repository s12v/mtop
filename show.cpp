/*
 *  Показ баннера
 *
 *  Входящие переменные: 
 *
 *    QUERY_STRING = "1;12;1088;rnd
 *                    ^ ^  ^        
 *                    ^ ^   ID аккаунта
 *                    ^ ID сайта
 *                    Тип кода
 *
 *  либо:
 *
 *    QUERY_STRING = "4;12;1088;2;5612372
 *                    ^ ^  ^    ^ ^   
 *                    ^ ^  ^    ^ IP-адрес (int)   
 *                    ^ ^  ^    Кол-баннеров для вывода    
 *                    ^ ^   ID аккаунта
 *                    ^ ID сайта
 *                    Тип кода
 *
 *  Используются библиотеки: MySQL, FastCGI, GeoIP
 *
 *  Строка для компиляции:
 *
 *  g++ show.cpp mt19937ar.c -o show.cgi -I/usr/include/mysql -I/usr/local/ -lfcgi -lGeoIP -lmysqlclient
 *
 */

#include <fcgi_stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
//#include <iostream.h>

#include "mysql.h"
#include "GeoIP.h"
#include "GeoIPCity.h"

#include "config.h"
#include "show.h"
#include "traffic.h"

// Окружение
extern char **environ;

// Константы
static const int dayOfWeekBitArr[7] = {64, 1, 2, 4, 8, 16, 32};
static const int timeBitArr[24] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608};

// Статические переменные
static GeoIP 	*gi;
static GeoIPRecord *gir;

static MYSQL 	*connection;
static MYSQL_RES	*result;
static MYSQL_RES	*result2;
static MYSQL_ROW	row;

static SITE			*siteList;
static SITE			*site;
static int			siteNum;

static PROFILE *profileList;
static PROFILE *profile;
static int			profileNum;

static BANNER		*bannerList;
static BANNER		*banner;
static int			bannerNum;

static int *areaCodeArr;
static int areaCodeNumber;

static int TargetingAreaIsSet;
static int TargetingDirectIsSet;
static int TargetingRubricIsSet;
static char *query;

static int i, j, tmp_i;
static unsigned int tmp_ui;
static int globalError;


// Показываем баннер по умолчанию
// networkId - сеть (для определения размера баннера)
// Сети заданы жёстко
int show_default_banner(int networkId = 0, int outputCodeType = 0)
{
  // Показываем баннер-заглушку, либо, если не определена сеть, пробел
  switch(networkId)
  {
  	case 1:
  	{
  	  if(outputCodeType == CODE_TEXT_PHP)
  	  {
				printf("<a href=\"http://www.mtop.ru/\" target=\"_blank\">Баннерная сеть Mtop.ru</a>\r\n");
			}
			else
			{
				printf("document.write('<a href=\"http://www.mtop.ru/\" target=\"_blank\">Баннерная сеть Mtop.ru</a><br>');\n");
			}
  		break;
  	}
  	case 2:
  	{
			printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://www.mtop.ru/net/\" target=\"_blank\"><img src=\"http://www.mtop.ru/i/default/468x60.gif\" width=468 height=60 border=0 alt=\"MTop.ru Banner Network\"></a><br></body></html>\r\n");
  		break;
  	}
  	case 3:
  	{
			printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://www.mtop.ru/net/\" target=\"_blank\"><img src=\"http://www.mtop.ru/i/default/120x240.gif\" width=120 height=240 border=0 alt=\"MTop.ru Banner Network\"></a><br></body></html>\r\n");
  		break;
  	}
  	case 4:
  	{
			printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://www.mtop.ru/net/\" target=\"_blank\"><img src=\"http://www.mtop.ru/i/default/120x60.gif\" width=120 height=60 border=0 alt=\"MTop.ru Banner Network\"></a><br></body></html>\r\n");
  		break;
  	}
  	case 5:
  	{
			printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://www.mtop.ru/net/\" target=\"_blank\"><img src=\"http://www.mtop.ru/i/default/100x100.gif\" width=100 height=100 border=0 alt=\"MTop.ru Banner Network\"></a><br></body></html>\r\n");
  		break;
  	}
  	case 6:
  	{
			printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://www.mtop.ru/net/\" target=\"_blank\"><img src=\"http://www.mtop.ru/i/default/600x90.gif\" width=600 height=90 border=0 alt=\"MTop.ru Banner Network\"></a><br></body></html>\r\n");
  		break;
  	}
  	default:
  	{
			printf("&nbsp;\n");
  		break;
  	}
  }
}

// Запуск запроса с проверкой результата
int mysql_query_check(int &error, MYSQL* connection, const char* query)
{
  int state = mysql_query(connection, query);
  if(state != 0)
  {
		printf("Error in MySQL Query: %s\n", query);
		printf("%d %s", mysql_errno(connection), mysql_error(connection));
  	error = 1;
  	exit(1);
  }
  return 0;
}

// Ошибка на этапе загрузки
int loading_error_exit(int error)
{
	fprintf(stderr, "global error %d\n", error);
	exit(error);
}


// Загрузка данных
static void data_load(void)
{
  #ifdef DEBUG
		struct timeval start_time, end_time;  // необходимо для определения времени работы основного цикла
		gettimeofday(&start_time, NULL);
	#endif

	// Инициализация глобальных переменных
	siteList = new SITE;
	siteNum = 0;

	profileList = new PROFILE;
	profileNum = 0;

	bannerList = new BANNER;
	bannerNum = 0;

	float accountBalance;
	int bannerStatTodayShows;
	int	bannerStatTodayClicks;
	int bannerShowsDaily;
	int bannerClicksDaily;
	//

	// Загрузка сайтов
	mysql_query_check(globalError, connection, "SELECT SiteId, SiteUserId, SiteRubricId, SiteUrl, AccountId, AccountNetworkId, NetworkXSize, NetworkYSize, SiteUrlHash FROM Site, Account, Network WHERE SiteUserId = AccountUserId AND AccountNetworkId = NetworkId");
	result = mysql_store_result(connection);
	siteNum = mysql_num_rows(result);

	site = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(site != NULL)
		{
			site->next = new SITE;
			site = site->next;
		}
		else
		{
			site = siteList;
		}

		site->Id = atoi(row[0]);
		site->UserId = atoi(row[1]);
		site->RubricId = atoi(row[2]);
		strncpy(site->Url, row[3], 159);
		site->AccountId = atoi(row[4]);
		site->NetworkId = atoi(row[5]);
		site->XSize = atoi(row[6]);
		site->YSize = atoi(row[7]);
		strncpy(site->UrlHash, row[8], 32);
	}
	mysql_free_result(result);
	//

	// Загрузка профилей
	mysql_query_check(globalError, connection, "SELECT ProfileId, ProfileUserId, ProfileOwnSites, ProfileDayOfWeek, ProfileTime, ProfileTargetingAreaIsSet, ProfileTargetingDirectIsSet, ProfileTargetingRubricIsSet FROM Profile WHERE ProfileEnabled = 'Y' AND ProfileStartTime < CURRENT_DATE AND ProfileStopTime > CURRENT_DATE AND ProfileHaveBanners = 1");
	result = mysql_store_result(connection);
	profileNum = mysql_num_rows(result);

	profile = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(profile != NULL)
		{
			profile->next = new PROFILE;
			profile = profile->next;
		}
		else
		{
			profile = profileList;
		}

		profile->Id = atoi(row[0]);
  	profile->UserId = (short)atoi(row[1]);
		profile->OwnSites = (short)atoi(row[2]);
		profile->DayOfWeek = atoi(row[3]);
		profile->Time = (short)atoi(row[4]);

		TargetingAreaIsSet = (short)atoi(row[5]);
		TargetingDirectIsSet = (short)atoi(row[6]);
		TargetingRubricIsSet = (short)atoi(row[7]);

		// Загрузка списка регионов/городов
		if(TargetingAreaIsSet)
		{
			sprintf(query, "SELECT TargetingAreaAreaId FROM TargetingArea WHERE TargetingAreaProfileId = '%d'", profile->Id);
			mysql_query_check(globalError, connection, query);
			result2 = mysql_store_result(connection);
			profile->TargetingAreaNumber = mysql_num_rows(result2);
			if(profile->TargetingAreaNumber > 0) profile->TargetingArea = new short[profile->TargetingAreaNumber];

			i = 0;
			while((row = mysql_fetch_row(result2)) != NULL)
			{
			  // AreaId
			  tmp_i = (int)atoi(row[0]);
			  // Добавляем AreaCode в массив
			  profile->TargetingArea[i++] = areaCodeArr[tmp_i];
			}
			mysql_free_result(result2);
		}
		//

		if(TargetingDirectIsSet == 1)
		{
			// Загрузка белого списка профиля
			sprintf(query, "SELECT TargetingDirectSiteId FROM TargetingDirect WHERE TargetingDirectProfileId = '%d' AND TargetingDirectType = 'white'", profile->Id);
			mysql_query_check(globalError, connection, query);
			result2 = mysql_store_result(connection);
			profile->TargetingWhiteNumber = mysql_num_rows(result2);
			profile->TargetingWhite = new int[profile->TargetingWhiteNumber];

			i = 0;
			while((row = mysql_fetch_row(result2)) != NULL)
			{
			  profile->TargetingWhite[i++] = atoi(row[0]);
			}
			mysql_free_result(result2);

			// Загрузка черного списка
			sprintf(query, "SELECT TargetingDirectSiteId FROM TargetingDirect WHERE TargetingDirectProfileId = '%d' AND TargetingDirectType = 'black'", profile->Id);
			mysql_query_check(globalError, connection, query);
			result2 = mysql_store_result(connection);
			profile->TargetingBlackNumber = mysql_num_rows(result2);
			profile->TargetingBlack = new int[profile->TargetingBlackNumber];

			i = 0;
			while((row = mysql_fetch_row(result2)) != NULL)
			{
			  profile->TargetingBlack[i++] = atoi(row[0]);
			}
			mysql_free_result(result2);
		}
		//

		// Загрузка списка рубрик
		if(TargetingRubricIsSet)
		{
			sprintf(query, "SELECT TargetingRubricRubricId FROM TargetingRubric WHERE TargetingRubricProfileId = '%d'", profile->Id);
			mysql_query_check(globalError, connection, query);
			result2 = mysql_store_result(connection);
			profile->TargetingRubricNumber = mysql_num_rows(result2);
			profile->TargetingRubric = new short[profile->TargetingRubricNumber];

			i = 0;
			while((row = mysql_fetch_row(result2)) != NULL)
			{
			  profile->TargetingRubric[i++] = (short)atoi(row[0]);
			}
			mysql_free_result(result2);
		}
		//

	}
	mysql_free_result(result);
	profile = NULL;
	//

	// Загрузка баннеров
	//                                                        0                1                2           3           4             5               6          7               8               9                10                 11                    12                     13
	mysql_query_check(globalError, connection, "SELECT BannerId, BannerProfileId, BannerNetworkId, BannerFile, BannerHref, BannerString, BannerPriority, AccountId, AccountBalance, BannerHrefHash, BannerShowsDaily, BannerClicksDaily, BannerStatTodayShows, BannerStatTodayClicks FROM Banner, BannerStat, Profile, Account WHERE ProfileEnabled = 'Y' AND BannerEnabled = 'Y' AND BannerChecked = 'Y' AND BannerProfileId = ProfileId AND BannerNetworkId = AccountNetworkId AND AccountUserId = ProfileUserId AND BannerStatBannerId = BannerId");
	result = mysql_store_result(connection);
	bannerNum = mysql_num_rows(result);

	banner = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		accountBalance = atof(row[8]);
		bannerShowsDaily = atoi(row[10]);
		bannerClicksDaily = atoi(row[11]);
		bannerStatTodayShows = atoi(row[12]);
		bannerStatTodayClicks = atoi(row[13]);

		// Проверка - может ли этот баннер быть показан
		if( (accountBalance >= 0) && (bannerStatTodayShows < bannerShowsDaily) &&
		    ((bannerStatTodayClicks < bannerClicksDaily) || (bannerClicksDaily == 0)) )
		{
			if(banner != NULL)
			{
				banner->next = new BANNER;
				banner = banner->next;
			}
			else
			{
				banner = bannerList;
			}

			banner->Id = atoi(row[0]);
			banner->ProfileId = atoi(row[1]);
			banner->NetworkId = (short)atoi(row[2]);
			strncpy(banner->File, row[3], 19);
			strncpy(banner->Href, row[4], 159);
			strncpy(banner->String, row[5], 104);
			banner->Priority = (float)atof(row[6]);
			banner->AccountId = atoi(row[7]);
			strncpy(banner->HrefHash, row[9], 32);
		}
	}
	mysql_free_result(result);
	banner = NULL;
	//

	if(globalError)
	{ // во время загрузки возникла ошибка
    loading_error_exit(globalError);
	}

  #ifdef DEBUG
		// Статистика
		gettimeofday(&end_time, NULL);
		double timediff = (end_time.tv_sec - start_time.tv_sec) + ((double)end_time.tv_usec - (double)start_time.tv_usec)/1000000;
		printf("data_load() time: %f seconds\n", timediff);
	#endif
}


static void data_clear(void)
{
  if(siteList != NULL) delete siteList;
	if(profileList != NULL) delete profileList;
	if(bannerList != NULL) delete bannerList;
}

static void PrintEnv(char *label, char **envp)
{
	printf("%s:\n", label);
	for( ; *envp != NULL; envp++)
	{
		printf("%s\n", *envp);
	}
	printf("\n");
}




int main()
{
  // Инициализация глобальных переменных
  query = new char[65536];
	areaCodeNumber = 0;
	globalError = 0;

	// Инициализация генератора случайных чисел с помощью номера процесса
	init_genrand(getpid());

  // Подключение к MySQL
  connection = mysql_init(NULL);
  if(connection == NULL)
  {
    loading_error_exit(1);
  }
  if(mysql_real_connect(connection, NULL, MTOP_MYSQL_RO_USER, MTOP_MYSQL_RO_PASSWD, MTOP_MYSQL_DB, 0, NULL, 0) == NULL)
  {
    loading_error_exit(1);
  }

	// Загрузка регионов/городов
	// Загрузка данных геотаргетинга
	gi = GeoIP_open(MTOP_GEOIP_FILE, GEOIP_MEMORY_CACHE);
	if(gi == NULL)
	{
    loading_error_exit(3);
	}

	// Составляем массив соответствий areaCodeArr[AreaId] = AreaCode
	mysql_query_check(globalError, connection, "SELECT AreaId, AreaCode FROM Area;");
	result = mysql_store_result(connection);
	areaCodeNumber = mysql_num_rows(result);
	if(areaCodeNumber > 0) areaCodeArr = new int[areaCodeNumber];
	while((row = mysql_fetch_row(result)) != NULL)
	{
	  i = atoi(row[0]);
	  if(i > 0 && i < areaCodeNumber)	areaCodeArr[i] = atoi(row[1]);
	}
	mysql_free_result(result);

	// Открытие канала FIFO
	int fd = open(MTOP_FIFO_FILE, O_WRONLY | O_NONBLOCK);
	if(fd < 0)
	{
    loading_error_exit(133);
	}

  // Загрузка данных
	data_load();

	//// ПЕРЕМЕННЫЕ, необходимые для работы основного цикла

  #ifdef DEBUG
		struct timeval start_time, end_time;  // необходимо для определения времени работы основного цикла
	#endif
	struct timeval load_timeout, tmp_time;	// таймаут с предыдущей загрузки данных
	gettimeofday(&load_timeout, NULL);
	
  char *query_string = new char[1024];
  char *remote_addr = new char[50];
  char *p;
  int siteId;
  int accountId;
  int areaCodeId;
	time_t time_of_day;
	struct tm *tmbuf;

	PROFILE **profileCheckedArr;
	if(profileNum > 0) profileCheckedArr = new PROFILE*[profileNum];
	int	profileCheckedNum;

	BANNERCHECKED	*bannerCheckedArr;
	if(bannerNum) bannerCheckedArr = new BANNERCHECKED[bannerNum];
	int	bannerCheckedNum;
	int newPriority;
	int prioritySumm;

	unsigned long	rnd;
	unsigned long trafficKey;

	SITE *currentSite;
	BANNER *currentBanner;
	unsigned int bannerShowNum;

	TRAFFIC currentShow;

	int found;
	int error;

	int outputCodeType;

/////// ОСНОВНОЙ ЦИКЛ

	while(FCGI_Accept() >= 0)
	{
		#ifdef DEBUG
			printf("Content-type: text/plain\r\n\r\n");
			PrintEnv("Request environment", environ);
		#endif

    // Определение необходимости перезагрузки данных
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec - load_timeout.tv_sec > MTOP_DATA_TIMEOUT)
		{
			data_clear();										// Очистка памяти
			data_load();										// Перезагрузка данных
			gettimeofday(&load_timeout, NULL);
		}
		
		#ifdef DEBUG
			gettimeofday(&start_time, NULL);
		#endif

		// Инициализация переменных
		memset(query_string, 0, 1024);		// строка запроса
		memset(remote_addr, 0, 50);       // адрес

		p = NULL;                         // временная переменная

		siteId = 0;                       // ID сайта и аккаунта, полученные
		accountId = 0;                    // из строки запроса. Не доверять!
		areaCodeId = 0;                   // ID области (именно ID)

	  tmp_ui = 0;

		time_of_day = time(NULL);
		tmbuf = localtime(&time_of_day);  // структура с днём недели и т.п.

		for(i = 0; i < profileNum; i++)
		{ // обнуляем массив
			profileCheckedArr[i] = NULL;
		}
		profileCheckedNum = 0;

		for(i = 0; i < bannerNum; i++)
		{ // обнуляем массив
			bannerCheckedArr[i].banner = NULL;
			bannerCheckedArr[i].Priority = 0;
		}
		bannerCheckedNum = 0;

		newPriority = 0;									// временная переменная
		prioritySumm = 0;                 // сумма приоритетов подходящих баннеров

		currentSite = NULL;								// сайт, на котором происходит показ
		currentBanner = NULL;							// баннер, который показывается
		bannerShowNum = 1;								// количество баннеров, которые необходимо показать

		error = 0;                        // во время выполнения возникла какая-от ошибка
		found = 0;												// временная переменная
		outputCodeType = 0;               // тип выводимого кода (TEXT, WAP, картинка)

	  // Переносим переменную окружения QUERY_STRING в локальную переменную
	  p = getenv("QUERY_STRING");
		if(p != NULL)
		{
		  strncpy(query_string, p, 1024);
		}

		#ifdef DEBUG
		  printf("query_string = %s\n", query_string);
		#endif

	  // Выделяем тип вывода
	  // !!!!!! доработать
	  p = strtok(query_string, ";");
	  if(p != NULL) outputCodeType = atoi(p);
	  p = strtok(NULL, ";"); 									// ID аккаунта
	  if(p != NULL) siteId = atoi(p);
	  p = strtok(NULL, ";");									// ID сайта
	  if(p != NULL) accountId = atoi(p);

	  if(outputCodeType == CODE_TEXT_PHP)
	  {
		  p = strtok(NULL, ";");								// Количество баннеров для показа
		  if(p != NULL) bannerShowNum = atoi(p);
		  p = strtok(NULL, ";");								// IP-адрес, тип - int
		  if(p != NULL) tmp_ui= atoi(p);
	  }

	  // Проверка bannerShowNum in [1,MTOP_MAX_BANNER_NUM]
	  if( (bannerShowNum <= 0) || (bannerShowNum > MTOP_MAX_BANNER_NUM) )
	  {
			bannerShowNum = 1;
	  }

		#ifdef DEBUG
		  printf("outputCodeType = %d\n", outputCodeType);
		  printf("siteId         = %d\n", siteId);
		  printf("accountId      = %d\n", accountId);
		#endif

		// Определяем IP-адрес
		if(tmp_ui > 0)
		{
			ipntoa(tmp_ui, remote_addr);
		}
		else
		{
		  // Переносим переменную окружения REMOTE_ADDR в локальную переменную
		  p = getenv("REMOTE_ADDR");
			if(p != NULL)
			{
				strncpy(remote_addr, p, 49);
			}
			else
			{
				strncpy(remote_addr, MTOP_SERVER_IP, 49);
			}
		}

	  // Проверяем ID аккаунта и сайта на корректность (частично)
	  if(siteId <= 0 || accountId <= 0)
	  {
			error = 2;
	  }

		// Находим сайт, на котором происходит показ
		// Таким образом проверяем корректность siteId и accountID
		if(!error)
		{
		  // перебираем все сайты (обход списка)
			for(site = siteList; site != NULL; site = site->next)
			{
				if( (site->Id == siteId) && (site->AccountId == accountId) )
				{ // нашли сайт, устанавливаем указатель на него
					currentSite = site;
					#ifdef DEBUG
					  printf("Site found:\n");
					  currentSite->show();
					#endif
					break;
				}
			}
			if(currentSite == NULL)
			{ // не нашли сайт, ошибка
				error = 4;
			}
		}
		//

	  printf("IP (%s)\n", remote_addr);

		// Находим область (город или регион) по адресу
		if(!error)
		{
			#ifdef DEBUG
			  printf("IP (%s) ", remote_addr);
			#endif
			gir = GeoIP_record_by_addr(gi, remote_addr);
			if(gir != NULL)
			{ // нашли регион для IP
			  if(gir->region != NULL)
			  {
				  areaCodeId = atoi(gir->region);
					#ifdef DEBUG
					  printf(" FOUND. ", remote_addr);
					  printf("Country_code=%s, ", gir->country_code);
					  printf("region=%s, ", gir->region);
						printf("city=%s\n", gir->city);
					#endif
					GeoIPRecord_delete(gir);
				}
			}
		}

		// Перебираем профили (нерекурсивный обход списка)
		if(!error)
		{
			for(profile = profileList; profile != NULL; profile = profile->next)
			{ // отбор подходящего профиля
				#ifdef DEBUG
					printf("Profile:\n");
					profile->show();
				#endif

				// проверяем совпадение сайта
				if(profile->UserId == currentSite->UserId)
				{
					if(profile->OwnSites == MTOP_NET_OWNSITES_NO)
					{
						#ifdef DEBUG
							printf("MTOP_NET_OWNSITES_NO, rejecting profile:\n");
						#endif
						continue;
					}
				}

				// проверяем день недели
				if(!(dayOfWeekBitArr[tmbuf->tm_wday] & profile->DayOfWeek))
				{
					#ifdef DEBUG
						printf("DayOfWeek, rejecting profile:\n");
					#endif
					continue;
				}
	
				// проверяем время суток
				if(!(timeBitArr[tmbuf->tm_wday] & profile->Time))
				{
					#ifdef DEBUG
						printf("Time, rejecting profile:\n");
					#endif
					continue;
				}

				// Проверяем директ-таргетинг (белый список)
				if(profile->TargetingWhiteNumber > 0)
				{
				  found = 0;
				  for(i = 0; i < profile->TargetingWhiteNumber; i++)
				  {
				  	if(profile->TargetingWhite[i] == currentSite->Id)
				  	{
				  	  found = 1;
				  		break;
				  	}
				  }
				  // не найдено в белом списке - пропускаем профиль
				  if(!found) continue;
				}

				// Проверяем директ-таргетинг (черный список)
				if(profile->TargetingBlackNumber > 0)
				{
				  found = 0;
				  for(i = 0; i < profile->TargetingBlackNumber; i++)
				  {
				  	if(profile->TargetingBlack[i] == currentSite->Id)
				  	{
				  	  found = 1;
				  		break;
				  	}
				  }
				  // найдено в черном списке - пропускаем профиль
				  if(found) continue;
				}

				// Проверяем таргетинг по рубрикам
				if(profile->TargetingRubricNumber > 0)
				{
				  found = 0;
				  for(i = 0; i < profile->TargetingRubricNumber; i++)
				  {
				  	if(profile->TargetingRubric[i] == currentSite->RubricId)
				  	{
				  	  found = 1;
				  		break;
				  	}
				  }
				  // не найдено в рубриках - пропускаем профиль
				  if(!found) continue;
				}

				// Проверяем геотаргетинг
				if(profile->TargetingAreaNumber > 0)
				{
				  found = 0;
				  for(i = 0; i < profile->TargetingAreaNumber; i++)
				  {
				  	if(profile->TargetingArea[i] == areaCodeId)
				  	{
				  	  found = 1;
				  		break;
				  	}
				  }
				  // не найдено в рубриках - пропускаем профиль
				  if(!found) continue;
				}

				// добавляем в массив проверенных профилей
				profileCheckedArr[profileCheckedNum++] = profile;
			}
		}

		#ifdef DEBUG
	  printf("Selected profiles:\n");
		for(i = 0; i < profileCheckedNum; i++)
		{
		  profileCheckedArr[i]->show();
		}
		#endif

		// Перебираем баннеры и составляем массив сумм
		#ifdef DEBUG
		  printf("Total banners. bannerNum=%d\n", bannerNum);
		#endif
		if(!error)
		{
			for(banner = bannerList; banner != NULL; banner = banner->next)
			{
			  if( (banner->NetworkId == currentSite->NetworkId) && (banner->Priority > 0) )
			  {
					#ifdef DEBUG
					  printf("Banner:\n");
					  banner->show();
					#endif
			    found = 1;
			    // поиск в массиве подходящих профилей
					for(i = 0; i < profileCheckedNum; i++)
					{
					  profile = profileCheckedArr[i];
						if(banner->ProfileId == profile->Id)
						{
						  if(profile->OwnSites == MTOP_NET_OWNSITES_CHECK)
						  { // Требуется проверка URL сайта и баннера
							  if(strcmp(banner->HrefHash, currentSite->UrlHash) == 0)
							  { // URL совпадают, баннер не подходит
									found = 0;
									#ifdef DEBUG
									  printf("OwnSites == MTOP_NET_OWNSITES_CHECK, HrefHash = UrlHash, found = 0\n\n");
									#endif
							  	break;
							  }
							}
						  if( (profile->OwnSites == MTOP_NET_OWNSITES_NO) && (currentSite->UserId == profile->UserId))
						  { // Показ на своих сайтах запрещен
								#ifdef DEBUG
								  printf("MTOP_NET_OWNSITES_NO, Site->UserId = profile->UserId\n\n");
								#endif
						  	found = 0;
								break;
							}
						}
					}

					// баннер в профиле и подходит для показа
					if(found)
					{
					  // Добавляем баннер к массиву проверенных
					  newPriority = (int)(banner->Priority*200);
						bannerCheckedArr[bannerCheckedNum].banner = banner;
						bannerCheckedArr[bannerCheckedNum].Priority = prioritySumm + newPriority;
						prioritySumm += newPriority;

						#ifdef DEBUG
							printf("added banner to bannerCheckedArr[%d], ", bannerCheckedNum);
						  printf("Id: %d, ", banner->Id);
							printf("PrioritySumm: %d\n\n", prioritySumm);
						#endif

						bannerCheckedNum++;
						//
					}
				}
			}
		}

		// Цикл показов баннера
		for(j = 0; (j < bannerShowNum) && (j < MTOP_MAX_BANNER_NUM); j++)
		{
			// Если хотя бы один баннер можно показать
			if((bannerCheckedNum > 0) && (prioritySumm > 0))
			{
				rnd = genrand_int32();			// генерация случайного числа (скорость - ~10 млн. раз в сек)
				trafficKey = rnd;						// ключ для записи в базу трафика

				// Выбираем случайное целое число из интервала [0, prioritySumm-1]
				// можно попробовать заменить на (int)genrand_real2()*prioritySumm
				// т.е. rnd [0,1) * prioritySumm
				if(!error)
				{
					rnd = rnd % prioritySumm;
				}

				// определяем выпавший баннер
				if(!error)
				{
					for(i = 0; i < bannerCheckedNum; i++)
					{
						if(rnd < bannerCheckedArr[i].Priority)
						{
						  currentBanner = bannerCheckedArr[i].banner;
							break;
						}
					}
				}

				// Передача данных обработчику статистики
				if(!error)
				{
					currentShow.Type = mtop_signature_show;
					currentShow.SiteId = siteId;
					currentShow.AccountId = accountId;
					currentShow.NetworkId = currentSite->NetworkId;
					currentShow.BannerId = currentBanner->Id;
					currentShow.BannerAccountId = currentBanner->AccountId;
					currentShow.IP = ipaton(remote_addr);
					currentShow.Key = trafficKey;

				  p = getenv("HTTP_REFERER");
					if(p != NULL)
					{
					  strncpy(currentShow.Referer, p, 128);
					}

					// Находим и записываем AreaId
					for(i = 0; i < areaCodeNumber; i++)
					{
						if(areaCodeArr[i] == areaCodeId)
						{
							currentShow.AreaId = i;
							break;
						}
					}
				
					// Запись в канал
					if(write(fd, &currentShow, sizeof(TRAFFIC)) == -1)
					{
						#ifdef DEBUG
							printf("ERROR writing\n");
						#endif
					}
				}
			}
			//

			// Вывод отклика
			if((currentBanner != NULL) && !error)
			{ // баннер найден
				#ifdef DEBUG
					printf("\n-----------------\nSHOW BANNER:\n");
				  currentBanner->show();
				#endif

				switch(outputCodeType)
				{
					case CODE_IFRAME:
					{
					  if(j == 0)
					  {
							printf("Content-type: text/html\r\n\r\n");
						}
						printf("<html><head><title></title></head><body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0><a href=\"http://nw.mtop.ru/go.cgi?%u;%u;%u\" target=\"_blank\"><img src=\"http://mtop.ru/i/%s\" width=%d height=%d border=0 alt=\"%s\"></a><br></body></html>\n", currentBanner->Id, siteId, trafficKey, currentBanner->File, currentSite->XSize, currentSite->YSize, currentBanner->String);
					 	break;
					}
					case CODE_TEXT:
					{
					  if(j == 0)
					  {
							printf("Content-type: application/x-javascript\r\n\r\n");
						}
						printf("document.write('<a href=\"http://nw.mtop.ru/go.cgi?%u;%u;%u\" target=\"_blank\">%s</a><br>');\r\n", currentBanner->Id, siteId, trafficKey, currentBanner->String);
					 	break;
					}
					case CODE_WAP:
					{
					  printf("Location: http://mtop.ru/i/%s\r\n\r\n", currentBanner->File);
					 	break;
					}
					case CODE_TEXT_PHP:
					{
					  if(j == 0)
					  {
							printf("Content-type: text/plain\r\n\r\n");
						}
						printf("<a href=\"http://nw.mtop.ru/go.cgi?%u;%u;%u\" target=\"_blank\">%s</a>\n", currentBanner->Id, siteId, trafficKey, currentBanner->String);
					 	break;
					}
				}
			}
			else
			{ // нет ни одного подходящего баннера или возникла ошибка
				// показываем загрушку для данной сети
			  if(j == 0)
			  {
					printf("Content-type: text/html\r\n\r\n");
				}
				if(currentSite != NULL)
				{
					#ifdef DEBUG
						printf("No banner found\n");
					#endif
					show_default_banner(currentSite->NetworkId, outputCodeType);
				}
				else
				{ // не найден даже сайт
					#ifdef DEBUG
						printf("No banner found, no site found\n");
					#endif
					show_default_banner();
				}
			}
		}
		//

		#ifdef DEBUG
			printf("Process ID: %d\n", getpid());
			gettimeofday(&end_time, NULL);
			double timediff = (end_time.tv_sec - start_time.tv_sec) + ((double)end_time.tv_usec - (double)start_time.tv_usec)/1000000;
			printf("main while{} time: %f seconds\n", timediff);
		#endif
	}

/////// ДЕИНИЦИАЛИЗАЦИЯ: закрываем соединение с MySQL, освобождаем память

  // Закываем соединение с MySQL
  mysql_close(connection);

  // Освобождаем память
	delete query;
	delete query_string;
	delete remote_addr;
	delete areaCodeArr;

	// Закрываем базу GeoIP
	GeoIP_delete(gi);

	return 0;
}
