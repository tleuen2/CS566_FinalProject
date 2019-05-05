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
    int sender;

    Path(){}

    // Copy constructor
    Path(const Path& p)
    {
        number_visit_city = p.number_visit_city;
        cost = p.cost;
        sender = p.sender;
        std::copy(p.path, p.path + MAXSIZE, path);

    }

    //print out the final solution path + cost
    void toString() {
        for (int i = 0; i < MAXSIZE; i++)
            cout << path[i] << " -> ";
        cout << path[0] << " . The cost = " << cost << endl;
        cout << path[0] << " . The Sender = " << sender << endl;
    }

    //initializtion
    void init()
    {
        for (int i = 0; i < MAXSIZE; i++)
            path[i] = -1;
        path[0] = 0;
        cost = 0;
        number_visit_city = 1;
        sender = -1;
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
void send_termination_data(int rank, int num_processors, int sig_num, Path *toSend, MPI_Datatype MPI_Path);
bool send_termination_message(int rank, int num_processors, int sig_num);
bool recieve_termination_message(int rank, int num_processors, int sig_num, int *guysWhoAreDone);
// function to check if x is power of 2
bool isPowerOfTwo(unsigned int x);

// function to check whether the two numbers
// differ at one bit position only
bool differAtOneBitPos(unsigned int a, unsigned int b);

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
    MPI_Datatype type[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint disp[4];
    MPI_Aint base;

    //Configuration specific data
    int s_QE;// = std::stoi(argv[1]);
    sscanf(argv[1], "%d", &s_QE);
    int distance_QE;// = std::stoi(argv[2]);
    sscanf(argv[2], "%d", &distance_QE);

    // Displacement from the root
    MPI_Get_address(&sample, &base);
    MPI_Get_address(&sample.number_visit_city, &disp[0]);
    MPI_Get_address(&sample.cost, &(disp[1]));
    MPI_Get_address(&sample.path[0], &(disp[2]));
    MPI_Get_address(&sample.sender, &(disp[3]));

    for(int i = 0; i < 4; i++)
    {
        //disp[i] = MPI_Aint_diff(disp[i], base);
        disp[i] = (char*) disp[i] - (char*) base;
    }

    int blockLength[4] = {1,1, MAXSIZE,1};
    MPI_Type_create_struct(4,         // [in] number of blocks (integer), also number of entries in the next three parameters
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
        //printf("\nCannot open file strike any key exit!");
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

    //    for (int i = 0; i < size_input; i++)
    //    {
    //        for (int j = 0; j < size_input; j++)
    //            cout << adj_matrix[i][j] << " ";
    //        cout << endl;
    //    }

    int offloadCount = 1;
    int repeatCount = 5;

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
    int fortyFiveFlag[size];
    int fortyFlag[size];
    Path someonesSolution;
    priority_queue<Path, vector<Path>, NodeCompare> tempQ;
    priority_queue<Path, vector<Path>, NodeCompare> tempQ2;
    Path qeSendBuffer[size][offloadCount];
    Path qeReceiveBuffer[size][offloadCount];
    Path node_S_QE[size];
    Path node_S_QE_ReceiveBuffer[size];
    int node_S_flag[size];
    int fortyFiveSendBuffer[size];
    int fortyFiveReceiveBuffer[size];
    Path tempSolutionForSending40;
    Path myBestNode;
    Path otherSthBestNode;
    priority_queue<Path, vector<Path>, NodeCompare> sTH_Q;
    int rankToSend;
    int guysWhoAreDone = 0;





    //MPI_Barrier(MPI_COMM_WORLD);

    if(size > 1){
        //Parallel implementation goes here
        //Get the seeds as per rank
        distStart = MPI_Wtime();
        int partial_solution_size = 0;
        start_partition_phase(partial_solution_size, size, MAXSIZE, rank, &pq, adj_matrix);
        //printf("Called the start phase");
        //        while(!pq.empty()){
        //            //printf("Hello: rank %d\n",rank);
        //            Path current = pq.top();
        //            //current.toString();
        //            pq.pop();
        //        }
        //Display the seeds
        //-----------------------------------------PART 2------------------------------------------//
        //-------------------------------------SEQ TSB W/ B&B using PRIM (MST) for LB estimation---//
        //SEQ-TSP w/ B&B, where the lower-bound cost is comuputed by using MST (minimum spanning tree) by PRIM algo.

        //Path temp_solution; //the temp solution is stored here (temp_solution is assuming starting from city 1 (index in matrix = 0))
        //temp_solution.init(); //initilzation

        //cout << "temp cost: " << temp_solution.cost << endl;

        // using priority queue (increasing order)
        // so that, we only explore the partial solution with the smallest cost so far.
        //pq.push(temp_solution);

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
        int iter = 0;
        distEnd = MPI_Wtime();
        totalStart = MPI_Wtime();
        //printf("1->rank->%d\n", rank);
        while (!pq.empty()) // if there still exists some partial solution are not explored complete, keep check the result
        {
            Path current_solution = pq.top();  //everytime, only explore the partial solution with the smallest cost so far
            pq.pop();

            int cost1;
            int cost2 = 10;

            //Here we can check for code 60
            //printf("I am rank %d and checking for termination messages\n",rank);
            bool value = recieve_termination_message(rank, size, 60, &guysWhoAreDone);
            //printf("I am rank %d and value is %s\n",rank, value?"true":"false");
            if(value)
            {
                if(guysWhoAreDone == size-1){
                    //printf("Breaking the loop and announcing myself as a winner %d\n", rank);
                    break;
                }
                //printf("***This is the number that have terminated %d\n", guysWhoAreDone);
            }
            //guysWhoAreDone = 0;


            for(int i=0; i<size; i++){
                if(rank!=i){
                    MPI_Iprobe(i, 0, MPI_COMM_WORLD, &zeroFlag[i], &status[i]);
                    if(zeroFlag[i] != 0){
                        someonesSolution.init();
                        if(MPI_Recv(&someonesSolution, 1, MPI_Path, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE)== MPI_SUCCESS){
                            if(someonesSolution.cost<=best_solution.cost){
                                best_solution = someonesSolution;
                            }
                            zeroFlag[i] = 0;
                        }else{
                            //printf("Error making TAG0 receive probe for processor %d\n",i);
                        }
                    }
                }
            }

            if (current_solution.cost >= best_solution.cost) // if the cost is greater than the best solution (so far) cost, prune it directly.
            {
                iter++;
                continue;
            }
            if (current_solution.is_solution()) // cost = -1 means, this solution is not feasible (cannot back to the starting city)
            {
                best_solution = current_solution;
                best_solution.sender = rank;

                for(int i=0; i<size; i++){
                    if(rank!=i){
                        if(MPI_Send(&best_solution, 1, MPI_Path, i, 0, MPI_COMM_WORLD) == MPI_SUCCESS){
                        }else{
                            //printf("Error Sending data to processor %d\n",i);
                        }
                    }
                }
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

            //This is where we will do QE in this file
            //Broadcast the sth node if needed
            //Code runs with this
            //MPI_Barrier(MPI_COMM_WORLD);
            for(int i=0; i<s_QE-1; i++){
                Path pqs_Top = pq.top();
                tempQ.push(pqs_Top);
                pq.pop();
            }
            node_S_QE[0] = pq.top();
            while(!tempQ.empty()){
                Path tempQ_top = tempQ.top();
                pq.push(tempQ_top);
                tempQ.pop();
            }
            for(int i=1; i<size; i++){
                node_S_QE[i] = node_S_QE[0];
            }
            if(((iter+1)%5) == 0){
                for(int i=0; i<size; i++){
                    //node_S_QE[i] = tempQ2.top();
                    node_S_QE[i].sender = rank;
                    unsigned u_i = i;
                    unsigned u_rank = rank;
                    if(rank!=i && differAtOneBitPos(u_rank, u_i)){
                        //printf("50 Send of rank %d sending to ran %d\n", rank, node_S_QE[i].sender);
                        if(MPI_Send(&node_S_QE[i], 1, MPI_Path, i, 50, MPI_COMM_WORLD) == MPI_SUCCESS){
                            //printf("I am rank %d and I am broadcasting sth cost as %d\n", rank, node_S_QE[i].cost);
                            //printf("Succcessfully sent data to processor %d\n",i);
                        }else{
                            //printf("Error Sending data 50 to processor %d\n",i);
                        }
                    }
                }
            }




            //MPI_Barrier(MPI_COMM_WORLD);
            //Start receiving here 'S'th node of all the cores
            //Code runs with this too.
            for(int i=0; i<size; i++){
                unsigned u_i = i;
                unsigned u_rank = rank;
                if(rank!=i && differAtOneBitPos(u_rank, u_i)){
                    MPI_Iprobe(i, 50, MPI_COMM_WORLD, &node_S_flag[i], &status[i]);
                    if(node_S_flag[i] != 0){
                        //Someone has sent me some work.
                        //So i need to receive it
                        if(MPI_Recv(&node_S_QE_ReceiveBuffer[i], 1, MPI_Path, i, 50, MPI_COMM_WORLD,MPI_STATUS_IGNORE)== MPI_SUCCESS){
                            //Print something here if needed.
                            //printf("I am rank %d and I received sth node of %d\n", rank,node_S_QE_ReceiveBuffer[i].sender);
                            Path toSthQ;
                            toSthQ = node_S_QE_ReceiveBuffer[i];
                            sTH_Q.push(toSthQ);
                        }else{
                            //printf("Error making TAG0 receive probe for processor %d\n",i);
                        }
                        node_S_flag[i] = 0;
                    }

                }
            }

            //MPI_Barrier(MPI_COMM_WORLD);
            bool flag = false;
            if(sTH_Q.size()!=0){
                //Now i will compare the top of pq and sTH_Q
                myBestNode = pq.top();
                otherSthBestNode = sTH_Q.top();
                //otherSthBestNode.toString();
                sTH_Q.pop();
                if(pq.top().cost < otherSthBestNode.cost){
                    //This processor's top node is better than everyone, no need to ask for any work.print
                    //printf("I am rank %d and my best Node is %d\n", rank, pq.top().cost);
                    //printf("I am rank %d and sth Node is %d\n", rank, otherSthBestNode.cost);
                    //printf("I am rank %d and Mycost is best\n", rank);
                }else{
                    //I need to ask for some work and also raise the flag so i should not execute the loop
                    fortyFiveSendBuffer[otherSthBestNode.sender] = 1;
                    rankToSend = otherSthBestNode.sender;
                    //printf("45 Send of rank %d sending to ran %d\n", rank, rankToSend);
                    if(MPI_Send(&fortyFiveSendBuffer[rankToSend], 1, MPI_INT, rankToSend, 45, MPI_COMM_WORLD) == MPI_SUCCESS){
                        //printf("I am rank %d and I asked for data from %d\n", rank, rankToSend);
                        //printf("Succcessfully sent data to processor %d\n",i);
                    }else{
                        //printf("Error Sending data 45 to processor %d\n",rank);
                    }
                    flag = true;
                }
                //Need to flushout the sTH_Q
                while(!sTH_Q.empty()){
                    sTH_Q.pop();
                }
            }
            //MPI_Barrier(MPI_COMM_WORLD);

            //This is check 45 and send the nodes with signal 40.
            for(int i=0; i<size; i++){
                unsigned u_i = i;
                unsigned u_rank = rank;
                if(rank!=i && differAtOneBitPos(u_rank, u_i)){
                    MPI_Iprobe(i, 45, MPI_COMM_WORLD, &fortyFiveFlag[i], &status[i]);
                    //May be i need to receive the fortyfive in some dummy array so as to flush it out of the mpi ecosystem.
                    if(fortyFiveFlag[i] != 0){
                        //printf("I am rank %d and I am sending some data to %d\n", rank, i);
                        MPI_Recv(&fortyFiveReceiveBuffer[i], 1, MPI_INT, i, 45, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                        if(pq.size() < offloadCount*3){
                            continue;
                        }
                        for(int j = 0; j < s_QE - 1; j++){
                            Path pqs_Top = pq.top();
                            tempQ.push(pqs_Top);
                            pq.pop();
                        }
                        tempSolutionForSending40 = pq.top();
                        pq.pop();
                        while(!tempQ.empty()){
                            Path tempQ_top = tempQ.top();
                            pq.push(tempQ_top);
                            tempQ.pop();
                        }
                        //printf("40 Send of rank %d sending to ran %d\n", rank, i);
                        if(MPI_Send(&tempSolutionForSending40, offloadCount, MPI_Path, i, 40, MPI_COMM_WORLD) == MPI_SUCCESS){
                            //printf("Succcessfully sent data to processor %d\n",i);
                        }else{
                            //printf("Error Sending 40 to processor %d\n",i);
                        }
                        fortyFiveFlag[i] = 0;
                    }else{
                        //printf("I am rank %d and nobody asked me anything\n", rank);
                    }
                }
            }

            //There will be a QE barrier here, B2
            //MPI_Barrier(MPI_COMM_WORLD);

            //This is check 40 and receive the nodes in qeReceiveBuffer.
            for(int i=0; i<size; i++){
                unsigned u_i = i;
                unsigned u_rank = rank;
                if(rank!=i && differAtOneBitPos(u_rank, u_i)){
                    MPI_Iprobe(i, 40, MPI_COMM_WORLD, &fortyFlag[i], &status[i]);
                    if(fortyFlag[i] != 0){
                        //Someone has send me some work.
                        //So i need to receive it
                        //printf("I am rank %d and I am receiving some data from %d\n", rank, i);
                        if(MPI_Recv(&qeReceiveBuffer[i], offloadCount, MPI_Path, i, 40, MPI_COMM_WORLD,MPI_STATUS_IGNORE)== MPI_SUCCESS){
                            for(int j=0; j<offloadCount; j++){
                                pq.push(qeReceiveBuffer[i][j]);
                            }
                        }else{
                            //printf("Error making TAG0 receive probe for processor %d\n",i);
                        }
                        fortyFlag[i] = 0;
                    }
                }
            }
            //send_termination_message(rank, size, 60);

            iter++;
        }
        if(send_termination_message(rank, size, 60)){
            //printf("%s\n", "Send the 60 message properly");
        }
        MPI_Barrier(MPI_COMM_WORLD);
        totalEnd = MPI_Wtime();
        //printf("Iteration %d\n", iter);
        //MPI_Finalize();
    }else{
        int iter=0;
        //-----------------------------------------PART 2------------------------------------------//
        //-------------------------------------SEQ TSB W/ B&B using PRIM (MST) for LB estimation---//
        //SEQ-TSP w/ B&B, where the lower-bound cost is comuputed by using MST (minimum spanning tree) by PRIM algo.
        distStart = MPI_Wtime();
        //        Path temp_solution; //the temp solution is stored here (temp_solution is assuming starting from city 1 (index in matrix = 0))
        //        temp_solution.init(); //initilzation
        int partial_solution_size = 0;
        start_partition_phase(partial_solution_size, size, MAXSIZE, rank, &pq, adj_matrix);

        //cout << "temp cost: " << temp_solution.cost << endl;

        // using priority queue (increasing order)
        // so that, we only explore the partial solution with the smallest cost so far.
        //
        //pq.push(temp_solution);

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
        distStart = MPI_Wtime();
        totalStart = MPI_Wtime();
        while (!pq.empty()) // if there still exists some partial solution are not explored complete, keep check the result
        {
            Path current_solution = pq.top();  //everytime, only explore the partial solution with the smallest cost so far

            pq.pop();
            //current_solution.toString();
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
                            //printf("Lower cost is %d\n", lowerBoundEstimation);
                        }
                        else //just in case if only 1 city here, the cases is i = 0, where the starting city is the only one remaining (i must be 0, otherwise, the size of vprims = 2 at least).
                            lowerBoundEstimation = 0; //thus, no LB cost (starting city to starting city).

                        //Now, after finding the LB cost, update the estimated LB cost to the current cost so that, can have an esitmation of how good the current solution is.
                        int costEstimation = costTemp + lowerBoundEstimation;
                        //printf("costEstimation is %d\n", costEstimation);
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
            iter++;
        }

    }
    //send_termination_message(rank, size, 60);
    totalEnd = MPI_Wtime();
    MPI_Finalize();

    if(rank == 0)
    {
        printf("***************FINISHED***************\n");
        printf("--Config--\n");
        printf("\t--Number of Processors = %d\n", size);
        printf("\t--Input data size - (n * n) matrix - = %d\n", MAXSIZE);
        printf("--Results--\n");
        printf("\t--My Best Solution Path = ");
        best_solution.toString();
        printf("\n\t--My Distribution time = %f\n", distEnd-distStart);
        printf("\t--My Computation time = %f\n", totalEnd-totalStart);
        printf("***************FINISHED***************\n");
    }


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
        for (i = 0; i < n; i++)         // trace the node that has been visited
            if (visited[i] == true)
                for (j = 0; j < n; j++) // trace the node that has not been visited
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

void start_partition_phase(int partial_solution_size, int size_of_processors, int size_input, int rank,
                           priority_queue<Path, vector<Path>, NodeCompare>  *myQueue, float adj_matrix[MAXSIZE][MAXSIZE])
{
    //Initialise the seed
    //Get the number of combinations needed and generate them from two buffers


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
    // Get the intial paths and add them to the temporary queue
    for(int k = 1; k < size_input; k++)
    {
        Path temp;
        temp.init();
        //temp.path[temp.number_visit_city - 1] = 0;
        if(adj_matrix[temp.path[temp.number_visit_city - 1]][k] == INF){
            continue;
        }
        temp.path[temp.number_visit_city] = k;
        temp.cost = temp.cost + adj_matrix[temp.path[temp.number_visit_city - 1]][temp.path[temp.number_visit_city]];
        temp.number_visit_city += 1;
        tempBuffer1.push(temp);
        totalCount++;
    }

    int number_of_partial_solution = totalCount;
    partial_solution_size = 1;
    while (number_of_partial_solution < size_of_processors && partial_solution_size < size_input) //16proc20city
    {
        number_of_partial_solution *= (size_input - partial_solution_size - 1);
        partial_solution_size++;
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
        int tempInputSize = tempInput->size();
        for(int i = 0; i < tempInputSize; i++)
        {
            Path node = tempInput->top();
            tempInput->pop();

            for(int j = 0; j < size_input; j++)
            {
                if(!node.visited(j))
                {
                    //get a copy of the current solution and update path info, cost info, and # of cities have been visited info.
                    Path newNode;
                    newNode.init();
                    std::copy(node.path, node.path + MAXSIZE, newNode.path);
                    newNode.cost = node.cost;
                    newNode.path[node.number_visit_city] = j;
                    newNode.number_visit_city = node.number_visit_city + 1;
                    if(newNode.number_visit_city > 1)
                    {
                        for(int i = 0; i < MAXSIZE - 1; i++)
                        {
                            if(newNode.path[i] != -1)
                            {
                                newNode.cost += adj_matrix[newNode.path[i]][newNode.path[i + 1]];
                            }
                        }
                    }
                    tempOutput->push(newNode);
                }
            }
        }
        Buffer1 = !Buffer1;
    }

    // Give all of the processors their intial set of partial solutions.
    //cout << number_of_partial_solution << " - Partial Solutions" << endl;
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
    int max_local_solutions;
    int num_of_solutions_so_far = 0;
    int temp = number_of_partial_solution;
    for(int w = 0; w < number_of_partial_solution; w++)
    {
        Path outNode = tempInput->top();
        while(temp % size_of_processors != 0)
        {
            temp++;
        }
        max_local_solutions = temp / size_of_processors;

        if(num_of_solutions_so_far > max_local_solutions)
        {
            continue;
        }
        if(w == (rank + (size_of_processors * num_of_solutions_so_far)))
        {
            // This is a local solution for this rank


            //std::copy(outNode.path, outNode.path + MAXSIZE, outNode.path);

            myQueue->push(outNode);
            num_of_solutions_so_far++;
        }
        tempInput->pop();

    }

    // Now, number_of_partial_solution is the total number of initial solution will be
    // also, partial_solution_size is the # of cities in each partial solution

    //after known how many cities should be in the partial solution at the beginning, each processor will start to obtain their own initial solutions
    // you can now implement your own way to do the partition processing
}


bool recieve_termination_message(int rank, int num_processors, int sig_num, int *guysWhoAreDone)
{
    bool output = false;
    int recieveFlags[num_processors];
    int recieveBuffer[num_processors];
    MPI_Status status1[num_processors];
    for(int i = 0; i < num_processors; i++)
    {
        unsigned u_i = i;
        unsigned u_rank = rank;
        if(rank!=i && differAtOneBitPos(u_rank, u_i))
        {
            MPI_Iprobe(i, sig_num, MPI_COMM_WORLD, &recieveFlags[i], &status1[i]);
            //May be i need to receive the fortyfive in some dummy array so as to flush it out of the mpi ecosystem.
            if (recieveFlags[i] != 0)
            {
                //printf("I am rank %d and I found out another processor terminated %d\n", rank, i);
                MPI_Recv(&recieveBuffer[i], 1, MPI_INT, i, sig_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                output = true;
                *guysWhoAreDone += 1;
            }
            recieveFlags[i] = 0;
        }
    }

    return output;
}

void recieve_termination_data(int rank, int num_processors, int sig_num, Path *output, MPI_Datatype MPI_Path, int guysWhoAreDone[])
{
    int recieveFlags[num_processors];
    MPI_Status status[num_processors];
    for(int i = 0; i < num_processors; i++)
    {
        if(guysWhoAreDone[i] != 0)
        {
            unsigned u_i = i;
            unsigned u_rank = rank;
            if(rank!=i && differAtOneBitPos(u_rank, u_i)){
                MPI_Iprobe(i, sig_num, MPI_COMM_WORLD, &recieveFlags[i], &status[i]);
                //May be i need to receive the fortyfive in some dummy array so as to flush it out of the mpi ecosystem.
                if (recieveFlags[i] != 0) {
                    //printf("I am rank %d and I found out another processor terminated %d\n", rank, i);
                    MPI_Recv(&output, 1, MPI_Path, i, sig_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                }
            }
        }
    }
}

bool send_termination_message(int rank, int num_processors, int sig_num)
{

    int sendValues[num_processors];
    for(int i = 0; i < num_processors; i++)
    {
        sendValues[i] = 1;
        unsigned u_i = i;
        unsigned u_rank = rank;
        if(rank!=i && differAtOneBitPos(u_rank, u_i))
        {
            //printf("I am terminated, snding out the message now, from rank %d\n", rank);
            MPI_Send(&sendValues[i], 1, MPI_INT, i, sig_num, MPI_COMM_WORLD);
        }
    }
    return true;
}

void send_termination_data(int rank, int num_processors, int sig_num, Path *toSend, MPI_Datatype MPI_Path)
{
    for(int i = 0; i < num_processors; i++)
    {
        unsigned u_i = i;
        unsigned u_rank = rank;
        if(rank!=i && differAtOneBitPos(u_rank, u_i))
        {
            MPI_Send(&toSend, 1, MPI_Path, i, sig_num, MPI_COMM_WORLD);
        }
    }
}

// function to check if x is power of 2
bool isPowerOfTwo(unsigned int x)
{
    // First x in the below expression is
    // for the case when x is 0
    return x && (!(x & (x - 1)));
}

// function to check whether the two numbers
// differ at one bit position only
bool differAtOneBitPos(unsigned int a,
                       unsigned int b)
{
    return isPowerOfTwo(a ^ b);
}