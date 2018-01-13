#include "mpiWrapper.h"
using namespace mpi;

double mpi::wtime()
{
	return MPI_Wtime();
}

void mpi::init(int* argc, char*** argv)
{
	MPI_Init(argc, argv);
}

void mpi::finalize()
{
	MPI_Finalize();
}

void mpi::barrier(MPI_Comm comm)
{
	MPI_Barrier(comm);
}

int mpi::getSize(MPI_Comm comm)
{
	int _size;
	MPI_Comm_size(comm, &_size);
	return _size;
}

int mpi::getRank(MPI_Comm comm)
{
	int _rank;
	MPI_Comm_rank(comm, &_rank);
	return _rank;
}
