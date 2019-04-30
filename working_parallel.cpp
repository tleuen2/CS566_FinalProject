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
#include "mpi.h"
#include <unistd.h>
#include <cstring>


using namespace std;

#define filename "Input8.txt"
#define INF 3000000
#define MAXSIZE 8

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

    // Copy constructor
    Path(const Path& p)
    {
        number_visit_city = p.number_visit_city;
        cost = p.cost;
        std::copy(p.path, p.path + MAXSIZE, path);

    }

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


class NodeCompare               // compare the cost of the two given nodes, return yes if the former one is greater than the latter one
{
public:
    bool operator()(const Path& n1, const Path& n2) const
    {
        return n1.cost > n2.cost;
    }
};

int prims(int **A, int n); //function of prim-algo for MST.
void start_partition_phase(int partial_solution_size, int size_of_processors, int size_input, int rank,
                           priority_queue<Path, vector<Path>, NodeCompare>  *myQueue, float adj_matrix[MAXSIZE][MAXSIZE]);

int main(int argc, char *argv[])
{

    int rank, size;
    double totalStart, totalEnd, distStart, distEnd;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //MPI_Finalize();
    //MPI_Barrier(MPI_COMM_WORLD);
    // make datatype for path
    Path sample;                  // the initial partial solution
    sample.init();
    MPI_Datatype MPI_Path;       // the new data type
    MPI_Datatype type[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint disp[3];
    MPI_Aint base;

    // Displacement from the root
    MPI_Get_address(&sample, &base);
    MPI_Get_address(&sample.number_visit_city, &disp[0]);
    MPI_Get_address(&sample.cost, &(disp[1]));
    MPI_Get_address(&sample.path[0], &(disp[2]));

    for(int i = 0; i < 3; i++)
    {
        //disp[i] = MPI_Aint_diff(disp[i], base);
        disp[i] = (char*) disp[i] - (char*) base;
    }

    int blockLength[3] = {1,1, MAXSIZE};
    MPI_Type_create_struct(3,         // [in] number of blocks (integer), also number of entries in the next three parameters
                           blockLength,   // [in] number of elements in each block (array of integer)
                           disp,      // [in] byte displacement of each block (array of integer)
                           type,      // [in] type of elements in each block (array of handles to datatype objects)
                           &MPI_Path  // [out] new datatype (handle)
    );            // create an MPI datatype from a general set of datatypes, displacements, and block sizes
    MPI_Type_commit(&MPI_Path);          // commit the data type
    //int MAXSIZE = (int)argv[1];
    //use for transform EUD_2D format into adjecent matrix
    float adj_matrix[MAXSIZE][MAXSIZE];
    int size_input = 0;
    int city_index = 0;
    int x_index = 0;
    int y_index = 0;
    city *city; //store city info

    int j;

    // the initial version of best solution:
    // assume it is 0 ->1->2->3->...n-2->n-1->0

    //TSP common datastrcutures
    Path best_solution; //the overall best solution.
    best_solution.init();
    best_solution.number_visit_city = MAXSIZE;
    //best_solution.cost = best_solution.path[0] = 0;

    priority_queue<Path, vector<Path>, NodeCompare> pq;


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
    Path best_path_list[size];

    //if file does not exist, warning and exit
    if ((fp = fopen(filename, "r")) == NULL) {
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
    Path inputBuffer[size];
    Path outputBuffer[size];
    MPI_Request request[size];
    MPI_Status status[size];
    MPI_Request request100[size];
    MPI_Status status100[size];
    int tag100[size];
    int tag100Temp[size];
    bool shouldIWait = true;
    int zeroFlag[size];
    Path someonesSolution;
            for(int i=0; i<size; i++){
                if(rank!=i){
                    MPI_Iprobe(i, 0, MPI_COMM_WORLD, &zeroFlag[i], &status[i]);
                    if(zeroFlag[i] != 0){
                        someonesSolution.init();
                        printf("Rank %d expecting something from %d", rank, i);
                        if(MPI_Recv(&someonesSolution, 1, MPI_Path, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE)== MPI_SUCCESS){
                            printf("Process %d received number %d from process %d\n",rank,someonesSolution.cost, i);
                            if(someonesSolution.cost<best_solution.cost){
                                best_solution.cost = someonesSolution.cost;
                                best_solution.number_visit_city = someonesSolution.number_visit_city;
                                for(int j=0; j<MAXSIZE; j++){
                                    best_solution.path[j] = someonesSolution.path[j];
                                }
                            }
                            zeroFlag[i] = 0;
                        }else{
                            printf("Error making TAG0 receive probe for processor %d\n",i);
                        }
                    }
                }
            }

            if (current_solution.cost >= best_solution.cost) // if the cost is greater than the best solution (so far) cost, prune it directly.
                continue;

            //check current solution if it is better than the current best solution so far, update the best solution.
            if (current_solution.is_solution()) // cost = -1 means, this solution is not feasible (cannot back to the starting city)
            {
                //                for (int i = 0; i < MAXSIZE; i++){
                //                    cout << current_solution.path[i] << endl;
                //                }
                best_solution = current_solution;
                best_path_list[rank] = best_solution;
                //printf("Rank - %d current best solution\n", rank);

                // If i find a solution I want to sent my solution path and a tag of 1 to every other processor
                for(int i=0; i<size; i++){
                    if(rank!=i){
//                        inputBuffer[i].cost = best_solution.cost;
//                        inputBuffer[i].number_visit_city = best_solution.number_visit_city;
//                        for(int j=0; j<MAXSIZE; j++){
//                            inputBuffer[i].path[j] = best_solution.path[j];
//                        }
                        if(MPI_Send(&best_solution, 1, MPI_Path, i, 0, MPI_COMM_WORLD) == MPI_SUCCESS){
                            printf("Succcessfully sent data to processor %d\n",i);
                        }else{
                            printf("Error Sending data to processor %d\n",i);
                        }
                    }
                }
                //MPI_Barrier(MPI_COMM_WORLD);
                //continue;
            }

            if (!current_solution.is_solution()) //explore the partial solution (this partial solution has less cost than the best solution, pruned by the above if statement).
            {
                // now visit all the cities that can be visited (if the graph is not a complete graph) and also, has not been visited yet (skip the visited cities, not repeat path) :
                for (int i = 0; i < MAXSIZE; i++)
                    if (adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i] != INF && !current_solution.visited(i))    // check if can be visited AND is an unvisited city
                    {
                        int lowerBoundEstimation, costTemp = 0; // LB cost, temp path cost

                        // compute the current cost so far (for the partial solution).
                        for (int pathi = 0; pathi < current_solution.number_visit_city - 1; pathi++)
                            costTemp += adj_matrix[current_solution.path[pathi]][current_solution.path[pathi + 1]];

                        // add the next city based on the index i
                        costTemp += adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i];

                        //Now, based on this new city (i), check the MST cost of the remaining part which is the lower-bound cost (but may not be able to reach)
                        vector<int> primsVertices;  // The structure for computing MST using PRIM algo.

                        for (int city = 0; city < MAXSIZE; city++)  // get all cities that will be involved into MST (in the sub-graph)
                            if (city == 0 || city == i || !current_solution.visited(city))  // if the city has not been visited, or the starting city (need to add the path/cost back to the starting city)
                                primsVertices.push_back(city);
                        int vprims = primsVertices.size();  // # of cities in the MST

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
                                        temp[x][y] = temp[y][x] = adj_matrix[primsVertices[x]][primsVertices[y]];   // symmetry matrix

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
            MPI_Barrier(MPI_COMM_WORLD);

        }
        //MPI_Finalize();
    }else{
        //-----------------------------------------PART 2------------------------------------------//
        //-------------------------------------SEQ TSB W/ B&B using PRIM (MST) for LB estimation---//
        //SEQ-TSP w/ B&B, where the lower-bound cost is comuputed by using MST (minimum spanning tree) by PRIM algo.

        Path temp_solution; //the temp solution is stored here (temp_solution is assuming starting from city 1 (index in matrix = 0))
        temp_solution.init(); //initilzation

        //cout << "temp cost: " << temp_solution.cost << endl;

        // using priority queue (increasing order)
        // so that, we only explore the partial solution with the smallest cost so far.
        pq.push(temp_solution);

        // this for loop compute the cost for 0 -> 1 ->2 -> ... n-1
        for (int i = 0; i < MAXSIZE-1; i++)
        {
            best_solution.path[i] = i; //the order of visited for each city
            best_solution.cost += adj_matrix[i][i + 1]; //adding the cost for city i -> i+1 (i+1 is up to n-1)
        }
        best_solution.path[MAXSIZE-1]=MAXSIZE-1;
        best_solution.cost += adj_matrix[MAXSIZE - 1][0]; //adding the cost for (n-1 -> 0)

        //cout << "best cost: " << best_solution.cost << endl;
        //best_solution.toString();


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
                //  cout << current_solution.path[i] << endl;
                best_solution = current_solution;
                //printf("Rank - %d current best solution\n", rank);
                //best_solution.toString();
                //cout << "best cost: " << best_solution.cost << endl;
                continue;
            }

            if (!current_solution.is_solution()) //explore the partial solution (this partial solution has less cost than the best solution, pruned by the above if statement).
            {
                // now visit all the cities that can be visited (if the graph is not a complete graph) and also, has not been visited yet (skip the visited cities, not repeat path) :
                for (int i = 0; i < MAXSIZE; i++)
                    if (adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i] != INF && !current_solution.visited(i))    // check if can be visited AND is an unvisited city
                    {
                        int lowerBoundEstimation, costTemp = 0; // LB cost, temp path cost

                        // compute the current cost so far (for the partial solution).
                        for (int pathi = 0; pathi < current_solution.number_visit_city - 1; pathi++)
                            costTemp += adj_matrix[current_solution.path[pathi]][current_solution.path[pathi + 1]];

                        // add the next city based on the index i
                        costTemp += adj_matrix[current_solution.path[current_solution.number_visit_city - 1]][i];

                        //Now, based on this new city (i), check the MST cost of the remaining part which is the lower-bound cost (but may not be able to reach)
                        vector<int> primsVertices;  // The structure for computing MST using PRIM algo.

                        for (int city = 0; city < MAXSIZE; city++)  // get all cities that will be involved into MST (in the sub-graph)
                            if (city == 0 || city == i || !current_solution.visited(city))  // if the city has not been visited, or the starting city (need to add the path/cost back to the starting city)
                                primsVertices.push_back(city);
                        int vprims = primsVertices.size();  // # of cities in the MST

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
                                        temp[x][y] = temp[y][x] = adj_matrix[primsVertices[x]][primsVertices[y]];   // symmetry matrix

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