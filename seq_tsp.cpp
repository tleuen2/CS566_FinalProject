#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iterator>
#include <vector>
#include <queue>
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <math.h>
#include <stdint.h>


using namespace std;

#define filename "Input.txt"
#define INF 3000000
#define MAXSIZE 28

struct city
{
	//EUD 2D format: (x, y) and distance are computed by SQRT( delta_X^2 + delay_Y^2)
	int index; 
	int x;
	int y;
};

struct Path
{
	int number_visit_city;
	int cost;
	int path[MAXSIZE];

	Path(){}

	//print out the final solution path + cost
	void toString() {
		for (int i = 0; i < MAXSIZE; i++)
			cout << path[i] << " -> ";
		cout << path[0] << " . The cost = " << cost << endl;
	}

	//initializtion
	void init()
	{
		for (int i = 0; i < MAXSIZE; i++)
			path[i] = -1;
		path[0] = 0;
		cost = 0;
		number_visit_city = 1;
	}

	void updateCostA()
	{
		if(number_visit_city > 1)
		{
			for(int i = 0; i < MAXSIZE - 1; i++)
			{
				if(path[i] != -1)
				{
					cost += Adj[path[i]][path[i + 1];
				}				
			}
		}
	}

	//check if a city has been visited (in the current solution)
	bool visited(int index)
	{
		for (int i = 0; i < number_visit_city; i++)
			if (path[i] == index)
				return true;
		return false;
	}

	bool is_solution() //whether or not all cities are visited, if yes, this is the solution, if no, not a complete solution
	{
		for (int i = 0; i < MAXSIZE; i++)
			if (path[i] == -1)
				return false;
			return cost != -1;
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

int prims(int **A, int n); //function of prim-algo for MST.
void start_partition_phase(int *partial_solution_size, int size_of_processors, int *size_input); // starting partiton_phase for parallel TSP.

int main(int argc, char *argv[]) {

	//int MAXSIZE = (int)argv[1];
	//use for transform EUD_2D format into adjecent matrix
	float adj_matrix[MAXSIZE][MAXSIZE];
	int size_input = 0;
	int city_index = 0;
	int x_index = 0;
	int y_index = 0;
	city *city; //store city info
	int rank;
	

	int j;

	//-----------------------------------------PART 1------------------------------------------//
	//-------------------------------------GRAPH FORMAT CONVERISON-----------------------------//
	//obtain the input city graph and transform into adjcent matrix format
	//step1: read input file with EUD_2D format
	//step2: transform into adjcent matrix format
	
	//read file;
	FILE *fp;
	char line[100];
	const char *delim = " "; //to seperate the information from one line into different sub-string
	char *string; // store the current line from reading the input file
	char *token; //the sub-string with splited by the space " ". e.g., "1 2 3" -> "1", "2", "3"

	//if file does not exist, warning and exit
	if ((fp = fopen("Input.txt", "r")) == NULL) {
		printf("\nCannot open file strike any key exit!");
		exit(1);
	}

	//get # of input size from the input file
	while (1) {
		fgets(line, 100, fp);
		if (strstr(line, "DIMENSION")) { // the line shows the # of cities which is input size
			char *pt = strstr(line, ":");
			size_input = atof(pt + 1);
			break;
		}
	}

	city = (struct city*)malloc(sizeof(struct city) * size_input);
	int temp[3]; //temp int array store the info of a city with : index, x_index, y_index
	int num; //change the type string in a line from input file to int number

	//generate the input data for all cities to create the adjcent matrix
	//starting with the label line of the node's info
	while (1) {
		fgets(line, 100, fp);
		if (strstr(line, "NODE_COORD_SECTION")) {
			break;
		}
	}

	//read and write each city's index, x, y position info
	for (city_index = 0; city_index < size_input; ++city_index){
		j = 0; //temp array index, initialize to 0
		string = fgets(line + 1, 100000, fp); //obtain one city info line
		token = strtok(string, delim);
		while (token != NULL) {
			//read each element (token)
			num = atoi(token);
			temp[j] = num; //store into temp array
			j++;
			token = strtok(NULL, delim); //the current token is changed to NULL de-initialize
		}
		city[city_index].index = temp[0];
		city[city_index].x = temp[1];
		city[city_index].y = temp[2];
	}
	fclose(fp); //close input file
	
	//NOW, step2: create the adjcent matrix
	//CITY i to j distance = sqrt[(Xi-Xj)^2 + (Yi-Yj)^2]

	for (x_index = 0; x_index < size_input; ++x_index) {
		for (y_index = 0; y_index < size_input; ++y_index) {
			if (x_index == y_index) { //for the same city, distance = INF
				adj_matrix[x_index][y_index] = INF;
			}
			else { //for different city, can compute the distance by the above equation
				adj_matrix[x_index][y_index] = (float)sqrt(pow((double)(city[x_index].x - city[y_index].x), 2.0) + pow((double)(city[x_index].y - city[y_index].y), 2.0));
			}
		}
	}
	// end of creating adjcent matrix (note, actual city 1 has index = 0 (index starting at 0)
	// now adj_matrix is the adjcent matrix for the distance (cost) of different cities (convert to the same input format that Owen's program used)

	/*
	for (int i = 0; i < size_input; i++)
	{
		for (int j = 0; j < size_input; j++)
			cout << adj_matrix[i][j] << " ";
		cout << endl;
	}
	*/



	//-----------------------------------------PART 2------------------------------------------//
	//-------------------------------------SEQ TSB W/ B&B using PRIM (MST) for LB estimation---//
	//SEQ-TSP w/ B&B, where the lower-bound cost is comuputed by using MST (minimum spanning tree) by PRIM algo.

	priority_queue<Path, vector<Path>, NodeCompare> pq; 

	Path temp_solution; //the temp solution is stored here (temp_solution is assuming starting from city 1 (index in matrix = 0))
	temp_solution.init(); //initilzation
	
	//cout << "temp cost: " << temp_solution.cost << endl;

	// using priority queue (increasing order)
	// so that, we only explore the partial solution with the smallest cost so far.
	pq.push(temp_solution);
	

	// the initial version of best solution:
	// assume it is 0 ->1->2->3->...n-2->n-1->0 

	Path best_solution; //the overall best solution.
	best_solution.init();

	best_solution.number_visit_city = MAXSIZE;
	best_solution.cost = best_solution.path[0] = 0;

	// this for loop compute the cost for 0 -> 1 ->2 -> ... n-1
	for (int i = 0; i < MAXSIZE-1; i++)
	{
		best_solution.path[i] = i; //the order of visited for each city
		best_solution.cost += adj_matrix[i][i + 1]; //adding the cost for city i -> i+1 (i+1 is up to n-1)
	}
	best_solution.cost += adj_matrix[MAXSIZE - 1][0]; //adding the cost for (n-1 -> 0)

	//cout << "best cost: " << best_solution.cost << endl;

	// Now, starting the TSP B&B processing

	while (!pq.empty()) // if there still exists some partial solution are not explored complete, keep check the result
	{
		Path current_solution = pq.top();  //everytime, only explore the partial solution with the smallest cost so far

		pq.pop();
		//pop out the smallest cost solution so far, 
		//and after adding the next sub-path (next city),
		//need to push this updated solution back to the queue and update the priority (cost based order)

		if (current_solution.cost >= best_solution.cost) // if the cost is greater than the best solution (so far) cost, prune it directly.
			continue;

		//check current solution if it is better than the current best solution so far, update the best solution.
		if (current_solution.is_solution()) // cost = -1 means, this solution is not feasible (cannot back to the starting city) 
		{
			//for (int i = 0; i < MAXSIZE; i++)
			//	cout << current_solution.path[i] << endl;
			best_solution = current_solution;
			//cout << "best cost: " << best_solution.cost << endl;
			continue;
		}

		if (!current_solution.is_solution()) //explore the partial solution (this partial solution has less cost than the best solution, pruned by the above if statement).
		{
			// now visit all the cities that can be visited (if the graph is not a complete graph) and also, has not been visited yet (skip the visited cities, not repeat path) :
			for (int i = 0; i < MAXSIZE; i++)
				if (adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i] != INF && !current_solution.visited(i))	// check if can be visited AND is an unvisited city
				{
					int lowerBoundEstimation, costTemp = 0;	// LB cost, temp path cost

					// compute the current cost so far (for the partial solution).
					for (int pathi = 0; pathi < current_solution.number_visit_city - 1; pathi++)
						costTemp += adj_matrix[current_solution.path[pathi]][current_solution.path[pathi + 1]];

					// add the next city based on the index i
					costTemp += adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i];

					//Now, based on this new city (i), check the MST cost of the remaining part which is the lower-bound cost (but may not be able to reach)
					vector<int> primsVertices;	// The structure for computing MST using PRIM algo.

					for (int city = 0; city < MAXSIZE; city++)	// get all cities that will be involved into MST (in the sub-graph)
						if (city == 0 || city == i || !current_solution.visited(city))	// if the city has not been visited, or the starting city (need to add the path/cost back to the starting city)
							primsVertices.push_back(city);
					int vprims = primsVertices.size();	// # of cities in the MST

					//do PRIM to check the LB:
					// obtain the sub-graph (adj-matrix) for the remaining cities (>1 cities are remaining) to compute the PRIM more efficient (not visit the entire graph)
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
									temp[x][y] = temp[y][x] = adj_matrix[primsVertices[x]][primsVertices[y]];	// symmetry matrix

						//DO PRIM to find MST of the sub-matrix (sub-graph) which is the LB cost.
						lowerBoundEstimation = prims(temp, vprims); //MST: PRIM algo.
						for (int iter = 0; iter < vprims; iter++)
							delete[] temp[iter];
						delete[] temp;
					}
					else //just in case if only 1 city here, the cases is i = 0, where the starting city is the only one remaining (i must be 0, otherwise, the size of vprims = 2 at least).
						lowerBoundEstimation = 0; //thus, no LB cost (starting city to starting city).

					//Now, after finding the LB cost, update the estimated LB cost to the current cost so that, can have an esitmation of how good the current solution is.
					int costEstimation = costTemp + lowerBoundEstimation;
					// Code for lb ends

					// decide whether or not keep this updated current solution (compared to the best solution so far)
					if (costEstimation < best_solution.cost) // if the current solution is good, then, keep it
					{
						//get a copy of the current solution and update path info, cost info, and # of cities have been visited info.
						Path child;
						child.init();
						std::copy(current_solution.path, current_solution.path + MAXSIZE, child.path);
						child.cost = costEstimation;
						child.path[current_solution.number_visit_city] = i;
						child.number_visit_city = current_solution.number_visit_city + 1;

						// if all nodes have been visited, but the last city cannot go back to the starting city, this is not the complete solution and set the cost = -1 as a flag.
						//
						if (child.number_visit_city == MAXSIZE)
							if (adj_matrix[0][child.path[MAXSIZE - 1]] == INF)
								child.cost = -1; //a flag that this solution is not feasible and will be removed.
						pq.push(child); //push back into priority_queue for futher update (partial solution) or update best solution (complete solution)
					}
				}
		}


	}

