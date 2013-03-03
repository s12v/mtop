#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <string.h>

#include "config.h"

class TRAFFIC
{
public:

  short					Type;									// Тип трафика: показ или клик
	int						SiteId;               // ID сайта, на котором произошёл показ(клик)
	int						AccountId;            // Аккаунт, который показал баннер
	short					NetworkId;            // Сеть, в которой произошло событие
	int						BannerId;             // ID баннера
	int						BannerAccountId;      // Аккаунт показавшего баннер
	int						AreaId;               // ID области (зависит от IP)
	unsigned int	IP;                   // IP-адрес
	unsigned int	Key;                  // Ключ показа
	char					Referer[128];         // HTTP_REFERER

	TRAFFIC()
	{
		clear();
	}

	void clear()
	{
	  Type = 0;
		SiteId = 0;
		AccountId = 0;
		NetworkId = 0;
		BannerId = 0;
		BannerAccountId = 0;
		IP = 0;
		Key = 0;
		memset(Referer, 0, 128);
	}

//#ifdef DEBUG
	void show()
	{
	  char buf[24];
	  ipntoa(IP, buf);
		
		printf("Type            = %d\n", Type);
		printf("SiteId          = %d\n", SiteId);
		printf("AccountId       = %d\n", AccountId);
		printf("NetworkId       = %d\n", NetworkId);
		printf("BannerId        = %d\n", BannerId);
		printf("BannerAccountId = %d\n", BannerAccountId);
		printf("IP              = %s (%u)\n", buf, IP);
		printf("Key             = %u\n", Key);
		printf("Referer         = %s\n", Referer);
		printf("\n");
	}
//#endif
};


class TRAFFICKEY
{
public:

	int						SiteId;
	int						BannerId;
	int						BannerAccountId;
	short					NetworkId;
	short					AreaId;
	unsigned int	IP;
	unsigned int	Key;
	
	TRAFFICKEY()
	{
	  SiteId = 0;
		BannerId = 0;
		NetworkId = 0;
		AreaId = 0;
		IP = 0;
		Key = 0;
	}
	
	void clear()
	{
	  SiteId = 0;
		BannerId = 0;
		NetworkId = 0;
		AreaId = 0;
		IP = 0;
		Key = 0;
	}

	void show()
	{
		printf("SiteId          = %d\n", SiteId);
		printf("BannerId        = %d\n", BannerId);
		printf("BannerAccountId = %d\n", BannerAccountId);
		printf("NetworkId       = %d\n", NetworkId);
		printf("AreaId          = %d\n", AreaId);
		printf("IP              = %u\n", IP);
		printf("Key             = %u\n", Key);
		printf("\n");
	}
};


class TRAFFICSTACK
{
private:
	
	TRAFFICKEY *data;
	int maxNumber;
	int currentNumber;
	int i, found;

public:
	
	TRAFFICSTACK(int num)
	{
	  data = NULL;
		currentNumber = 0;

		maxNumber = num;
		data = new TRAFFICKEY[maxNumber];
	}

	~TRAFFICSTACK()
	{
		if(data != NULL) delete data;
	}

	void push(TRAFFIC *show)
	{
	  if(show != NULL)
	  {
			if(currentNumber >= maxNumber)
			{
				currentNumber = 0;
			}

			// Добавляем элемент
			data[currentNumber].SiteId = show->SiteId;
			data[currentNumber].BannerId = show->BannerId;
			data[currentNumber].BannerAccountId = show->BannerAccountId;
			data[currentNumber].NetworkId = show->NetworkId;
			data[currentNumber].AreaId = show->AreaId;
			data[currentNumber].IP = show->IP;
			data[currentNumber].Key = show->Key;

			currentNumber++;
		}
	}

	int check_click(TRAFFIC *click)
	{
    found = 0;
	  if(click != NULL)
	  {
			for(i = 0; i < maxNumber; i++)
			{
			  if(data[i].SiteId != 0)
			  {
					if( (data[i].SiteId == click->SiteId) && (data[i].BannerId == click->BannerId) &&
					    (data[i].IP == click->IP) && (data[i].Key == click->Key))
					{
					  // Записываем ID аккаунта баннера
					  click->BannerAccountId = data[i].BannerAccountId;
					  click->NetworkId = data[i].NetworkId;
					  click->AreaId = data[i].AreaId;
						data[i].clear(); // очистка элемента, для избежания повторного нахождения
						                 // при следующем цикле
						found = 1;
						break;
					}
				}
			}
		}
		return found;
	}
};

#endif