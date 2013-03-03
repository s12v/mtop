#ifndef SHOW_H
#define SHOW_H

#include <stdio.h>

class PROFILE
{
public:
	
	PROFILE* next;
	
	int 		Id;
	short		UserId;
	short 	OwnSites;
	int 		DayOfWeek;
	short 	Time;

	short*	TargetingArea;
	int*		TargetingWhite;
	int*		TargetingBlack;
	short*	TargetingRubric;

	unsigned short		TargetingAreaNumber;
	unsigned short		TargetingWhiteNumber;
	unsigned short		TargetingBlackNumber;
	unsigned short		TargetingRubricNumber;

	PROFILE()
	{
	  next = NULL;
		
		Id = 0;
		UserId = 0;
		OwnSites = 0;
		DayOfWeek = 0;
		Time = 0;

		TargetingArea = NULL;
		TargetingWhite = NULL;
		TargetingBlack = NULL;
		TargetingRubric = NULL;

		TargetingAreaNumber = 0;
		TargetingWhiteNumber = 0;
		TargetingBlackNumber = 0;
		TargetingRubricNumber = 0;
	}

	~PROFILE()
	{
		if(TargetingAreaNumber > 0) delete TargetingArea;
		if(TargetingWhiteNumber > 0) delete TargetingWhite;
		if(TargetingBlackNumber > 0) delete TargetingBlack;
		if(TargetingRubricNumber > 0) delete TargetingRubric;
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("next      = %d\n", next);
		
		printf("Id        = %d\n", Id);
		printf("UserId    = %d\n", UserId);
		printf("OwnSites  = %d\n", OwnSites);
		printf("DayOfWeek = %d\n", DayOfWeek);
		printf("TargetingAreaNumber = %d\n", TargetingAreaNumber);
		printf("TargetingWhiteNumber = %d\n", TargetingWhiteNumber);
		printf("TargetingBlackNumber = %d\n", TargetingBlackNumber);
		printf("TargetingRubricNumber = %d\n\n", TargetingRubricNumber);
	}
};


class BANNER
{
public:
	BANNER* next;
	
	int			Id;
	int			ProfileId;
	int			AccountId;
	short		NetworkId;
	char		File[20];
	char		Href[160];
	char		HrefHash[33];
	char		String[105];
	int			Daily;
	float		Priority;

	BANNER()
	{
		next = NULL;
		Id = 0;
		ProfileId = 0;
		AccountId = 0;
		NetworkId = 0;
		memset(File, 0, 20);
		memset(Href, 0, 160);
		memset(HrefHash, 0, 33);
		memset(String, 0, 105);
		Daily = 0;
		Priority = 0.1;
	}

	~BANNER()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("next      = %d\n", next);
		printf("Id        = %d\n", Id);
		printf("ProfileId = %d\n", ProfileId);
		printf("AccountId = %d\n", AccountId);
		printf("NetworkId = %d\n", NetworkId);
		printf("File      = %s\n", File);
		printf("Href      = %s\n", Href);
		printf("String    = %s\n", String);
		printf("Daily     = %d\n", Daily);
		printf("Priority  = %f\n\n", Priority);
	}

};


class BANNERCHECKED
{
public:
	BANNER* banner;
	int	Priority;

	BANNERCHECKED()
	{
		banner = NULL;
	}
};


class SITE
{
public:
	SITE		*next;

  int			Id;
	int			UserId;
	int			RubricId;
	int			AccountId;
	int			NetworkId;
	char		Url[160];
	char		UrlHash[33];

	int			XSize;
	int			YSize;

	SITE()
	{
		next = NULL;

		Id = 0;									// ID сайта
		UserId = 0;             // ID пользователя
		RubricId = 0;           // ID рубрики
		AccountId = 0;          // ID аккаунта
		NetworkId = 0;          // ID сети
		memset(Url, 0, 160);
		memset(UrlHash, 0, 33);

		XSize = 0;
		YSize = 0;
	}

	~SITE()
	{
		// рекурсивный обход списка
		if(next != NULL) delete next;
	}

	void show()
	{
		printf("Id             = %d\n", Id);
		printf("UserId         = %d\n", UserId);
		printf("RubricId       = %d\n", RubricId);
		printf("AccountId      = %d\n", AccountId);
		printf("NetworkId      = %d\n", NetworkId);
		printf("Url            = %s\n", Url);
		printf("XSize          = %d\n", XSize);
		printf("YSize          = %d\n\n", YSize);
	}
};

#endif
