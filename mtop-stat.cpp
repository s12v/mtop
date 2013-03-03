/*
 *  Сбор и обработка статистики
 *
 *  Плавный выход с сохранением данных - SIGUSR1
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

//#include <iostream.h>

#include "mysql.h"
#include "config.h"
#include "traffic.h"
#include "mtop-stat.h"

// Глобальные переменные
extern int errno;

static int fd;

static char* query;

static MYSQL			*connection;
static MYSQL_RES	*result;
static MYSQL_ROW	row;

static SITESTAT	*siteList;
static SITESTAT	*site;
static int			siteNum;

static BANNERSTAT	*bannerList;
static BANNERSTAT	*banner;
static int				bannerNum;

static ACCOUNT		*accountList;
static ACCOUNT		*account;
static int				accountNum;

static ACCOUNT		**adminAccount;			// Массив аккаунтов администратора
                                      // adminAccount[NetworkId] = указатель на элемент accountList;
static AREASTAT		*areaList;
static AREASTAT		*area;
static int				areaNum;

static NETWORKSTAT	*networkList;
static NETWORKSTAT	*network;
static int				networkNum;         // Количество сетей
static int				maxNetwork;         // Сеть с максимальным номером

static double			*adminProfitArr;

static int i, j, tmp, user_id, network_id;
static int globalError;
//

// Ошибка
int fatal(int error, const char* msg=NULL)
{
	printf("Error #%d\n", error);
	if(msg != NULL)
	{
		printf("message: %s\n", msg);
	}
	exit(error);
}

// Создание файла блокировки
int makelock(void)
{
	int fd;
	char buf[100];

	if((fd = open(MTOP_PID_FILE, O_RDWR | O_CREAT | O_EXCL, 0600)) < 0)
	{
		if(errno == EEXIST)
		{
		  fatal(1, "Lockfile exists");
		}
		else
		{
		  fatal(127);
		}
 	}
 	else
 	{
		sprintf(buf, "%d", getpid());
		write(fd, buf, strlen(buf)+1);
 	}
	close(fd);

	return 0;
}

// Удаление файла блокировки
int deletelock(void)
{
	unlink(MTOP_PID_FILE);

	return 0;
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


// Загрузка данных
static void data_load()
{
  #ifdef DEBUG
		struct timeval start_time, end_time;  // необходимо для определения времени работы основного цикла
		gettimeofday(&start_time, NULL);
	#endif

	// Инициализация глобальных переменных
	siteList = new SITESTAT;
	siteNum = 0;

	bannerList = new BANNERSTAT;
	bannerNum = 0;

	accountList = new ACCOUNT;
	accountNum = 0;

	networkList = new NETWORKSTAT;
	networkNum = 0;

	areaList = new AREASTAT;
	areaNum = 0;
	//

	// Загрузка сайтов
	mysql_query_check(globalError, connection, "SELECT SiteId, NetworkId FROM Site, Network");
	result = mysql_store_result(connection);
	siteNum = mysql_num_rows(result);

	site = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(site != NULL)
		{
			site->next = new SITESTAT;
			site = site->next;
		}
		else
		{
			site = siteList;
		}

		site->Id = atoi(row[0]);
		site->NetworkId = atoi(row[1]);
	}
	mysql_free_result(result);
	//

	// Загрузка баннеров
	mysql_query_check(globalError, connection, "SELECT BannerId FROM Banner");
	result = mysql_store_result(connection);
	bannerNum = mysql_num_rows(result);

	banner = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(banner != NULL)
		{
			banner->next = new BANNERSTAT;
			banner = banner->next;
		}
		else
		{
			banner = bannerList;
		}

		banner->Id = atoi(row[0]);
	}
	mysql_free_result(result);
	//

	// Загрузка аккаунтов
	mysql_query_check(globalError, connection, "SELECT AccountId, AccountRate, AccountRateSub, AccountUserId, AccountNetworkId FROM Account");
	result = mysql_store_result(connection);
	accountNum = mysql_num_rows(result);

	account = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(account != NULL)
		{
			account->next = new ACCOUNT;
			account = account->next;
		}
		else
		{
			account = accountList;
		}

		user_id = atoi(row[3]);
		network_id = atoi(row[4]);
		if(user_id == MTOP_ADMIN_USER_ID)
		{ // Аккаунт администратора
			adminAccount[network_id] = account;
		}

		account->Id = atoi(row[0]);
		account->FinalRate = (float)atof(row[1]) - (float)atof(row[2]);
	}
	mysql_free_result(result);
	//

	// Загрузка сетей
	mysql_query_check(globalError, connection, "SELECT NetworkId FROM Network");
	result = mysql_store_result(connection);
	networkNum = mysql_num_rows(result);

	network = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(network != NULL)
		{
			network->next = new NETWORKSTAT;
			network = network->next;
		}
		else
		{
			network = networkList;
		}

		network->Id = atoi(row[0]);
	}
	mysql_free_result(result);
	//

	// Загрузка областей
	mysql_query_check(globalError, connection, "SELECT AreaStatAreaId, AreaStatNetworkId FROM AreaStat");
	result = mysql_store_result(connection);
	areaNum = mysql_num_rows(result);

	area = NULL;
	while((row = mysql_fetch_row(result)) != NULL)
	{
		if(area != NULL)
		{
			area->next = new AREASTAT;
			area = area->next;
		}
		else
		{
			area = areaList;
		}

		area->Id = atoi(row[0]);
		area->NetworkId = atoi(row[1]);
	}
	mysql_free_result(result);
	//


  #ifdef DEBUG
		// Статистика
		gettimeofday(&end_time, NULL);
		double timediff = (end_time.tv_sec - start_time.tv_sec) + ((double)end_time.tv_usec - (double)start_time.tv_usec)/1000000;
		printf("data_load() time: %f seconds<br/>\n", timediff);
	#endif
}

// Очистка данных
static void data_clear()
{
  if(siteList != NULL) delete siteList;
	if(bannerList != NULL) delete bannerList;
	if(accountList != NULL) delete accountList;
	if(networkList != NULL) delete networkList;

	for(i = 0; i < maxNetwork; i++)
	{
		adminProfitArr[i] = 0;
	}
}

// Сохранение данных в базу MySQL
static void data_save()
{
  // Перебор сайтов
	for(site = siteList; site != NULL; site = site->next)
	{
		if( (site->Shows > 0) && (site->Clicks > 0) )
		{
		  sprintf(query, "UPDATE SiteStat SET SiteStatTodayShows = SiteStatTodayShows + '%d', SiteStatTodayClicks = SiteStatTodayClicks + '%d' WHERE SiteStatSiteId = '%d' AND SiteStatNetworkId = '%d'", site->Shows, site->Clicks, site->Id, site->NetworkId);
		}
		else if(site->Shows > 0)
		{
		  sprintf(query, "UPDATE SiteStat SET SiteStatTodayShows = SiteStatTodayShows + '%d' WHERE SiteStatSiteId = '%d' AND SiteStatNetworkId = '%d'", site->Shows, site->Id, site->NetworkId);
		}
		else if(site->Clicks > 0)
		{
		  sprintf(query, "UPDATE SiteStat SET SiteStatTodayClicks = SiteStatTodayClicks + '%d' WHERE SiteStatSiteId = '%d' AND SiteStatNetworkId = '%d'", site->Clicks, site->Id, site->NetworkId);
		}

	  if( (site->Shows > 0) || (site->Clicks > 0) )
	  {
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
    }
	}

  // Перебор аккаунтов
	for(account = accountList; account != NULL; account = account->next)
	{
		if(account->Profit != 0)
		{
		  sprintf(query, "UPDATE Account SET AccountBalance = AccountBalance  + '%f' WHERE AccountId = '%d'", account->Profit, account->Id);
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}

		if( (account->Shows > 0) && (account->Clicks > 0))
		{
		  sprintf(query, "UPDATE AccountStat SET AccountStatTodayShows = AccountStatTodayShows + '%u', AccountStatTodayClicks = AccountStatTodayClicks + '%u' WHERE AccountStatAccountId = '%d'", account->Shows, account->Clicks, account->Id);
		}
		else if( (account->Shows > 0))
		{
		  sprintf(query, "UPDATE AccountStat SET AccountStatTodayShows = AccountStatTodayShows + '%u' WHERE AccountStatAccountId = '%d'", account->Shows, account->Id);
		}
		else if( (account->Clicks > 0))
		{
		  sprintf(query, "UPDATE AccountStat SET AccountStatTodayClicks = AccountStatTodayClicks + '%u' WHERE AccountStatAccountId = '%d'", account->Clicks, account->Id);
		}
		if( (account->Shows > 0) || (account->Clicks > 0))
		{
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}
	}

  // Перебор баннеров
	for(banner = bannerList; banner != NULL; banner = banner->next)
	{
		if( (banner->Shows > 0) && (banner->Clicks > 0))
		{
		  sprintf(query, "UPDATE BannerStat SET BannerStatTodayShows = BannerStatTodayShows  + '%d', BannerStatTodayClicks = BannerStatTodayClicks + '%d' WHERE BannerStatBannerId = '%d'", banner->Shows, banner->Clicks, banner->Id);
		}
		else if( (banner->Shows > 0))
		{
		  sprintf(query, "UPDATE BannerStat SET BannerStatTodayShows = BannerStatTodayShows  + '%d' WHERE BannerStatBannerId = '%d'", banner->Shows, banner->Id);
		}
		else if( (banner->Clicks > 0))
		{
		  sprintf(query, "UPDATE BannerStat SET BannerStatTodayClicks = BannerStatTodayClicks + '%d' WHERE BannerStatBannerId = '%d'", banner->Clicks, banner->Id);
		}
		if( (banner->Shows > 0) || (banner->Clicks > 0))
		{
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}
	}

  // Перебор сетей
	for(network = networkList; network != NULL; network = network->next)
	{
		if( (network->Shows > 0) && (network->Clicks > 0))
		{
		  sprintf(query, "UPDATE NetworkStat SET NetworkStatTodayShows = NetworkStatTodayShows  + '%d', NetworkStatTodayClicks = NetworkStatTodayClicks + '%d' WHERE NetworkStatNetworkId = '%d'", network->Shows, network->Clicks, network->Id);
		}
		else if( (network->Shows > 0))
		{
		  sprintf(query, "UPDATE NetworkStat SET NetworkStatTodayShows = NetworkStatTodayShows  + '%d' WHERE NetworkStatNetworkId = '%d'", network->Shows, network->Id);
		}
		else if( (network->Clicks > 0))
		{
		  sprintf(query, "UPDATE NetworkStat SET NetworkStatTodayClicks = NetworkStatTodayClicks + '%d' WHERE NetworkStatNetworkId = '%d'", network->Clicks, network->Id);
		}
		if( (network->Shows > 0) || (network->Clicks > 0))
		{
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}

		// Сохранение adminProfit
		if(adminProfitArr[network->Id] > 0)
		{
		  sprintf(query, "UPDATE AdminProfitStat SET AdminProfitStatTodayShows = AdminProfitStatTodayShows  + '%f' WHERE AdminProfitStatNetworkId = '%d'", adminProfitArr[network->Id], network->Id);
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}
	}

  // Перебор областей
  // Сохраняем только показы
	for(area = areaList; area != NULL; area = area->next)
	{
		if(area->Shows > 0)
		{
		  sprintf(query, "UPDATE AreaStat SET AreaStatTodayShows = AreaStatTodayShows  + '%d' WHERE AreaStatAreaId = '%d' AND AreaStatNetworkId = '%d'", area->Shows, area->Id, area->NetworkId);
			#ifdef DEBUG
			  printf("%s\n", query);
			#endif
			mysql_query_check(globalError, connection, query);
		}
	}
}


void soft_exit(int s)
{
  printf("Soft exit\n");
	
	data_save();
	
	// Закрываем канал FIFO
	close(fd);

	// Очищаем память
	delete query;
	delete adminAccount;
	delete adminProfitArr;
	data_clear();
	
	// Удаляем файл блокировки
	deletelock();

	exit(0);
}



int main()
{
  // Меняем пользователя
  printf("Changing user to %s...\n", mtop_stat_user);
	
	struct passwd *pd;
	if((pd = getpwnam(mtop_stat_user)) == NULL)
	{
		fatal(100, "Cannot load data for user");
	}

	#ifdef DEBUG
		printf("User: %s, uid: %d, gid: %d\n", mtop_stat_user, pd->pw_uid, pd->pw_gid);
	#endif

	if(setuid(pd->pw_uid) != 0)	fatal(100, "Cannot change UID");
	//
	
	// Проверяем файл блокировки
	makelock();

	// TEST!
//	fd_test = fopen("/home/mtop/log/query.log");


	// Локальные переменные
	int signature;
	struct timeval load_timeout, tmp_time;	// таймаут с предыдущей загрузки данных
	struct timeval start_time, end_time;
	gettimeofday(&load_timeout, NULL);

	TRAFFIC currentTraffic;
	TRAFFICSTACK trafficStack(MTOP_STACK_SIZE);
	printf("Used %d Mb for stack\n", (MTOP_STACK_SIZE*sizeof(TRAFFICSTACK))/1048576);

	// Сигналы
  sigset_t set1;
  struct sigaction signalAct;
  signalAct.sa_handler = soft_exit;

  // Выбираем полный набор сигналов
  sigfillset(&set1);

	// Инициализация глобальных переменных
	query = new char[65536];
	memset(query, 0, 65536);

	maxNetwork = 0;
	//

	// Создание канала FIFO, если он ещё не существует,
	// открытие для чтения и записи
	if(mkfifo(MTOP_FIFO_FILE, 0666) == -1)
	{
		if(errno != EEXIST)
		{
		  fatal(101);
		}
	}
	if((fd = open(MTOP_FIFO_FILE, O_RDWR)) < 0)
	{
	  fatal(102, "Can't create lockfile");
	}
	//

  // Подключение к MySQL
  connection = mysql_init(NULL);
  if(connection == NULL)
  {
	  fatal(103);
  }
  if(mysql_real_connect(connection, NULL, MTOP_MYSQL_USER, MTOP_MYSQL_PASSWD, MTOP_MYSQL_DB, 0, NULL, 0) == NULL)
  {
	  fatal(104);
  }
	//

	// Инициализация аккаунтов администратора
	mysql_query_check(globalError, connection, "SELECT NetworkId FROM Network");
	result = mysql_store_result(connection);

	while((row = mysql_fetch_row(result)) != NULL)
	{
	  tmp = atoi(row[0]);
	  if(tmp > maxNetwork) maxNetwork = tmp;
	}
	#ifdef DEBUG
		printf("networkNum: %d, maxNetwork: %d\n", networkNum, maxNetwork);
	#endif
	adminAccount = new ACCOUNT*[maxNetwork+10];				// 10 - на всякий случай
	adminProfitArr = new double[maxNetwork+10];	//
	mysql_free_result(result);

	// Загрузка данных
	data_load();

	// Обработка сигнала SIGUSR1
  sigaction(SIGINT, &signalAct, NULL);
  sigaction(SIGUSR1, &signalAct, NULL);


	//// Основной цикл
	for(;;)
	{
	  signature = 0;
		
		// Чтение из канала
		if(read(fd, &currentTraffic, sizeof(TRAFFIC)) < 0)
		{
		  fatal(105);
		}
		if(currentTraffic.Type == mtop_signature_show)
		{ //// Показ
		  sigprocmask(SIG_SETMASK, &set1, NULL);  // Блокирование сигналов
	    // Определение необходимости перезагрузки данных
			gettimeofday(&tmp_time, NULL);
			if(tmp_time.tv_sec - load_timeout.tv_sec > MTOP_STAT_DATA_TIMEOUT)
			{
			  data_save();														// Сохранение данных
				data_clear();														// Очистка памяти
				data_load();														// Перезагрузка данных
				gettimeofday(&load_timeout, NULL);
			}

			// Обработка текущего показа

			// !!! добавить проверку Url

		  // Поиск сайта и добавление показа
			for(site = siteList; site != NULL; site = site->next)
			{
				if( (site->Id == currentTraffic.SiteId) && (site->NetworkId == currentTraffic.NetworkId) )
				{
					site->Shows++;
				}
			}

		  // Поиск аккаунта и добавление показа
			for(account = accountList; account != NULL; account = account->next)
			{
			  // Начисление показа на аккаунт сайта
				if(account->Id == currentTraffic.AccountId)
				{
					account->Profit += account->FinalRate;						// Обновление баланса
					account->Shows++;                                 // Добавляем показ
					// добавление показа на аккаунт администратора
					if(currentTraffic.NetworkId <= maxNetwork)
					{
					  // Обновляем баланс администратора
						adminAccount[currentTraffic.NetworkId]->Profit += (double)(1 - account->FinalRate);
						// Обновляем статистику баланса администратора
						adminProfitArr[currentTraffic.NetworkId] += (double)(1 - account->FinalRate);
					}
				}
				// Списание показа с аккаунта показывающего
				if(account->Id == currentTraffic.BannerAccountId)
				{
					account->Profit -= 1;
				}
			}

		  // Поиск баннера и добавление показа
			for(banner = bannerList; banner != NULL; banner = banner->next)
			{
				if(banner->Id == currentTraffic.BannerId)
				{
					banner->Shows++;
					break;
				}
			}

		  // Поиск сети и добавление показа
			for(network = networkList; network != NULL; network = network->next)
			{
				if(network->Id == currentTraffic.NetworkId)
				{
					network->Shows++;
					break;
				}
			}

		  // Поиск области и добавление показа
			for(area = areaList; area != NULL; area = area->next)
			{
				if(area->Id == currentTraffic.AreaId && (area->NetworkId == currentTraffic.NetworkId) )
				{
					area->Shows++;
					break;
				}
			}

			// Добавление в стек
			trafficStack.push(&currentTraffic);

			#ifdef DEBUG
				currentTraffic.show();
			#endif
		  sigprocmask(SIG_UNBLOCK, &set1, NULL);  // Разблокирование сигналов
		}
		if(currentTraffic.Type == mtop_signature_click)
		{ //// Клик
		  sigprocmask(SIG_SETMASK, &set1, NULL);  // Блокирование сигналов

			gettimeofday(&start_time, NULL);
			if(trafficStack.check_click(&currentTraffic) == 1)
			{ /** Клик проверен **/
				// Статистика
			  // Поиск аккаунта и добавление клика
				for(account = accountList; account != NULL; account = account->next)
				{
					if(account->Id == currentTraffic.BannerAccountId)
					{
						account->Clicks++;
					}
				}

			  // Поиск баннера и добавление клика
				for(banner = bannerList; banner != NULL; banner = banner->next)
				{
					if(banner->Id == currentTraffic.BannerId)
					{
						banner->Clicks++;
					}
				}

			  // Поиск сайта и добавление клика
				for(site = siteList; site != NULL; site = site->next)
				{
					if( (site->Id == currentTraffic.SiteId) && (site->NetworkId == currentTraffic.NetworkId) )
					{
						site->Clicks++;
					}
				}

			  // Поиск сети и добавление клика
				for(network = networkList; network != NULL; network = network->next)
				{
					if(network->Id == currentTraffic.NetworkId)
					{
						network->Clicks++;
					}
				}

				// Запись в таблицу статистики по кликам
				sprintf(query, "INSERT INTO Click (ClickSiteId, ClickNetworkId, ClickBannerId, ClickAreaId, ClickDate, ClickIP) VALUES ('%d', '%d', '%d', '%d', NOW(), '%u')", currentTraffic.SiteId, currentTraffic.NetworkId, currentTraffic.BannerId, currentTraffic.AreaId, currentTraffic.IP);
				mysql_query_check(globalError, connection, query);

			}
			#ifdef DEBUG
			else
			{
			  printf("Click NOT checked\n");
			}
				currentTraffic.show();
			#endif
		  sigprocmask(SIG_UNBLOCK, &set1, NULL);  // Разблокирование сигналов
		}
	}
	////

	soft_exit(0);

	return 0;
}
