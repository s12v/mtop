#ifndef MTOPSTAT_H
#define MTOPSTAT_H

class ACCOUNT
{
public:
	ACCOUNT*			next;					// Указатель на следующий эл-т списка
	
	int						Id;						// AccountId
	double				Profit;				// Изменение баланса, может быть > или < 0
	float					FinalRate;		// Коэффициент обмена, с учетом AccountRate и AccountRateSub

	unsigned int	Shows;				// Количество показов на сайтах аккаунта
	unsigned int	Clicks;				// Количество кликов на сайтах аккаунта

	ACCOUNT()
	{
		next = NULL;

		Id = 0;
		Profit = 0;
		FinalRate = 0.9;

		Shows = 0;
		Clicks = 0;
	}

	~ACCOUNT()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("next      = %d\n", next);
		printf("Id        = %d\n", Id);
		printf("Profit    = %f\n", Profit);
		printf("FinalRate = %f\n", FinalRate);
		printf("Shows     = %u\n", Clicks);
		printf("Clicks    = %u\n", Clicks);
		printf("\n");
	}

};

class BANNERSTAT
{
public:
	BANNERSTAT*		next;
	
	int						Id;
	unsigned int	Shows;
	unsigned int	Clicks;

	BANNERSTAT()
	{
		next = NULL;

		Id = 0;
		Shows = 0;
		Clicks = 0;
	}

	~BANNERSTAT()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("next      = %d\n", next);
		printf("Id        = %d\n", Id);
		printf("Shows     = %d\n", Shows);
		printf("Clicks    = %d\n", Clicks);
		printf("\n");
	}
};


class SITESTAT
{
public:
	SITESTAT			*next;

  int						Id;
  short int			NetworkId;
  unsigned int	Shows;
  unsigned int	Clicks;

	SITESTAT()
	{
		next = NULL;

		Id = 0;							// ID сайта
		NetworkId = 0;			// ID сети
		Shows = 0;         	// Показы
		Clicks = 0;					// Клики
	}

	~SITESTAT()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("Id          = %d\n", Id);
		printf("NetworkId   = %d\n", NetworkId);
		printf("Shows       = %d\n", Shows);
		printf("Clicks      = %d\n", Clicks);
		printf("\n");
	}
};


class NETWORKSTAT
{
public:
	NETWORKSTAT		*next;
	
  int						Id;
  unsigned int	Shows;
  unsigned int	Clicks;

	NETWORKSTAT()
	{
		next = NULL;

		Id = 0;							// ID сайта
		Shows = 0;         	// Показы
		Clicks = 0;					// Клики
	}

	~NETWORKSTAT()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("Id          = %d\n", Id);
		printf("Shows       = %d\n", Shows);
		printf("Clicks      = %d\n", Clicks);
		printf("\n");
	}
};


class AREASTAT
{
public:
	AREASTAT		*next;
	
  int						Id;
  int						NetworkId;
  unsigned int	Shows;

	AREASTAT()
	{
		next = NULL;

		Id = 0;							// ID сайта
		NetworkId = 0;			// ID сайта
		Shows = 0;         	// Показы
	}

	~AREASTAT()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("Id          = %d\n", Id);
		printf("NetworkId   = %d\n", Id);
		printf("Shows       = %d\n", Shows);
		printf("\n");
	}
};

/*
class ADMINPROFIT
{
public:
	int						Id;						// AccountId
	double				Profit;				// Изменение баланса, может быть > или < 0

	ADMINPROFIT()
	{
		Profit = 0;
	}

	void show()
	{
		printf("Id        = %d\n", Id);
		printf("Profit    = %f\n", Profit);
		printf("\n");
	}

};
*/
#endif