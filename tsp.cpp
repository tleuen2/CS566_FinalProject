// File: tsp.cpp
// Author: Akshay Patil(http://www.akshaypatil.in), Rose Mary George
// Description: This file is an implementation of Travelling Salesman Problem with parallel programming using MPI.
// The code is written in C++ however only C style functions for MPI are used
// Flag -DMPICH_SKIP_MPICXX is used to disable C++ version of MPI during compilation in Makefile

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iterator>
#include <vector>
#include <queue>
#include <cmath>
#include <ctime>
#include "mpi.h"

#define GRAPHSIZE  17
#define INF 9999
//#define GRAPHSIZE  53
//#define INF 9999999
//#define GRAPHSIZE  33
//#define INF 100000000

using std::fstream;
using std::ios;
using std::string;
using std::istream_iterator;
using std::stringstream;
using std::priority_queue;
using std::vector;
using std::cout;
using std::endl;

int Adj[GRAPHSIZE][GRAPHSIZE];	// the input matrix

struct Path					// contains the partial solution
{
	int numberOfVisitedNode;// # of nodes have been visited
	int cost;				// cost of current solution
	int path[GRAPHSIZE];	// solution path
	Path() {}
	void toString()			// generate the well-written optimal solution of the final path
	{
		cout << "[";
		for (int i = 0; i < GRAPHSIZE - 1; i++)
			cout << path[i] << " ";
		cout << path[GRAPHSIZE - 1] << " 0] " << cost;
	}
	void init()				// initialization
	{
		for (int i = 0; i < GRAPHSIZE; i++)
			path[i] = -1;
		path[0] = 0;
		cost = 0;
		numberOfVisitedNode = 1;
	}
	bool hasVisited(int vertex)	// check whether the given node has been visited, return true if yes, false if no
	{
		for (int i = 0; i < numberOfVisitedNode; i++)
			if (path[i] == vertex)
				return true;
		return false;
	}
	bool isSolution()			// check whether the array "path" contains a solution, return true if yes, false if no
	{
		for (int i = 0; i < GRAPHSIZE; i++)
			if (path[i] == -1)
				return false;
		return cost != -1;
	}

	bool allVisited()			// check whether all nodes have been visited, return true if yes, false if no
	{
		for (int i = 0; i < GRAPHSIZE; i++)
			if (path[i] == -1)
				return false;
		return true;
	}
};

class NodeCompare				// compare the cost of the two given nodes, return yes if the former one is greater than the latter one
{
public:
	bool operator()(const Path& n1, const Path& n2) const
	{
		return n1.cost > n2.cost;
	}
};

void readInputFile(char *argv);
int prims(int **A, int n);
void pathIncrement(int *path, int *intialLevel);
//void printMatrix();				// print the input matrix for debugging purposes