	cout << "Ver  : " << MAXSIZE << endl;					// graph size
	
	cout << "Sol  : ";
	best_solution.toString();
	cout << " Best Cost Variable: " << best_solution.cost << endl;	// solution path and total cost

	getchar();
	return 0;

}

//The idea of PRIM is simple.
//Based on the current visited node so far, 
//span the tree structure by adding the smallest cost arc from the visited nodes group to the un-visited nodes group
//and thus, visited more nodes, until all nodes are visited
int prims(int **A, int n) //MST Prim algo.
{
	if (n == 1) //only one node in the graph, exit
		return 0;

	int i, j, numberOfEdges = 1, min, totalCost = 0, y;
	bool *visited = new bool[n]; //visited list
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
						if (min > A[i][j]) //find the smallest cost edge (or arc for directed graph) and add it to the visited list.
						{
							min = A[i][j];
							y = j;
						}
		visited[y] = true;
		totalCost = totalCost + min; //then, update cost.
		numberOfEdges++; //update # of edge, edge = # of node - 1 for a tree.
	}
	return totalCost;
}

// the idea of start_partition_phase is (used in Owen's code)
// need obtain enough partial solution for parallel version
// so that, each processor can have their own starting point
// in other words, each processor has their own sub-solution space to be explored
// note that, it is possible that, some processor will have more initial solutions (based on the way of implementation)
// 
// e.g., for cities = 16, two initial cities are known as 0->1 for the first processor 
// then, this partial solution is path{0, 1} for this processor
// the order inside is the order of visiting cities
// similarly, for the second processor, the two cities are 0->2
// the corresponding partial solution is path{0, 2} 


	//-----------------------------------------PART 3------------------------------------------//
	//-------------------------------------udea of starting partition_phase for parallel TSP---//

