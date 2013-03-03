/*
 *  Обработка клика
 *
 */

#include <fcgi_stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

//#include <iostream.h>

#include "mysql.h"
#include "config.h"
#include "traffic.h"

// Глобальные переменные
extern int errno;

static char* query;

static MYSQL			*connection;
static MYSQL_RES	*result;
static MYSQL_ROW	row;

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


int main()
{
	// Локальные переменные
	int fd;
  char *query_string = new char[1024];
  char *remote_addr = new char[50];
  char *href = new char[1024];
  char *p = NULL;
  int bannerSQLFound;

  TRAFFIC currentClick;

	// Инициализация глобальных переменных
	query = new char[65536];
	memset(query, 0, 65536);
	//

	// Открытие канала FIFO
	fd = open(MTOP_FIFO_FILE, O_WRONLY | O_NONBLOCK);
	if(fd < 0)
	{
    fatal(133);
	}

  // Подключение к MySQL
  connection = mysql_init(NULL);
  if(connection == NULL)
  {
	  fatal(103);
  }
  if(mysql_real_connect(connection, NULL, MTOP_MYSQL_RO_USER, MTOP_MYSQL_RO_PASSWD, MTOP_MYSQL_DB, 0, NULL, 0) == NULL)
  {
	  fatal(104);
  }
	//

	//// Основной цикл
	while(FCGI_Accept() >= 0)
	{
		#ifdef DEBUG
			printf("Content-type: text/plain\r\n\r\n");
		#endif

	  currentClick.clear();
	  currentClick.Type = mtop_signature_click;
		memset(query_string, 0, 1024);		// строка запроса
		memset(remote_addr, 0, 50);				// адрес
		memset(href, 0, 1024);						// ссылка баннера
		bannerSQLFound = 0;
		
	  // Переносим переменную окружения QUERY_STRING в локальную переменную
	  p = getenv("QUERY_STRING");
		if(p != NULL)
		{
		  strncpy(query_string, p, 1024);
		}

	  // Выделяем параметры
	  // !!!!!! доработать
	  p = strtok(query_string, ";");
	  if(p != NULL) currentClick.BannerId = atoi(p);		// ID баннера
	  p = strtok(NULL, ";"); 									
	  if(p != NULL) currentClick.SiteId = atoi(p);			// ID сайта
	  p = strtok(NULL, ";"); 									
	  if(p != NULL) currentClick.Key = atoi(p);					// Ключ

	  // Переносим переменную окружения REMOTE_ADDR в локальную переменную
	  p = getenv("REMOTE_ADDR");
		if(p != NULL)
		{
			strncpy(remote_addr, p, 49);
		}
		currentClick.IP = ipaton(remote_addr);

	  p = getenv("HTTP_REFERER");
		if(p != NULL)
		{
		  strncpy(currentClick.Referer, p, 128);
		}

		// Выборка из БД
		sprintf(query, "SELECT BannerId, BannerHref FROM Banner WHERE BannerId = '%d'", currentClick.BannerId);
		mysql_query_check(globalError, connection, query);
		result = mysql_store_result(connection);
		bannerSQLFound = mysql_num_rows(result);
	
		if((row = mysql_fetch_row(result)) != NULL)
		{
			currentClick.BannerId = atoi(row[0]);
		  strncpy(href, row[1], 1024);
		}
		mysql_free_result(result);
		//

		if(bannerSQLFound == 1)
		{
			// Запись в канал
			if(write(fd, &currentClick, sizeof(TRAFFIC)) == -1)
			{
				#ifdef DEBUG
					printf("ERROR writing\n");
				#else
					printf("Location: http://www.mtop.ru/\r\n\r\n");
				#endif
			}

//			printf("Content-type: text/html\r\n\r\n");
			printf("Location: http://%s\r\n\r\n", href);
		}
		else
		{ // Баннер с заданным Id не найден в БД
			printf("Content-type: text/html\r\n\r\n");
			printf("Location: http://www.mtop.ru/\r\n\r\n");
		}
	}
	////

	// Закрываем канал FIFO
	close(fd);

	// Очищаем память
	delete query;
	
	return 0;
}
