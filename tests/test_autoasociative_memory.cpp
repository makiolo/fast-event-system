#include <iostream>
#include <fast-event-system/fes.h>

#if 0

const float velocity = 0.01f;
const int NUM_NEURONS = 3;
fes::async_fast<float> neuron[NUM_NEURONS];	
bool learn = true;

int main2()
{
	for(int i=0; i<NUM_NEURONS; ++i)
	{
		for(int j=0; j<NUM_NEURONS; ++j)
		{
			// 0 = todo fluye, eres joven
			if(i==j)
			{
				neuron[i].set_weight(j, 1.0f);
				neuron[j].set_weight(i, 1.0f);
			}
			else
			{
				neuron[i].set_weight(j, 0.0f);
				neuron[j].set_weight(i, 0.0f);
			}
		}
	}
	
	// conexion dinamica
	neuron[0].connect([&](float data) {
			
		std::cout << "recibido en a: " << data << std::endl;
	
		if(learn)
		{
			if(data > neuron[1].get_weight(0))
			{
				neuron[1].set_weight(0, neuron[1].get_weight(0) + velocity );
				neuron[1](data);
			}

			if(data > neuron[2].get_weight(0))
			{
				neuron[2].set_weight(0, neuron[2].get_weight(0) + velocity );
				neuron[2](data);
			}
		}
	});
	neuron[1].connect([&](float data) {

		std::cout << "recibido en b: " << data << std::endl;

		if(learn)
		{
			if(data > neuron[0].get_weight(1))
			{
				neuron[0].set_weight(1, neuron[0].get_weight(1) + velocity );
				neuron[0](data);
			}

			if(data > neuron[2].get_weight(1))
			{
				neuron[2].set_weight(1, neuron[2].get_weight(1) + velocity );
				neuron[2](data);
			}
		}
	});
	neuron[2].connect([&](float data) {

		std::cout << "recibido en c: " << data << std::endl;

		if(learn)
		{
			if(data > neuron[0].get_weight(2))
			{
				neuron[0].set_weight(2, neuron[0].get_weight(2) + velocity );
				neuron[0](data);
			}
			
			if(data > neuron[1].get_weight(2))
			{
				neuron[1].set_weight(2, neuron[1].get_weight(2) + velocity );
				neuron[1](data);
			}
		}
	});

	float sol1 = 1;
	float sol2 = 1;
	float sol3 = 1;

	neuron[0](sol1);
	neuron[1](sol2);
	neuron[2](sol3);
	
	for(int j=0; j<100000; ++j)
	{
		for(int i=0; i<3; ++i)
		{
			neuron[i].update();
		}
	}
	for(int i=0; i<3; ++i)
	{
		std::cout << "size " << i << " = " << neuron[i].size() << std::endl;
		for(int j=0; j<3; ++j)
		{
			std::cout << "weight " << j << " = " << neuron[i].get_weight(j) << std::endl;
		}
	}

	sol1 = 0;
	sol2 = 0;
	sol3 = 1;

	neuron[0](sol1);
	neuron[1](sol2);
	neuron[2](sol3);
	
	for(int j=0; j<100000; ++j)
	{
		for(int i=0; i<3; ++i)
		{
			neuron[i].update();
		}
	}
	for(int i=0; i<3; ++i)
	{
		std::cout << "size " << i << " = " << neuron[i].size() << std::endl;
		for(int j=0; j<3; ++j)
		{
			std::cout << "weight " << j << " = " << neuron[i].get_weight(j) << std::endl;
		}
	}

	sol1 = 0;
	sol2 = 1;
	sol3 = 0;

	neuron[0](sol1);
	neuron[1](sol2);
	neuron[2](sol3);
	
	for(int j=0; j<100000; ++j)
	{
		for(int i=0; i<3; ++i)
		{
			neuron[i].update();
		}
	}
	for(int i=0; i<3; ++i)
	{
		std::cout << "size " << i << " = " << neuron[i].size() << std::endl;
		for(int j=0; j<3; ++j)
		{
			std::cout << "weight " << j << " = " << neuron[i].get_weight(j) << std::endl;
		}
	}

	sol1 = 1;
	sol2 = 0;
	sol3 = 0;
	
	neuron[0](sol1);
	neuron[1](sol2);
	neuron[2](sol3);
	
	for(int j=0; j<100000; ++j)
	{
		for(int i=0; i<3; ++i)
		{
			neuron[i].update();
		}
	}
	for(int i=0; i<3; ++i)
	{
		std::cout << "size " << i << " = " << neuron[i].size() << std::endl;
		for(int j=0; j<3; ++j)
		{
			std::cout << "weight " << j << " = " << neuron[i].get_weight(j) << std::endl;
		}
	}
	
	std::cout << "-------------- end learn------------" << std::endl;

	std::cout << " " << (0 * neuron[2].get_weight(0)) + (0 * neuron[2].get_weight(1))	<< std::endl;
	std::cout << " " << (0 * neuron[2].get_weight(0)) + (1 * neuron[2].get_weight(1))	<< std::endl;
	std::cout << " " << (1 * neuron[2].get_weight(0)) + (0 * neuron[2].get_weight(1))	<< std::endl;
	std::cout << " " << (1 * neuron[2].get_weight(0)) + (1 * neuron[2].get_weight(1))	<< std::endl;
	
	return 0;	
}
#endif

#include <stdio.h>
#include <math.h> 
#include <stdlib.h>

class neuron
{	
public:
	int weightv[4];
	neuron() {};
	
	neuron(int *j)
	{
		int i;
		for(i=0;i<4;i++)
		{
			weightv[i] = *(j+i);
		}
	}

	int update_threshld(int *x)
	{
		int i;
		int a=0;

		for(i=0;i<4;i++)
		{
			a += x[i] * weightv[i];
		}
		activation = a;
		if(activation > 0)
		{
			return (1);
		}
		else // if(activation < 0)
		{
			return (0);
		}
	}

protected:
	int activation;
	friend class network;
};

class network
{
public:
	network(int a[4],int b[4],int c[4],int d[4])
	{
		nrn[0] = ( neuron(a) );
		nrn[1] = ( neuron(b) );
		nrn[2] = ( neuron(c) );
		nrn[3] = ( neuron(d) );
	}
	
	bool test(int *patrn)
	{
		int i;
		
		for(i=0;i<4;i++)
		{
			output[i] = nrn[i].update_threshld(patrn);
		}
	
		bool matched = true;
		for(i=0;i<4;i++)
		{
			if (output[i] != patrn[i])
			{
				matched = false;
				break;
			}
		}
		return matched;
	}
public:
	neuron nrn[4];
	int output[4];
};

int main ()
{
	int patrn1[]= {1,0,1,0};
	int wt1[]= {0,-2,2,-2};
	int wt2[]= {-2,0,-2,2};
	int wt3[]= {2,-2,0,-2};
	int wt4[]= {-2,2,-2,0};

	network h1(wt1,wt2,wt3,wt4);
	printf("Pattern for 1010 :\n");
	if(h1.test(patrn1))
	{
		printf("Matched pattern\n");
	}
	else
	{
		printf("Unmatched pattern\n");
	}
	printf("\n");

	int patrn2[]= {0,1,0,1};
	printf("Pattern for 0101 :\n");
	if(h1.test(patrn2))
	{
		printf("Matched pattern\n");
	}
	else
	{
		printf("Unmatched pattern\n");
	}
	printf("\n");

	return 0;
}

