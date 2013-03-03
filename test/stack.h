#ifndef STACK_H
#define STACK_H

class TRAFFICSTACK
{
private:
	
	int *data;
	int maxNumber;
	int currentNumber;

public:
	
	TRAFFICSTACK(int num)
	{
	  data = NULL;
		currentNumber = 0;

		maxNumber = num;
		data = new int[maxNumber];
	}

	~TRAFFICSTACK()
	{
		if(data != NULL) delete data;
	}

	void push(int num)
	{
		if(currentNumber >= maxNumber)
		{
			currentNumber = 0;
		}
		data[currentNumber++] = num;
	}

#ifdef DEBUG
	void show()
	{
		printf("Stack info. currentNumber = %d, maxNumber = %d\n", currentNumber, maxNumber);
		for(int i = 0; i < maxNumber; i++)
		{
			printf("data[%d] = %d\n", i, data[i]);
		}
		printf("\n");
	}
#endif
};

#endif