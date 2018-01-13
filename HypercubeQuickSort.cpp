
#include <exception>
#include "HQSManager.h"
#include <iostream>
#include "random.h"

using namespace mpi;

void printStartFinishElements(mpi::shArray<int> &data, int coutForPrint)
{
	std::cout << "First " << coutForPrint << ":" <<  std::endl;
	for (int i = 0; i < coutForPrint; i++)
	{
		std::cout << data[i] << " ";
	}

	std::cout << std::endl;

	std::cout << "Last " << coutForPrint << ":" << std::endl;
	for (int i = data.size() - coutForPrint - 1; i < data.size() - 1; i++)
	{
		std::cout << data[i] << " ";
	}
}

void main(int argc, char** argv)
{

	mpi::init(&argc, &argv);
	auto rank = mpi::getRank(MPI_COMM_WORLD);
	auto ProcCount = mpi::getSize(MPI_COMM_WORLD);

	mpi::shArray<int> data(1000000);
	int coutForPrint = 100;

	if (rank == 0) {

		mpi::random::generate(std::begin(data), std::end(data), -1000, 1000);


		if (data.size() > 100)
		{
			std::cout << "Generated array with size: " << data.size() << std::endl;
			std::cout << "Elements for not sorted data:" << std::endl;
			printStartFinishElements(data, coutForPrint);
		}

		if (ProcCount > 1)
			std::cout << "\nStarting parallel sort with " << ProcCount << " processes\n";
		else
			std::cout << "\nStarting sequential sort (std::sort)\n";
	}

	if (ProcCount > 1) {

		int experimentCount = 1;

		double time = mpi::HQSManager<int>::experiment(data, experimentCount);

		if (rank == 0)
			printf("\n Exec time = %3f \n", time);

	} else {

		clock_t begin = clock();
		std::sort(std::begin(data), std::end(data));
		clock_t end = clock();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		printf("\n Exec time = %3f \n", elapsed_secs);

	}

	if (rank == 0) 
	{
		std::cout << "Elements for Sorted data:" << std::endl;
		printStartFinishElements(data, coutForPrint);
	}
	std::cout << std::endl;

	mpi::finalize();
}