void start_partition_phase(int *partial_solution_size, int size_of_processors, int *size_input,
							int *MAXSIZE, int rank, priority_queue<Path, vector<Path>, NodeCompare>  *myQueue)
{
	priority_queue<Path, vector<Path>, NodeCompare> tempBuffer1;
	priority_queue<Path, vector<Path>, NodeCompare> tempBuffer2;
	priority_queue<Path, vector<Path>, NodeCompare>  *tempInput;
	priority_queue<Path, vector<Path>, NodeCompare>  *tempOutput;
	int totalCount = 0;
	bool Buffer1 = true;
	//goal: find out how many cities should be had in the partial solution so that:
    // # of partial solution > # of processors
	// NOTE: need to check if the # of partial solution (a function of size_input) is less than the size of processors
	// in other words, the input graph size is too small, or the # of processors is unnecessarly large
	int number_of_partial_solution = 0;
	partial_solution_size = 0;
	while (number_of_partial_solution < size_of_processors && partial_solution_size < size_input) //16proc20city
	{
		number_of_partial_solution *= (size_input - partial_solution_size - 1);
		partial_solution_size++;
	}

	// Get the intial paths and add them to the temporary queue
	for(int k = 0; k < MAXSIZE; k++)
	{
		Path temp;
		temp.init();

		temp.path[temp.number_visit_city - 1] = k;

		tempBuffer1.push(temp);
		totalCount++; 
	}

	// Generate the partial paths
	while(totalCount < number_of_partial_solution)
	{
		if(Buffer1)
		{
			tempInput = &tempBuffer1;
			tempOutput = &tempBuffer2;
		}
		else
		{
			tempInput = &tempBuffer2;
			tempOutput = &tempBuffer1;
		}
		int tempInputSize = tempInput.size();
		for(int i = 0; i < tempInputSize; i++)		
		{
			Path node = tempInput.top();
			tempInput.pop();

			for(int j = 0; j < MAXSIZE; j++)
			{
				if(!node.visited[j])
				{
					//get a copy of the current solution and update path info, cost info, and # of cities have been visited info.
					Path newNode;
					newNode.init();
					std::copy(node.path, node.path + MAXSIZE, newNode.path);
					newNode.cost = node.cost;
					newNode.path[node.number_visit_city] = j;
					newNode.number_visit_city = node.number_visit_city + 1;
					newNode.updateCost();
					tempOutput.push(newNode);
				}
			}
		}
		Buffer1 = !Buffer1;
	}

	// Give all of the processors their intial set of partial solutions. 
	for(int w = 0; w < number_of_partial_solution; w++)
	{
		if(Buffer1)
		{
			tempInput = &tempBuffer1;
			tempOutput = &tempBuffer2;
		}
		else
		{
			tempInput = &tempBuffer2;
			tempOutput = &tempBuffer1;
		}

		if((w % rank) == 0)
		{
			Path outNode = tempInput.top();
			tempInput.pop();

			std::copy(node.path, node.path + MAXSIZE, newNode.path);

			*myQueue.push(outNode);
		}
	}

	// Now, number_of_partial_solution is the total number of initial solution will be
	// also, partial_solution_size is the # of cities in each partial solution

	//after known how many cities should be in the partial solution at the beginning, each processor will start to obtain their own initial solutions
	// you can now implement your own way to do the partition processing
}