int main(int argc, char **argv)
{
	int rank, size, master;
	double totalStart, totalEnd, distStart, distEnd;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	readInputFile(argv[1]);
	//printMatrix();
	totalStart = MPI_Wtime();
	MPI_Barrier(MPI_COMM_WORLD);	// block until all processes in the communicator MPI_COMM_WORLD have reached this routine
	// make datatype for path
	Path sample;						// the initial partial solution
	sample.init();
	MPI_Datatype MPI_Path;			// the new data type
	MPI_Datatype type[3] = {MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint disp[3];
	// Displacement from the root
	disp[0] = int(&sample.numberOfVisitedNode) - int(&sample);	// the memory location w.r.t structure root to store varibale numberOfVisitedNode
	disp[1] = int(&sample.cost) - int(&sample);					// the memory location w.r.t structure root to store varibale cost
	disp[2] = int(&sample.path[0]) - int(&sample);				// the memory location w.r.t structure root to store array path[]
	int blockLength[3] =
	{
		1,			// numberOfVisitedNode
		1,			// cost
		GRAPHSIZE	// input matrix
	};
	MPI_Type_create_struct(3,			// [in] number of blocks (integer), also number of entries in the next three parameters
						   blockLength,	// [in] number of elements in each block (array of integer)
						   disp,		// [in] byte displacement of each block (array of integer)
						   type,		// [in] type of elements in each block (array of handles to datatype objects)
						   &MPI_Path	// [out] new datatype (handle)
						  );			// create an MPI datatype from a general set of datatypes, displacements, and block sizes
	MPI_Type_commit(&MPI_Path);			// commit the data type
	// define initial best solution
	Path bestSolution;		// global best solution
	bestSolution.numberOfVisitedNode = GRAPHSIZE;
	bestSolution.cost = bestSolution.path[0] = 0;
	for (int i = 1; i < GRAPHSIZE; i++)
	{
		bestSolution.path[i] = i;
		bestSolution.cost += Adj[i - 1][i];
	}
	bestSolution.cost += Adj[GRAPHSIZE - 1][0];
	//cout << bestSolution.cost << endl;	// intial best solution debugging passed
	if (size > 1)
	{
		// randomly generate the master processor
		srand(time(NULL));
		master = rand() % size;
		// intial data partitioning
		distStart = MPI_Wtime();
		int setPartialSolutionSize = size;	// set the intial input data partition size
		int intialLevel = 0, partialSolutionSize = 1, count = 0;
		while (partialSolutionSize <= setPartialSolutionSize && intialLevel < GRAPHSIZE)	// determine the level of intial partial solution
		{
			partialSolutionSize *= (GRAPHSIZE - intialLevel - 1);
			intialLevel++;
		}
		Path solution;
		solution.init();
		for (int i = 1; i <= intialLevel; i++)	// initialize the partial solution path
			solution.path[i] = i;
		solution.path[intialLevel]--;
		if (rank == master)		// master command
		{
			while (count != size)	// after the interations, solution contains the immediate next partial solution path, which will be sent to one of the slaves
			{
				pathIncrement(solution.path, &intialLevel);
				count++;
			}
			for (int i = 1; i <= intialLevel; i++)
				solution.cost += Adj[solution.path[i - 1]][solution.path[i]];
			solution.numberOfVisitedNode += intialLevel;
		}
		else					// slave command
		{
			// initial input data partitioning
			if (rank < master)
			{
				while (count != rank + 1)	// after the interations, solution contains the immediate next partial solution path, which will be sent to one of the slaves
				{
					pathIncrement(solution.path, &intialLevel);
					count++;
				}
			}
			else
			{
				while (count != rank)	// after the interations, solution contains the immediate next partial solution path, which will be sent to one of the slaves
				{
					pathIncrement(solution.path, &intialLevel);
					count++;
				}
			}
			for (int i = 1; i <= intialLevel; i++)
				solution.cost += Adj[solution.path[i - 1]][solution.path[i]];
			solution.numberOfVisitedNode += intialLevel;
			//MPI_Send(&solution, 1, MPI_Path, master, 100, MPI_COMM_WORLD);
		}
		/*if (rank == master)	// initial data partitioning debugging passed
		{
		Path *slaveSolutions = new Path[size - 1];
		MPI_Status status;
		for (int i = 0; i < size - 1; i++)
		if (i < master)
		MPI_Recv(slaveSolutions + i, 1, MPI_Path, i, 100, MPI_COMM_WORLD, &status);
		else
		MPI_Recv(slaveSolutions + i, 1, MPI_Path, i + 1, 100, MPI_COMM_WORLD, &status);
		cout << "Master: " << master << endl;
		for (int i = 0; i < size - 1; i++)
		{
		if (i<master)
		cout << "Process " << i << " Initial Partial Solution: ";
		else
		cout << "Process " << i + 1 << " Initial Partial Solution: ";
		slaveSolutions[i].toString();
		cout << endl;
		}
		delete[]slaveSolutions;
		}*/
		// local processing
		distEnd = MPI_Wtime();
		if (rank != master)	// for all slaves
		{
			priority_queue<Path, vector<Path>, NodeCompare> Q, Q_temp;
			Path pathTemp;
			Q.push(solution);
			Path* receivednodes = new Path[size - 1];
			MPI_Status status_solutionbroadcast;
			MPI_Request solution_req;
			int received_flag;
			while (!Q.empty())
			{
doAgain:
				Path node = Q.top();
				Q.pop();
				/*if (size > 1)	// deal with the broadcasted local best solution seen so far by individual processors
				{
				MPI_Iprobe(MPI_ANY_SOURCE, 50, MPI_COMM_WORLD, &received_flag, MPI_STATUS_IGNORE);
				if (received_flag)
				{
				MPI_Irecv(receivednodes, size - 1, MPI_Path, MPI_ANY_SOURCE, 50, MPI_COMM_WORLD, &solution_req);
				int receivedsolutionnodes;
				MPI_Wait(&solution_req, &status_solutionbroadcast);
				MPI_Get_count(&status_solutionbroadcast, MPI_Path, &receivedsolutionnodes);
				for (int i = 0; i < receivedsolutionnodes; i++)	// deal with the newly received local best solutions
				if (receivednodes[i].cost < bestSolution.cost)
				bestSolution = receivednodes[i];
				}
				}*/
				if (node.cost >= bestSolution.cost)	// prune the bad (partial) solution
					continue;
				if (node.isSolution())	// if the path is a solution, see if it is better than the global best solution
				{
					bestSolution = node;
					/*MPI_Request req;
					for (int i = 0; i < size; i++)	// broadcast
					if (i != rank)
					MPI_Isend(&node, 1, MPI_Path, i, 50, MPI_COMM_WORLD, &req);*/
					continue;
				}
				if (!node.isSolution())	// continue exploring if it is not a solution
					for (int i = 0; i < GRAPHSIZE; i++)	// search all nodes
						if (Adj[node.path[node.numberOfVisitedNode - 1]][i] != INF && !node.hasVisited(i))	// for an unvisited node
						{
							int lowerBoundEstimation, costTemp = 0;	// path cost is a temp
							for (int pathi = 0; pathi < node.numberOfVisitedNode - 1; pathi++)
								costTemp += Adj[node.path[pathi]][node.path[pathi + 1]];
							costTemp += Adj[node.path[node.numberOfVisitedNode - 1]][i];
							vector<int> primsVertices;	// set of all nodes in the MST
							for (int city = 0; city < GRAPHSIZE; city++)	// get all nodes that will be involved into MST
								if (city == 0 || city == i || !node.hasVisited(city))	// if the city has not been visited
									primsVertices.push_back(city);
							int vprims = primsVertices.size();	// # of nodes in the MST
							if (vprims > 1)
							{
								int **temp = new int*[vprims];
								for (int iter = 0; iter < vprims; iter++)
									temp[iter] = new int[vprims];
								// define sub-matrix in the MST
								for (int x = 0; x < vprims; x++)
									for (int y = 0; y < vprims; y++)
										if (x == y)
											temp[x][y] = 0;
										else
											temp[x][y] = temp[y][x] = Adj[primsVertices[x]][primsVertices[y]];	// symmetry matrix
								lowerBoundEstimation = prims(temp, vprims);
								for (int iter = 0; iter < vprims; iter++)
									delete[] temp[iter];
								delete[] temp;
							}
							else
								lowerBoundEstimation = 0;
							int costEstimation = costTemp + lowerBoundEstimation;
							// Code for lb ends
							if (costEstimation < bestSolution.cost)
							{
								Path child;
								child.init();
								std::copy(node.path, node.path + GRAPHSIZE, child.path);
								child.cost = costEstimation;
								child.path[node.numberOfVisitedNode] = i;
								child.numberOfVisitedNode = node.numberOfVisitedNode + 1;
								if (child.numberOfVisitedNode == GRAPHSIZE)	// all nodes have been visited
									if (Adj[0][child.path[GRAPHSIZE - 1]] == INF)
										child.cost = -1;
								Q.push(child);
							}
						}
			}
			MPI_Send(&intialLevel, 1, MPI_INT, master, 4, MPI_COMM_WORLD);
			MPI_Recv(&pathTemp, 1, MPI_Path, master, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (pathTemp.cost != -1)
			{
				Q.push(pathTemp);
				goto doAgain;
			}
		}
		else
		{
			MPI_Status request_Status;
			while (count <= partialSolutionSize)
			{
				int temp;
				//MPI_Probe(MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &request_Status);
				MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &request_Status);
				MPI_Send(&solution, 1, MPI_Path, request_Status.MPI_SOURCE, 6, MPI_COMM_WORLD);
				pathIncrement(solution.path, &intialLevel);
				count++;
				solution.cost = 0;
				for (int i = 1; i <= intialLevel; i++)
					solution.cost += Adj[solution.path[i - 1]][solution.path[i]];
			}
			solution.cost = -1;
			for (int processor = 0; processor < size; processor++)
				if (processor != master)
					MPI_Send(&solution, 1, MPI_Path, processor, 6, MPI_COMM_WORLD);
		}
		// termination phase
		Path* receivednodes = new Path[size - 1];
		if (rank != master)
			MPI_Send(&bestSolution, 1, MPI_Path, master, 50, MPI_COMM_WORLD);
		else
		{
			int termination = 0;
			while (termination != size - 1 && size > 1)
			{
				if (termination < master)
					MPI_Recv(receivednodes + termination, 1, MPI_Path, termination, 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				else
					MPI_Recv(receivednodes + termination, 1, MPI_Path, termination + 1, 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if (receivednodes[termination].cost < bestSolution.cost)
					bestSolution = receivednodes[termination];
				termination++;
			}
			totalEnd = MPI_Wtime();
			cout << "Ver  : " << GRAPHSIZE << endl;					// graph size
			cout << "n(P) : " << size << endl;						// # of processors
			cout << "Sol  : ";
			bestSolution.toString();
			cout << " Best Cost Variable: " << bestSolution.cost << endl;	// solution path and total cost
			cout << "Time : " << totalEnd - totalStart << " s." << endl;
			cout << "DistT: " << distEnd - distStart << " s." << endl;
		}
		delete[] receivednodes;
	}
	else	// sequential TSP with B&B
	{
		priority_queue<Path, vector<Path>, NodeCompare> Q;
		Path solution;
		solution.init();
		Q.push(solution);
		Path* receivednodes = new Path[size - 1];
		MPI_Status status_solutionbroadcast;
		MPI_Request solution_req;
		int received_flag;
		while (!Q.empty())
		{
			Path node = Q.top();
			Q.pop();
			/*if (size > 1)	// deal with the broadcasted local best solution seen so far by individual processors
			{
			MPI_Iprobe(MPI_ANY_SOURCE, 50, MPI_COMM_WORLD, &received_flag, MPI_STATUS_IGNORE);
			if (received_flag)
			{
			MPI_Irecv(receivednodes, size - 1, MPI_Path, MPI_ANY_SOURCE, 50, MPI_COMM_WORLD, &solution_req);
			int receivedsolutionnodes;
			MPI_Wait(&solution_req, &status_solutionbroadcast);
			MPI_Get_count(&status_solutionbroadcast, MPI_Path, &receivedsolutionnodes);
			for (int i = 0; i < receivedsolutionnodes; i++)	// deal with the newly received local best solutions
			if (receivednodes[i].cost < bestSolution.cost)
			bestSolution = receivednodes[i];
			}
			}*/
			if (node.cost >= bestSolution.cost)	// prune the bad (partial) solution
				continue;
			if (node.isSolution())	// if the path is a solution, see if it is better than the global best solution
			{
				bestSolution = node;
				/*MPI_Request req;
				for (int i = 0; i < size; i++)	// broadcast
				if (i != rank)
				MPI_Isend(&node, 1, MPI_Path, i, 50, MPI_COMM_WORLD, &req);*/
				continue;
			}
			if (!node.isSolution())	// continue exploring if it is not a solution
				for (int i = 0; i < GRAPHSIZE; i++)	// search all nodes
					if (Adj[node.path[node.numberOfVisitedNode - 1]][i] != INF && !node.hasVisited(i))	// for an unvisited node
					{
						int lowerBoundEstimation, costTemp = 0;	// path cost is a temp
						for (int pathi = 0; pathi < node.numberOfVisitedNode - 1; pathi++)
							costTemp += Adj[node.path[pathi]][node.path[pathi + 1]];
						costTemp += Adj[node.path[node.numberOfVisitedNode - 1]][i];
						vector<int> primsVertices;	// set of all nodes in the MST
						for (int city = 0; city < GRAPHSIZE; city++)	// get all nodes that will be involved into MST
							if (city == 0 || city == i || !node.hasVisited(city))	// if the city has not been visited
								primsVertices.push_back(city);
						int vprims = primsVertices.size();	// # of nodes in the MST
						if (vprims > 1)
						{
							int **temp = new int*[vprims];
							for (int iter = 0; iter < vprims; iter++)
								temp[iter] = new int[vprims];
							// define sub-matrix in the MST
							for (int x = 0; x < vprims; x++)
								for (int y = 0; y < vprims; y++)
									if (x == y)
										temp[x][y] = 0;
									else
										temp[x][y] = temp[y][x] = Adj[primsVertices[x]][primsVertices[y]];	// symmetry matrix
							lowerBoundEstimation = prims(temp, vprims);
							for (int iter = 0; iter < vprims; iter++)
								delete[] temp[iter];
							delete[] temp;
						}
						else
							lowerBoundEstimation = 0;
						int costEstimation = costTemp + lowerBoundEstimation;
						// Code for lb ends
						if (costEstimation < bestSolution.cost)
						{
							Path child;
							child.init();
							std::copy(node.path, node.path + GRAPHSIZE, child.path);
							child.cost = costEstimation;
							child.path[node.numberOfVisitedNode] = i;
							child.numberOfVisitedNode = node.numberOfVisitedNode + 1;
							if (child.numberOfVisitedNode == GRAPHSIZE)	// all nodes have been visited
								if (Adj[0][child.path[GRAPHSIZE - 1]] == INF)
									child.cost = -1;
							Q.push(child);
						}
					}
		}
		totalEnd = MPI_Wtime();
		cout << "Ver  : " << GRAPHSIZE << endl;					// graph size
		cout << "n(P) : " << size << endl;						// # of processors
		cout << "Sol  : ";
		bestSolution.toString();
		cout << " Best Cost Variable: " << bestSolution.cost << endl;	// solution path and total cost
		cout << "Time : " << totalEnd - totalStart << " s." << endl;
	}
	MPI_Finalize();
	return 0;
}

void readInputFile(char *argv)
{
	fstream fin(argv, fstream::in);	// read the input matrix file input.txt
	istream_iterator<int> iter(fin);		// read a successive input from the input file at a time
	int r = 0, c = 0;	// row index, column index
	while (true)		// read the input file
	{
		Adj[r][c] = *iter;
		iter++;
		c++;
		if (c == GRAPHSIZE)
		{
			c = 0;
			r++;
		}
		if (r == GRAPHSIZE)
			break;
	}
	fin.close();
	fin.clear();
}

int prims(int **A, int n)
{
	if (n == 1)
		return 0;
	int i, j, numberOfEdges = 1, min, totalCost = 0, y;
	bool *visited = new bool [n];
	for (i = 0; i < n; i++)
		visited[i] = false;
	visited[0] = true;
	while (numberOfEdges < n)
	{
		min = INF;
		for (i = 0; i < n; i++)			// trace the node that has been visited
			if (visited[i] == true)
				for (j = 0; j < n; j++)	// trace the node that has not been visited
					if (visited[j] == false)
						if (min > A[i][j])
						{
							min = A[i][j];
							y = j;
						}
		visited[y] = true;
		totalCost = totalCost + min;
		numberOfEdges ++;
	}
	return totalCost;
}

void pathIncrement(int *path, int *intialLevel)
{
	bool generateNext = true;
	while (generateNext)	// generate a new initial partial solution path
	{
		path[*intialLevel]++;
		generateNext = false;
		for (int i = *intialLevel; i > 0; i--)
			if (path[i] == GRAPHSIZE)
			{
				path[i] = 0;
				path[i - 1]++;
			}
		// check for whether there are duplicated nodes
		for (int j = 0; j < *intialLevel && !generateNext; j++)
			for (int k = j + 1; k <= *intialLevel; k++)
				if (path[j] == path[k])	// duplicated node occurs
				{
					generateNext = true;
					break;
				}
	}
	//cout << path[1] << " " << path[2] << endl;
}
/*void printMatrix()
{
	for (int i = 0; i < GRAPHSIZE; i++)
	{
		for (int j = 0; j < GRAPHSIZE; j++)
			cout << Adj[i][j] << "\t";
		cout << endl;
	}
	cout << endl;
}*/


