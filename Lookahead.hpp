#include <random>
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <list>
using namespace std;

#include <chrono>
using namespace std::chrono;

#define CLOSED false
#define OPEN true

//algorithm parameters
//optional
//#define READ_ORIGINAL false
//optional
#define LOOKAHEAD false
#define LOOKAHEAD_BECCENERI true
#define LOOKAHEAD_YANASSE true
//only for OPTSICOM instances

#define REDUCE_BOTTLENECK_LOOP false
//if the above is selected one of the following must be selected
#define REDUCE_BOTTLENECK_LOOP_SIZE 40
//if the above is selected one of the following must be selected
#define REDUCE_BOTTLENECK false
#define REDUCE_BOTTLENECK_BEST_POSITION true

//optional
//#define RANDOM_PIECES false
//#define RANDOM_PATTERNS_BOTTLENECK false
//if not selected, then BFS starting form the minimum degree node
#define MULTIPLE_START_BFS false
//defines the depth for the scheduling rules
#define THRESHOLD_PERCENTAGE 0.1

#define BOTTLENECK_CHECK REDUCE_BOTTLENECK||REDUCE_BOTTLENECK_BEST_POSITION
#define GENERATE_ALL_SOLUTIONS false


//---

#define BREAK_TIES true

double tieBreaker = 999;

#define BEST_SOLUTION_PRUNING false
#define GMPLAN false
#define SCOOP false
#define CHU_STUCKEY false
#define CHALLENGE false
#define SMALL false
#define GRIDS false
#define FAGGIOLI_BENTIVOGLIO false

#define READ_ORIGINAL false

#define RANDOM_PIECES true
#define RANDOM_PATTERNS_BOTTLENECK true
#define RANDOM_PATTERNS_BI true
#define RANDOM_PATTERNS_FI true

#define LOOP_SIZE 1000
#define PLATEAU_LIMIT 500

#define LS_REDUCE_BOTTLENECK false

#define LS_REDUCE_BOTTLENECK_BEST_POSITION false
//#define DONT_MOVE_BOTTLENECKS false

//#define NEW_FIND_BEST_POSITION false
//#define OLD_BEST_POSITION true

#define LS_REMOVE_BOTTLENECK_RELATED_PATTERNS false
//#define REDUCE_STACK_SIZE_RELATED_PATTERNS false

#define LS_REMOVE_BOTTLENECKS false
//#define REDUCE_STACK_SIZE_BOTTLENECK_PATTERNS true

#define LS_RULE1 false
#define LS_RULE2 false
#define LS_BOTH_RULES false

#define LS_ALL true
#define DONT_MOVE_BOTTLENECKS false
#define OLD_BEST_POSITION true
#define NEW_FIND_BEST_POSITION false
#define REDUCE_STACK_SIZE_RELATED_PATTERNS false
#define REDUCE_STACK_SIZE_BOTTLENECK_PATTERNS false

#define CUTWIDTH true

class Pattern{
public:
	Pattern()
	{
		sequenced = false;
		dominated = false;
		size = 0;
	}

	void setSequenced(bool b)
	{
		sequenced = b;
	}
	
	void setDominated(bool b)
	{
		dominated = b;
	}
	
	void setSize(int i)
	{
		size = i;
	}

	bool isSequenced()
	{
		return sequenced;
	}
	
	bool isDominated()
	{
		return dominated;
	}
	
	int getSize()
	{
		return size;
	}

private:
	bool sequenced; 	//indicates whether the pattern has been already sequenced
	bool dominated; 	//indicates whether the pattern has been removed in the preprocessing 
	int size;    		//stores the size of the pattern, in number of pieces
};		

class Node{
public:	
	Node()
	{
		degree = 0;
		visited = false;
		id = 0;
	}
	
	void setDegree(int i)
	{
		degree = i;
	}
	
	void setVisited(bool b)
	{
		visited = b;
	}
		
	void setId(int i)
	{
		id = i;
	}

	int getDegree()
	{
		return degree;
	}
	
	bool wasVisited()
	{
		return visited;
	}
		
	int getId()
	{
		return id;
	}

private:
	int degree;	 //stores the degree of the node
	int id;		 //stores the index of the node
	bool visited; //indicates whether the node has been reached by the BFS
};

int lookahead_counter;

vector<Pattern> pattern;					//stores information about patterns during preprocessing and sequencing
vector<Node> node;							//stores information about nodes during the BFS
vector<list<int> > dominatedPatterns;		//stores information about patterns dominated by others
vector<vector<int> > inputMatrix;			//stores the input data
vector<vector<int> > adjacencyMatrix;		//stores the adjancency matrix of the MOSP graph, used as an upper diagonal matrix
vector<set<int> > patternStacks;			//stores the stacks related to each pattern
vector<set<int> > stackPatterns;			//stores the patterns related to each stack
vector<bool> stack;							//simulates the open stacks
vector<int> stackSize;						//stores the demands for pieces (original)
vector<int> stackSizeOriginal;				//stores the demands for pieces
vector<int> stackSizeEvaluation;			//stores the demands for pieces
vector<int> solution;						//stores the pattern sequencing
vector<int> alternativeSolution;			//stores the alternative pattern sequencing
vector<int> nodeSequence;					//stores the pieces sequencing
vector<int> degree;							//stores the vertices degrees on the complementary MOSP graph
vector<int> Q;								//the queue for the breadth first search
vector<int> patterns;
vector<int> pieces;

set<int> bottleneckPatterns;
set<int> bottleneckRelatedPatterns;

int nPatterns;								//number of patterns
int nPieces;								//number of pieces
int nPatternsOriginal;						//original number of patterns
int nOpenStacks;							//number of open stacks
int lookahead;								//how much to lookahead on local search
int largestPatternSize;						//stores the largest pattern size, used as lower bound

//GMPLAN
int GMPLAN_Solution[] = {3, 5, 5, 10, 9, 9, 4, 5, 4, 14, 18, 27, 6, 4, 8, 11, 5, 6, 7, 2, 2, 2, 4, 4, 4};

//SCOOP
int SCOOP_Solution[] = {12, 11, 9, 17, 9, 11, 13, 11, 6, 6, 6, 5, 6, 6, 6, 4, 10, 5, 5, 5, 5, 6, 5, 7};

//CHU-STUCKEY
int CHU_STUCKEY_Solution[] = {87, 85, 83, 84, 84, 24, 20, 23, 15, 19, 46, 50, 43, 44, 42, 68, 66, 61, 65, 63, 79, 76, 76, 77, 75, 68, 68, 68, 65, 67, 12, 11, 15, 8, 12, 26, 29, 29, 24, 24, 44, 43, 46, 48, 39, 55, 55, 56, 60, 55, 105, 104, 103, 105, 99, 24, 25, 21, 24, 20, 57, 57, 54, 51, 46, 81, 77, 80, 80, 74, 95, 93, 95, 94, 91, 27, 26, 27, 27, 28, 9, 8, 9, 9, 8, 17, 15, 16, 15, 17, 23, 20, 22, 21, 20, 25, 25, 26, 25, 25, 36, 36, 34, 36, 35, 11, 14, 10, 7, 13, 21, 21, 19, 18, 22, 28, 30, 26, 29, 28, 34, 32, 30, 33, 34, 48, 48, 48, 48, 47, 19, 21, 22, 24, 20, 35, 34, 34, 35, 35, 44, 44, 42, 42, 40, 47, 47, 46, 46, 45, 44, 43, 44, 45, 46, 15, 13, 11, 13, 14, 21, 25, 24, 27, 22, 36, 35, 33, 35, 37, 40, 39, 39, 42, 42, 66, 65, 65, 65, 63, 16, 17, 17, 13, 16, 35, 35, 37, 32, 32, 49, 52, 51, 47, 51, 58, 61, 60, 59, 59};

//CHALLENGE
int CHALLENGE_Solution[] = {45, 40, 40, 30, 95, 75, 75, 60, 13, 3, 4, 7, 7, 12, 12, 10, 16, 9, 19, 34, 53, 14, 13, 15, 13, 13, 14, 14, 14, 15, 14, 14, 12, 14, 12, 14, 14, 14, 14, 14, 13, 14, 14, 13, 14, 13};

//SMALL
int SMALL_Solution[] = {7, 5, 4, 4, 4, 4, 5, 8, 4, 4, 4, 3, 3, 4, 3, 4, 4, 4, 3, 4, 4, 3, 3, 8, 3, 5, 4, 6, 5, 3, 4, 4, 4, 4, 6, 5, 4, 6, 4, 5, 4, 3, 4, 4, 4, 7, 12, 3, 4, 6, 3, 6, 5, 5, 5, 14, 5, 6, 5, 6, 8, 6, 5, 5, 13, 4, 4, 4, 4, 3, 6, 4, 5, 7, 6, 4, 4, 6, 4, 4, 4, 5, 5, 7};

//GRIDS
int GRIDS_Solution[] = {0, 0, 0, 62, 0, 10, 5, 5, 5, 0, 0, 22, 1089, 107, 34, 0, 0, 6, 0, 25, 21, 0, 0, 10, 0, 0, 6, 0, 0, 0, 0, 0, 0, 52, 0, 0, 19, 0, 10, 19, 0, 0, 0, 20, 0, 18, 11, 0, 0, 6, 0, 37, 37, 0, 58, 0, 0, 4, 0, 12, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 14, 12, 12, 0, 0, 0, 0, 16, 7, 13, 13, 13, 13, 13, 13, 4, 7, 10, 13, 16, 16, 16, 16, 16, 4, 7, 10, 13, 16, 19, 19, 19, 19, 4, 7, 10, 13, 16, 19, 22, 22, 22, 4, 7, 10, 13, 16, 19, 22, 25, 25, 4, 7, 10, 13, 16, 19, 22, 25, 28, 4, 7, 10, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 4, 7, 7, 10, 10, 10, 10, 10, 10, 4, 7, 10};

//FAGGIOLI_BENTIVOGLIO
int FB_Solution[]= {6, 5, 6, 6, 5, 5, 6, 5, 6, 5, 6, 6, 8, 5, 6, 6, 6, 6, 7, 6, 6, 7, 6, 6, 6, 7, 5, 6, 6, 6, 7, 8, 8, 8, 8, 8, 7, 8, 8, 7, 8, 9, 9, 7, 7, 9, 9, 8, 8, 8, 6, 6, 6, 7, 7, 7, 7, 7, 7, 6, 7, 8, 6, 7, 7, 8, 7, 7, 8, 7, 6, 8, 8, 6, 8, 6, 7, 8, 8, 8, 8, 8, 6, 7, 7, 8, 7, 7, 7, 7, 8, 8, 8, 8, 7, 7, 7, 6, 8, 7, 8, 8, 7, 8, 8, 7, 7, 7, 8, 7, 9, 9, 9, 9, 8, 8, 9, 9, 7, 8, 7, 9, 8, 9, 8, 10, 9, 9, 10, 9, 8, 9, 8, 9, 9, 8, 8, 8, 9, 9, 9, 7, 7, 8, 8, 7, 8, 8, 7, 10, 8, 8, 7, 9, 8, 8, 8, 8, 8, 8, 9, 10, 10, 9, 10, 10, 9, 10, 11, 10, 9, 12, 11, 9, 12, 11, 10, 10, 11, 10, 10, 10, 11, 10, 10, 10, 11, 10, 11, 10, 11, 9, 11, 9, 10, 10, 9, 10, 11, 10, 8, 7, 8, 8, 8, 8, 8, 8, 8, 7, 11, 11, 12, 10, 11, 10, 13, 11, 11, 11, 11, 12, 13, 13, 12, 13, 12, 12, 12, 12, 12, 12, 12, 11, 12, 13, 12, 13, 12, 12, 12, 11, 11, 8, 11, 12, 13, 12, 10, 12, 9, 8, 9, 9, 8, 8, 8, 9, 8, 8, 13, 13, 13, 13, 13, 13, 13, 12, 14, 13, 13, 15, 15, 15, 15, 15, 16, 13, 14, 14, 16, 15, 15, 15, 14, 15, 15, 14, 14, 16, 14, 14, 15, 15, 14, 15, 15, 14, 15, 15};

int evaluation(vector<int> solution, bool bottleneckCheck);

/*
Initializes the structures, vectors and matrices used
*/
void initialization()
{
	int i;

	inputMatrix.resize(nPatterns);

	for(i=0; i<nPatterns; i++)
		inputMatrix[i].resize(nPieces);

	patternStacks.resize(nPatterns);
	stackPatterns.resize(nPieces);
	pattern.resize(nPatterns);
	stack.resize(nPieces);
	stackSize.resize(nPieces);
	solution.resize(nPatterns);
    alternativeSolution.resize(nPatterns);
	dominatedPatterns.resize(nPatterns);
	node.resize(nPieces);

	for(i=0; i<nPieces; i++)
		node[i].setId(i);

	adjacencyMatrix.resize(nPieces);
	
	for(i=0; i<nPieces; i++)
		adjacencyMatrix[i].resize(nPieces);
	
	degree.resize(nPieces);
	Q.resize(nPieces*nPieces, -1); 
	stackSizeOriginal.resize(nPieces);
    stackSizeEvaluation.resize(nPieces);
}

/*
Reads the problem from a file specified by fileName
*/
void readProblemMatrix(char *fileName)
{
	int i, j, k, patternSize;
	char description[257];
	FILE *fpIn = fopen(fileName, "r");						//input file

	fgets(description, 256, fpIn);

	fscanf(fpIn, "%d %d", &nPieces, &nPatterns);

	nPatternsOriginal = nPatterns;

	initialization();										//initializes all structures, vectors and matrices
	
	if(READ_ORIGINAL)
		for(j=nPieces-1; j>=0; j--)
		{
			for(i=0; i<nPatterns; i++)
			{
				fscanf(fpIn, "%d", &inputMatrix[i][j]);

				if(inputMatrix[i][j] == 1)
				{
					pattern[i].setSize(pattern[i].getSize()+1);
					patternStacks[i].insert(j);
				}
			}
		}
	else	
		for(j=0; j<nPieces;j++)
		{
			for(i=0; i<nPatterns; i++)
			{
				fscanf(fpIn, "%d", &inputMatrix[i][j]);

				if(inputMatrix[i][j] == 1)
				{
					pattern[i].setSize(pattern[i].getSize()+1);
					patternStacks[i].insert(j);
				}
			}
		}

	fclose(fpIn);

	largestPatternSize = 0; 	
}

/*
Builds the MOSP graph 
*/
void buildMOSPGraph()
{
    int i, j, k;
    
	for(j=nPieces-1; j>=0; j--)
	{
		for(i=0; i<nPatternsOriginal; i++)
		{
			if((!pattern[i].isDominated())&&(inputMatrix[i][j] == 1))	
			{                  
  			    for(k=0; k<nPieces; k++)
				{
					if((j!=k)&&(inputMatrix[i][k] == 1))			//updates the adjacency matrix
					{
						if (k<j)
						{
							adjacencyMatrix[k][j] = 1;
						}
						else
						{
							adjacencyMatrix[j][k] = 1;
						}
					}
				}
				 stackPatterns[j].insert(i);
                 stackSize[j]++;
                 stackSizeOriginal[j]++;
            }
		}
	}

   //Determines the degree of each node on the MOSP graph
   for(i=0; i<nPieces; i++)
   {
       for(j=0; j<nPieces; j++)
       {
           if(i<j)
               node[i].setDegree(node[i].getDegree()+adjacencyMatrix[i][j]);
           else if(i>j)
               node[i].setDegree(node[i].getDegree()+adjacencyMatrix[j][i]);
       }
   }

   //stores the degree of the vertices
   for(i=0; i<nPieces; i++){
  	  degree[i] = node[i].getDegree();
  }
}

/*
Swap procedure used by k-opt
*/
void swap(vector<int> elements, bool undo)
{
	int aux;
/*
	cout<<"Antes..."<<endl;
	for(vector<int>::iterator it = solution.begin(); it!= solution.end(); it++)
		cout<<*it<<" ";

	cout<<endl;
*/	
	if(!undo)
	{
		aux = solution[elements[0]];

		for(vector<int>::iterator it = elements.begin()+1; it!= elements.end(); it++)
		{
			solution[*(it-1)] = solution[*it];
		}

		solution[elements.back()] = aux;
	}
	else
	{
		aux = solution[elements.back()];

		for(vector<int>::reverse_iterator it = elements.rbegin()+1; it!= elements.rend(); it++)
		{
			solution[*(it-1)] = solution[*it];
		}

		solution[elements[0]] = aux;
	}
/*
	cout<<"Depois..."<<endl;
	for(vector<int>::iterator it = solution.begin(); it!= solution.end(); it++)
		cout<<*it<<" ";

	cout<<endl;
	getchar();
*/
}


/*
Local Search: k-opt (http://rosettacode.org/wiki/Category:C%2B%2B)
*/
void kOpt(int k)
{
	int previousSolution, newSolution;
	vector<int> elements;

    string bitmask(k, 1); 										// k leading 1's
    bitmask.resize(nPatterns, 0); 								// nPatterns-k trailing 0's

    previousSolution = evaluation(solution, false);

    do {
		elements.clear();														// print integers and permute bitmask
        for (int i = 0; i < nPatterns; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) 
            {
            	elements.push_back(i);
            }
        }

        swap(elements, false);

        newSolution = evaluation(solution, false);

        if(newSolution < previousSolution)
        {
        	cout<<"Solução melhorou, antes: "<<previousSolution<<" depois: "<<newSolution<<endl;
        	previousSolution = newSolution;
        	//getchar();
       		bitmask.assign(nPatterns, 0); 								// nPatterns-k trailing 0's
      		bitmask.replace(bitmask.begin(),bitmask.begin()+k,k,1);                // "replace is cooool!!!"  (5)
        }
        else
        {
        	swap(elements, true);
        }
    } while (prev_permutation(bitmask.begin(), bitmask.end()));
}

/*
Local Search: Tries to reduce the consecutive 1s of the bottleneck columns by reallocating the related columns to right after/before the bottleneck
*/
void reduceBottleNeckConsecutive1s(int previousSolution)
{
	int bottleneck;
	int relatedPattern;
	int newSolution;

	double previousTieBreaker;

	vector<int>::iterator it3;
	vector<int>::iterator it4;

	if(DONT_MOVE_BOTTLENECKS)
		for(set<int>::iterator itera = bottleneckPatterns.begin(); itera!=bottleneckPatterns.end(); ++itera){
			set<int>::iterator it6=find(bottleneckRelatedPatterns.begin(), bottleneckRelatedPatterns.end(), *itera);
			if(it6 != bottleneckRelatedPatterns.end())
				bottleneckRelatedPatterns.erase(it6);
		}

	vector<int> bottleneckPatternsVector(bottleneckPatterns.begin(), bottleneckPatterns.end());
	vector<int> bottleneckRelatedPatternsVector(bottleneckRelatedPatterns.begin(), bottleneckRelatedPatterns.end());

	if(RANDOM_PATTERNS_BOTTLENECK)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		shuffle(bottleneckPatternsVector.begin(), bottleneckPatternsVector.end(), std::default_random_engine(seed));	
		shuffle(bottleneckRelatedPatternsVector.begin(), bottleneckRelatedPatternsVector.end(), std::default_random_engine(seed));
	}

	for(vector<int>::iterator it = bottleneckPatternsVector.begin(); it!=bottleneckPatternsVector.end(); ++it)
	{
		bottleneck = *it;

		for(vector<int>::iterator it2 = bottleneckRelatedPatternsVector.begin(); it2!=bottleneckRelatedPatternsVector.end(); ++it2)
		{
			relatedPattern = *it2;

			if(bottleneck != relatedPattern)
			{
				it3 = find(solution.begin(), solution.end(), bottleneck);	
				it4 = find(solution.begin(), solution.end(), relatedPattern);

				if((it3 < it4 && it3 != solution.begin()) || (it3 > it4 && it3 != solution.end()))	
				{
					solution.erase(it4);
					solution.insert(it3, relatedPattern);

					previousTieBreaker = tieBreaker;

					newSolution = evaluation(solution, false);

					if(!BREAK_TIES)
					{
						if(newSolution > previousSolution)
						{
							solution.erase(it3);
							solution.insert(it4, relatedPattern);							
						}
						else
							previousSolution = newSolution;
					}
					else
					{
						if(newSolution > previousSolution || (newSolution == previousSolution && tieBreaker > previousTieBreaker))
						{
							solution.erase(it3);
							solution.insert(it4, relatedPattern);							
						}
						else
						{
							previousSolution = newSolution;						
							previousTieBreaker = tieBreaker;
						}
					}
				}
			}
		}
	}	
}

/*
Finds the best position (minimum symmetric difference to other patterns on the opposite side of the bottleneck) to insert the reallocated patterns 
*/
vector<int>::iterator findBestOppositePositionWRTPatterns(vector<int>::iterator relatedPattern, vector<int>::iterator bottleneck)
{
	int min = INT_MAX;
	vector<int>::iterator bestPosition;

	bestPosition = relatedPattern;

	int d = distance(relatedPattern, bottleneck);

	if(d < 0)
	{
		for(vector<int>::iterator it = solution.begin(); it != bottleneck+1; ++it)
			if(*it != *relatedPattern)
			{
				vector<int> diff(nPieces*2);
				vector<int>::iterator it2 = set_symmetric_difference (patternStacks[*relatedPattern].begin(), patternStacks[*relatedPattern].end(), patternStacks[*it].begin(), patternStacks[*it].end(), diff.begin());

				diff.resize(it2-diff.begin()); 

				if(diff.size()<min)
				{
					min = diff.size();
					bestPosition = it;
				}	
			}
	}
	else
	{
		for(vector<int>::iterator it = bottleneck+1; it!= solution.end(); ++it)
			if(*it != *relatedPattern)
			{
				vector<int> diff(nPieces*2);
				vector<int>::iterator it2 = set_symmetric_difference (patternStacks[*relatedPattern].begin(), patternStacks[*relatedPattern].end(), patternStacks[*it].begin(), patternStacks[*it].end(), diff.begin());

				diff.resize(it2-diff.begin()); 

				if(diff.size()<min)
				{
					min = diff.size();
					bestPosition = it;
				}	
			}
	}	

	return bestPosition;
}

/*
Finds the best position (minimum symmetric difference to open stacks on the opposite side of the bottleneck) to insert the reallocated patterns
*/
vector<int>::iterator findBestOppositePositionWRTOpenStacks(vector<int>::iterator relatedPattern, vector<int>::iterator bottleneck)
{
	int i, j, min = INT_MAX;
	vector<int>::iterator bestPosition, it2;
	set<int> open;

	bestPosition = relatedPattern;

	if(*bottleneck == solution.back())
 		return bestPosition;	
	
 	stackSizeEvaluation=stackSizeOriginal;

 	for(set<int>::iterator it = patternStacks[*relatedPattern].begin(); it != patternStacks[*relatedPattern].end(); ++it)
		stackSizeEvaluation[*it]--;

	fill(stack.begin(), stack.end(), CLOSED);

	int d = distance(relatedPattern, bottleneck);
	
	if(d < 0)
	{
		for(vector<int>::iterator it = solution.begin(); it != bottleneck; ++it)
		{
			for(j=0; j<nPieces; j++)				//simulates the sequencing of the current pattern
			{
				if(inputMatrix[*it][j] > 0)
				{
					stackSizeEvaluation[j]--;		//decreases the demand for each piece of the pattern

					if(stack[j] == CLOSED)
					{
						stack[j] = OPEN;			//simulates new open stacks
						open.insert(j);
					}

					if (stackSizeEvaluation[j] == 0)
					{
						stack[j] = CLOSED;			//simulates closed stacks
						open.erase(find(open.begin(), open.end(), j));
					}
				}
			}

			vector<int> diff(nPieces*2);
			it2 = set_symmetric_difference (patternStacks[*relatedPattern].begin(), patternStacks[*relatedPattern].end(), open.begin(), open.end(), diff.begin());

			diff.resize(it2-diff.begin()); 

			if(diff.size()<min)
			{
				min = diff.size();
				bestPosition = it;
			}	
		}
	}
	else
	{
		for(vector<int>::iterator it = solution.begin(); it != solution.end(); ++it)
		{
			if(*it != *relatedPattern)
			{
				for(j=0; j<nPieces; j++)				//simulates the sequencing of the current pattern
				{
					if(inputMatrix[*it][j] > 0)
					{
						stackSizeEvaluation[j]--;		//decreases the demand for each piece of the pattern

						if(stack[j] == CLOSED)
						{
							stack[j] = OPEN;			//simulates new open stacks
							open.insert(j);
						}

						if (stackSizeEvaluation[j] == 0)
						{
							stack[j] = CLOSED;			//simulates closed stacks
							open.erase(find(open.begin(), open.end(), j));
						}
					}
				}
				
				if(it > bottleneck)
				{
					vector<int> diff(nPieces*2);
					it2 = set_symmetric_difference (patternStacks[*relatedPattern].begin(), patternStacks[*relatedPattern].end(), open.begin(), open.end(), diff.begin());

					diff.resize(it2-diff.begin()); 

					if(diff.size()<min)
					{
						min = diff.size();
						bestPosition = it;
					}					
				}
			}
		}
	}
	return bestPosition;	
}

/*
Local Search: Tries to reduce the consecutive 1s of the bottleneck columns using the best position to insert the related patterns
*/
void reduceBottleNeckConsecutive1sBestOppositePosition(int previousSolution)
{
	int bottleneck;
	int relatedPattern;
	int newSolution;
	double previousTieBreaker;

	vector<int>::iterator it3;
	vector<int>::iterator it4;
	vector<int>::iterator it5;
	set<int>::iterator it6;

	if(DONT_MOVE_BOTTLENECKS)
		for(set<int>::iterator itera = bottleneckPatterns.begin(); itera!=bottleneckPatterns.end(); ++itera){
			it6=find(bottleneckRelatedPatterns.begin(), bottleneckRelatedPatterns.end(), *itera);
			if(it6 != bottleneckRelatedPatterns.end())
				bottleneckRelatedPatterns.erase(it6);
		}

	for(set<int>::iterator it = bottleneckPatterns.begin(); it!=bottleneckPatterns.end(); ++it)	
	{
		bottleneck = *it;
		
		for(set<int>::iterator it2 = bottleneckRelatedPatterns.begin(); it2!=bottleneckRelatedPatterns.end(); ++it2)
		{
			relatedPattern = *it2;
			
			if(bottleneck != relatedPattern)
			{
				it3 = find(solution.begin(), solution.end(), bottleneck);	
				it4 = find(solution.begin(), solution.end(), relatedPattern);

				if(NEW_FIND_BEST_POSITION)
					it5=findBestOppositePositionWRTOpenStacks(it4, it3);
				if(OLD_BEST_POSITION)
					it5=findBestOppositePositionWRTPatterns(it4, it3);

				if(*it4!=*it5)
				{
					solution.erase(it4);	
					solution.insert(it5, relatedPattern);
				}

				previousTieBreaker = tieBreaker;
				newSolution = evaluation(solution, false);

				if(!BREAK_TIES)
				{
					if(newSolution > previousSolution)
					{
						solution.erase(it5);
						solution.insert(it4, relatedPattern);
					}
					else
						previousSolution = newSolution;
				}
				else
				{
					if(newSolution > previousSolution || (newSolution == previousSolution && tieBreaker > previousTieBreaker))
					{
						solution.erase(it5);
						solution.insert(it4, relatedPattern);
					}
					else
					{
						previousSolution = newSolution;
						previousTieBreaker = tieBreaker;					
					}
				}
			}
		}
	}	
}

/*
Finds the best position (minimum symmetric difference to open stacks on any side of the bottleneck) to insert the reallocated patterns 
*/
vector<int>::iterator findBestPositionWRTOpenStacks(int pattern)
{
	int i, j, min = INT_MAX;
	vector<int>::iterator bestPosition, it2;
	set<int> open;

	if(solution.empty())
		return solution.begin();

 	stackSizeEvaluation=stackSizeOriginal;

	if(REDUCE_STACK_SIZE_RELATED_PATTERNS)
	{
	 	for(set<int>::iterator it = bottleneckRelatedPatterns.begin(); it != bottleneckRelatedPatterns.end(); ++it)
	 		for(set<int>::iterator itAux = patternStacks[*it].begin(); itAux != patternStacks[*it].end(); ++itAux)
				stackSizeEvaluation[*itAux]--;
	}

	if(REDUCE_STACK_SIZE_BOTTLENECK_PATTERNS)
	{
	 	for(set<int>::iterator it = bottleneckPatterns.begin(); it != bottleneckPatterns.end(); ++it)
	 		for(set<int>::iterator itAux = patternStacks[*it].begin(); itAux != patternStacks[*it].end(); ++itAux)
				stackSizeEvaluation[*itAux]--;
	}

	fill(stack.begin(), stack.end(), CLOSED);

	for(vector<int>::iterator it = solution.begin(); it != solution.end(); it++)
	{
		for(j=0; j<nPieces; j++)				//simulates the sequencing of the current pattern
		{
			if(inputMatrix[*it][j] > 0)
			{
				stackSizeEvaluation[j]--;		//decreases the demand for each piece of the pattern

				if(stack[j] == CLOSED)
				{
					stack[j] = OPEN;			//simulates new open stacks
					open.insert(j);
				}

				if (stackSizeEvaluation[j] == 0)
				{
					stack[j] = CLOSED;			//simulates closed stacks
					open.erase(find(open.begin(), open.end(), j));
				}
			}
		}

		vector<int> diff(nPieces*2);
		it2 = set_symmetric_difference (patternStacks[pattern].begin(), patternStacks[pattern].end(), open.begin(), open.end(), diff.begin());

		diff.resize(it2-diff.begin()); 

		if(diff.size()<min)
		{
			min = diff.size();
			bestPosition = it;
		}	
	}

	return bestPosition;	
}

/*
Local Search: removes the bottlenecks from the problem and insert them in the best position with respect to the open stacks
*/
void reInsertBottleneckRelatedPatterns()
{
	vector<int>::iterator it2;
	vector<int> backup = solution;
	double previousTieBreaker;

	int before = evaluation(solution, true);
	
	//erases all bottleneck related patterns from the solution
	for(set<int>::iterator it = bottleneckRelatedPatterns.begin(); it!=bottleneckRelatedPatterns.end(); ++it)	
	{
		it2 = find(solution.begin(), solution.end(), *it);	
		solution.erase(it2);
	}

	vector<int> aux(bottleneckRelatedPatterns.begin(), bottleneckRelatedPatterns.end());
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(aux.begin(), aux.end(), std::default_random_engine(seed));

	//finds the best position w.r.t. open stacks and insert the bottleneck related patterns
	for(vector<int>::iterator it = aux.begin(); it!=aux.end(); ++it)	
	{
		it2 = findBestPositionWRTOpenStacks(*it);
		
		if(!solution.empty())
			++it2;
		
		solution.insert(it2, *it);
	}

	previousTieBreaker = tieBreaker;
	
	int after = evaluation(solution, false);

	if(BREAK_TIES)
	{
		if(before < after || (before == after && tieBreaker > previousTieBreaker))
			solution = backup;

		if (before == after && tieBreaker < previousTieBreaker)
			previousTieBreaker = tieBreaker;
	}
	else
	{
		if(before < after)
			solution = backup;
	}
}


/*
Local Search: removes the bottlenecks from the problem and insert them in the best position with respect to the open stacks
*/
void reInsertBottlenecks()
{
	vector<int>::iterator it2;

	vector<int> backup = solution;
	double previousTieBreaker;

	int before = evaluation(solution, true);

	//erases all bottlenecks from the solution
	for(set<int>::iterator it = bottleneckPatterns.begin(); it!=bottleneckPatterns.end(); ++it)	
	{
		it2 = find(solution.begin(), solution.end(), *it);	
		solution.erase(it2);
	}

	vector<int> aux(bottleneckPatterns.begin(), bottleneckPatterns.end());
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(aux.begin(), aux.end(), std::default_random_engine(seed));

	//finds the best position w.r.t. open stacks and insert the bottleneck patterns
	for(vector<int>::iterator it = aux.begin(); it != aux.end(); ++it)	
	{
		it2 = findBestPositionWRTOpenStacks(*it);
		
		if(!solution.empty())
			++it2;

		solution.insert(it2, *it);
	}

	previousTieBreaker = tieBreaker;

	int after = evaluation(solution, false);

	if(BREAK_TIES)
	{
		if(before < after || (before == after && tieBreaker > previousTieBreaker))
			solution = backup;

		if (before == after && tieBreaker < previousTieBreaker)
			previousTieBreaker = tieBreaker;
	}
	else
	{
		if(before < after)
			solution = backup;
	}
}

/*
Determines the patterns that cause consecutive 1s on the bottleneck columns
*/
void setBottleneckRelatedPatterns(set<int> bottleneckRelatedPieces)
{
	int i, j;

	for(i=0; i<nPatterns; i++)
		for(set<int>::iterator it = bottleneckRelatedPieces.begin(); it!=bottleneckRelatedPieces.end(); ++it)
			if(inputMatrix[solution[i]][*it]==1)
				bottleneckRelatedPatterns.insert(solution[i]);
}

/*
Evaluates the current solution, simulating the opened and closed stacks
*/
int evaluation(vector<int> solution, bool bottleneckCheck)
{
	int i, j;
	int index;
	int openStacks;
	int closedStacks;
	int maxOpenStacks;

	set<int> bottleneckRelatedPieces;

	tieBreaker = 0;
	openStacks = 0;
	closedStacks = 0;
	maxOpenStacks= -1;

 	stackSizeEvaluation=stackSizeOriginal;

	fill(stack.begin(), stack.end(), CLOSED);

	for(i=0; i<nPatterns; i++)
	{
		index = solution[i];					//simulates the sequencing of the current pattern

	 	for(set<int>::iterator it2 = patternStacks[index].begin(); it2 != patternStacks[index].end(); ++it2)		//simulates the decrease on the demand of each piece after the sequencing of each pattern of the solution
	 	{
			stackSizeEvaluation[*it2]--;

			if(stack[*it2] == CLOSED)
			{
				openStacks++;					//simulates new open stacks
				stack[*it2] = OPEN;
			}

			if (stackSizeEvaluation[*it2] == 0)
			{
				closedStacks++;					//simulates closed stacks
				stack[*it2] = CLOSED;
			}
	 	}

	 	if(BREAK_TIES)
	 		tieBreaker+=openStacks;

		if(CUTWIDTH)							//The cutwidth problem doesn't count just closed stacks -- or the endpoint of an edge
			openStacks -= closedStacks;

		if (openStacks > maxOpenStacks)
		{
			maxOpenStacks = openStacks;			//stores the maximum number of open stacks

			if(bottleneckCheck)
			{
				bottleneckPatterns.clear();
				bottleneckRelatedPieces.clear();

				bottleneckPatterns.insert(index);

				for(j=0; j<nPieces; j++)
					if(stack[j] == OPEN && inputMatrix[index][j]!=1)
						bottleneckRelatedPieces.insert(j);
			}
		}
		else if(openStacks == maxOpenStacks && bottleneckCheck)
			{
				bottleneckPatterns.insert(index);

				for(j=0; j<nPieces; j++)
					if(stack[j] == OPEN && inputMatrix[index][j]!=1)
						bottleneckRelatedPieces.insert(j);
			}

		if(!CUTWIDTH)	
			openStacks -= closedStacks;
		closedStacks = 0;
	}
	
	if(bottleneckCheck)
		setBottleneckRelatedPatterns(bottleneckRelatedPieces);

	if(BREAK_TIES)
	{
	 	tieBreaker/=(nPatterns*maxOpenStacks);
	 	tieBreaker+=maxOpenStacks;
	 	//cout<<maxOpenStacks<<"-"<<tieBreaker<<endl;
	 	//getchar();
	}

	return maxOpenStacks;						//returns the maximum number of open stacks
}

/*
Gives the pattern sequencing correspondent to the pieces sequence, Yanasse's version
*/
void patternSequencingYanasse()
{
	int i;
	int j;
	int index;

	index = nPatterns-1;

	for(i = nPieces-1; index>=0; i--)										//traverses the pieces sequence in reversal order
	{
		for(j=0; (j<nPatternsOriginal)&&(index>=0); j++)					//finds the pattern that include the current piece
		{
			if((!pattern[j].isSequenced())&&(!pattern[j].isDominated()))	//if the pattern wasn't	already sequenced and wasn't removed by the preprocessing
			{
				if(inputMatrix[j][nodeSequence[i]] == 1)
				{
					pattern[j].setSequenced(true);
					solution[index--] = j;									//inserts it in the pattern sequence from the end to the beginning
				}
			}
		}
	}

    for(i=0; i<nPatternsOriginal; i++)
    	pattern[i].setSequenced(false);
}

/*
 Gives the pattern sequencing correspondent to the piece sequence, as in Becceneri (2004)
 */
void patternSequencingBecceneri()
{
	int i;
	int j;
	int index = 0;
    vector<int> completion;
    
    completion.resize(nPatternsOriginal);
    
    for(i=0; i<nPatternsOriginal; i++)
    {
        for(j = 0; j<nPieces; j++)
           if((!pattern[i].isDominated())&&(inputMatrix[i][j] == 1))	 	
              completion[i]++;
    }
    
	for(i = nPieces-1; i>=0; i--)//trocar do início para o final								
    {
		for(j=0; j<nPatternsOriginal; j++) 
        {
			if((!pattern[j].isSequenced())&&(!pattern[j].isDominated()))		
			{
				if(inputMatrix[j][nodeSequence[i]] == 1)
				{
                    completion[j]--;
                    
                    if(completion[j] == 0)
                    {
                       pattern[j].setSequenced(true);
					   solution[index++] = j;								//inserts it in the pattern sequence from the left to the right
                    }
				}
			}
		}
	}
    
    for(i=0; i<nPatternsOriginal; i++)
    	pattern[i].setSequenced(false);
}

/*
Finds the minimum degree node not reached by the BFS
*/
int findMinimumDegree()
{
	int index  = -1;
	int i;
	int smaller = nPieces*nPieces+1;

	for(i=0; i<nPieces; i++)
	{
		if((node[i].getDegree() < smaller)&&(!node[i].wasVisited()))	
		{
			smaller = node[i].getDegree();								//selects the node of smaller degree not sequenced yet
			index = i;													//stores its index
		}
	}

	return index;
}

/*
Compares the degrees of two nodes, used in sort()
*/
bool compareDegree(const int& first, const int& second)
{

	if(degree[first] < degree[second])
		return true;
	return false;
}

/*
Explore each component of the MOSP graph, used by the BFS
*/
void exploreComponent(int currentNode)
{
	list<int> Q;
	list<int> currentNeighborhood;
	int i;

	Q.push_back(currentNode);

	while(!Q.empty())
	{
		currentNode = Q.front();
		nodeSequence.push_back(currentNode);
		Q.pop_front();	

		for(i=0; i<currentNode && node[currentNode].getDegree()>0; i++)		//searches as long as the degree of the node allow	
		{
			if(adjacencyMatrix[i][currentNode] == 1)						//searches for its neighbours
			{
				if(!node[i].wasVisited())									//if they are not in queue or pieces sequence	
				{
					node[i].setVisited(true);								//blocks the neighbour
					currentNeighborhood.push_back(i);	
				}
			}
		}

		for(i=currentNode+1; i<nPieces&&node[currentNode].getDegree()>0; i++) //same as the loop above, but the indexes of the adjacency matrix are reversed (remember it is an upper diagonal matrix)
		{
			if(adjacencyMatrix[currentNode][i] == 1)
			{
				if(!node[i].wasVisited())
				{
					node[i].setVisited(true);
					currentNeighborhood.push_back(i);				
				}
			}
		}

		node[currentNode].setVisited(true);									//removes the dequeued piece from the problem
		currentNeighborhood.sort(compareDegree);

		Q.insert(Q.end(), currentNeighborhood.begin(), currentNeighborhood.end());	
		currentNeighborhood.clear();
	}
}

/*
Breadth-First Search - BFS
*/
void BFS()
{
	while(nodeSequence.size() < nPieces)
		exploreComponent(findMinimumDegree());
}

/*
Breadth-First Search - overloaded version, the parameter is the initial node
*/
void BFS(int start)
{
	while(nodeSequence.size() < nPieces)
		exploreComponent(start);
}

/*
Breadth-First Search - starting from each node
*/
void MultipleStartBFS()
{
	int best = INT_MAX;
	vector<int> aux(nPieces);
	vector<int> becceneri(nPatterns);
	vector<int> yanasse(nPatterns);
	int BecceneriSolutionValue;
	int YanasseSolutionValue;

	BFS();
	aux = nodeSequence;	

	for(int i=0; i<nPieces; i++)
	{
		BFS(i);

		BecceneriSolutionValue = evaluation(becceneri, false);
		YanasseSolutionValue = evaluation(yanasse, false);

		if(BecceneriSolutionValue < YanasseSolutionValue && BecceneriSolutionValue < best)
		{
			best = BecceneriSolutionValue;
			aux = nodeSequence;
		}
		else if (BecceneriSolutionValue > YanasseSolutionValue && YanasseSolutionValue < best)
		{
			best = YanasseSolutionValue;
			aux = nodeSequence;
		}

		for(int j = 0; j<nPieces; j++)
		{
			node[j].setVisited(false);
		}
	}
	nodeSequence = aux;
}

/*
 Pre-processing procedure, detects and removes dominated patterns
 */
void dominancePreProcessing()
{
	int i;
	int j;
	int k;
	int flag = 1;
	int firstPattern;
	int secondPattern;
	int dominated = 0;
    
	for(i=0; i<nPatterns; i++)
	{
		if(!pattern[i].isDominated())	
		{
			for(j=i+1; j<nPatterns; j++)
			{
				if(!pattern[j].isDominated())
				{
					if(pattern[i].getSize() < pattern[j].getSize())						//if pattern i is smaller than pattern j	
					{
						firstPattern = i;
						secondPattern = j;
						for(k=0; k<nPieces; k++)
						{
							if ((inputMatrix[i][k] == 1) && (inputMatrix[j][k] == 0))	//and pattern i has a piece that pattern j doesn't
							{
								flag = 0;												//then i is not dominated by j
								break;
							}
						}
					}
					else	if(pattern[i].getSize() == pattern[j].getSize())			//if patterns i and j have the same size	
                    {
                        firstPattern = j;
                        secondPattern = i;
                        for(k=0; k<nPieces; k++)
                        {
                            if (inputMatrix[i][k] != inputMatrix[j][k])					//and they differ in any position
                            {
                                flag = 0;												//then no one is dominated
                                break;
                            }
                        }
                    }
                    else	if((pattern[i].getSize() > pattern[j].getSize()))			//if pattern i is larger than pattern j	
                    {
                        firstPattern = j;
                        secondPattern = i;
                        for(k=0; k<nPieces; k++)
                        {
                            if ((inputMatrix[i][k] == 0) && (inputMatrix[j][k] == 1)) 	//and j has a piece that i doesn't
                            {
                                flag = 0;												//then j is not dominated by i
                                break;
                            }
                        }
                    }
                    
					if (flag == 1)														//if the a dominance condition is met
					{
						pattern[firstPattern].setDominated(true);						//the pattern is removed
						dominated++;
                        
						dominatedPatterns[secondPattern].push_back(firstPattern);
                        
						if(dominatedPatterns[firstPattern].size() > 0)					//if the dominated pattern dominates other patterns
							dominatedPatterns[secondPattern].splice(dominatedPatterns[secondPattern].end(), dominatedPatterns[firstPattern]);		//the list is transferred

						if(firstPattern == i)											//if the first case was selected, then firstPattern can't dominate another pattern
							break;
					}
				}
                
				flag = 1;
			}
		}
	}
    
	nPatternsOriginal = nPatterns;														//stores the original number of patterns
	nPatterns -= dominated;																//updates the number of patterns

	solution.resize(nPatterns);
}

/*
Local Search: Tries to postpone the opening of new stacks as much as possible, eventually using a random order of pieces
*/
int forwardImprovement(int bestValue)
{
	int j;
	int reallocatedPattern;
	int solutionValue;
	int position; 

	stackSize = stackSizeOriginal;

	for(vector<int>::iterator it=solution.begin(); it != solution.end()-1; ++it)
	{
		for(j=0; j<nPieces; j++)
			if(inputMatrix[*it][j] > 0)
				stackSize[j]--;																					//simulates the decrease on the demand of each piece after the sequencing of each pattern of the solution

		for(vector<int>::iterator itPiece = pieces.begin(); itPiece != pieces.end(); ++itPiece)	
		{	
			if ((stackSize[*itPiece] == lookahead) && (inputMatrix[*(it+1)][*itPiece] != 1))					//if a stack is about to close
			{
				position = 1;																					//defines the offset from the analyzed pattern for reallocating the next pattern

				for(vector<int>::iterator it2=it+1; it2!= solution.end(); ++it2)								//searches for the pattern that will close that stack
				{
					if(inputMatrix[*it2][*itPiece] == 1)														//when found
					{
						reallocatedPattern = *it2;
						solution.erase(it2);
						solution.insert(it+position, reallocatedPattern);										//reallocates the pattern
						solutionValue = evaluation(solution, false);											//evaluates the modified solution

						if(solutionValue <= bestValue)															//if there's no worsening of the solution
						{
							bestValue = solutionValue;															//stores the value found
							position++;																			//next reallocation position is set
						}
						else																					//if the solution is worsen
						{
							stackSize[*itPiece] = 0;		

							solution.erase(it+position);														//rollback the reallocation
							solution.insert(it2, reallocatedPattern);					
						}
                        //position++;																			//next reallocation position is set
					}
				}
			}
		}
	}

	return bestValue;
}

/*
Local Search: Tries to anticipate the closure of stacks as much as possible, eventually using a random order of pieces
*/
int backwardImprovement(int bestValue)
{
	int j;
	int reallocatedPattern;
	int solutionValue;
	int position;																		//determines the relative position where patterns will be reallocated to
	
	stackSize = stackSizeOriginal;

	for(vector<int>::reverse_iterator it=solution.rbegin(); it != solution.rend(); ++it)
	{
		for(j=0; j<nPieces; j++)
			if(inputMatrix[*it][j] > 0)
				stackSize[j]--;															//simulates the decrease on the demand of each piece after the sequencing of each pattern of the solution
			
		for(vector<int>::iterator itPiece = pieces.begin(); itPiece != pieces.end(); ++itPiece)	
		{
			if ((stackSize[*itPiece] == lookahead) && (inputMatrix[*(it+1)][*itPiece] != 1))			//if a stack is about to open
			{
				position = 2;															//defines the offset from the analyzed pattern for reallocating the next pattern

				for(vector<int>::reverse_iterator it2 =it+1; it2!= solution.rend(); ++it2)//searches for the pattern that will open that stack
				{
					if(inputMatrix[*it2][*itPiece] == 1)										//when found
					{
						reallocatedPattern = *it2;

						solution.erase(it2.base()-1);
						solution.insert(it.base()-position, reallocatedPattern);		//reallocates the pattern
						solutionValue = evaluation(solution, false);					//evaluates the modified solution

						if(solutionValue <= bestValue)									//if there's no worsening of the solution
						{
							bestValue = solutionValue;									//stores the value found
							position++;														//next reallocation position is set
						}
						else															//if the solution is worsen
						{
							stackSize[*itPiece] = 0;
							solution.erase(it.base()-position);							//rollback the reallocation
							solution.insert(it2.base()-1, reallocatedPattern);			
						}

						//position++;														//next reallocation position is set
					}
				}
				
			}
		}
	}

	return bestValue;
}

/*
Local Search: Tries to reduce discontinuities by postponing the opening of new stacks as much as possible inserting the related patterns in the best position w.r.t. the open stacks, eventually using a random order of pieces and patterns
*/
int forwardImprovementWRTOpenStacks(int bestValue)
{
	int j;
	int reallocatedPattern;
	int solutionValue;
	double previousTieBreaker;

	stackSize = stackSizeOriginal;

	for(vector<int>::iterator it=solution.begin(); it != solution.end()-1; ++it)
	{
	 	for(set<int>::iterator it2 = patternStacks[*it].begin(); it2 != patternStacks[*it].end(); ++it2)		//simulates the decrease on the demand of each piece after the sequencing of each pattern of the solution
			stackSize[*it2]--;

		for(vector<int>::iterator itPiece = pieces.begin(); itPiece != pieces.end(); ++itPiece)	
		{	
			if(stackSizeOriginal[*itPiece] > 0)																	//some VLSI instances (namely, x6) have unused pieces
				lookahead = ((lookahead_counter) % stackSizeOriginal[*itPiece])+1;

			if ((stackSize[*itPiece] == lookahead) && (inputMatrix[*(it+1)][*itPiece] != 1))					//if a stack is about to close
			{
				vector<int> stackPatternsVector(stackPatterns[*itPiece].begin(), stackPatterns[*itPiece].end());
				
				if(RANDOM_PATTERNS_FI)
				{
					unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
					shuffle(stackPatternsVector.begin(), stackPatternsVector.end(), std::default_random_engine(seed));
				}

				for(vector<int>::iterator it3 = stackPatternsVector.begin(); it3 != stackPatternsVector.end(); ++it3)	
				//for(set<int>::iterator it3 = stackPatterns[*itPiece].begin(); it3 != stackPatterns[*itPiece].end(); ++it3)
				{
					vector<int>::iterator it2 = find(solution.begin(), solution.end(), *it3);

					if(it2 > it)
					{	
						vector<int>::iterator itPosition = findBestOppositePositionWRTOpenStacks(it2, it);							//defines the position for reallocating the next pattern

						reallocatedPattern = *it2;
						solution.erase(it2);
						solution.insert(itPosition, reallocatedPattern);										//reallocates the pattern
						previousTieBreaker = tieBreaker;
						solutionValue = evaluation(solution, false);											//evaluates the modified solution

						if(!BREAK_TIES)	
						{
							if(solutionValue <= bestValue)															//if there's no worsening of the solution
							{
								bestValue = solutionValue;															//stores the value found
							}
							else																					//if the solution is worsen
							{
								//stackSize[*itPiece] = 0;															//this stack won't be analyzed again in this procedure call (it would be, as the stack size match the lookahead)
								solution.erase(itPosition);															//rollback the reallocation
								solution.insert(it2, reallocatedPattern);					
							}
						}
						else
						{
							if(solutionValue < bestValue || (solutionValue == bestValue && tieBreaker < previousTieBreaker))															//if there's no worsening of the solution
							{
								bestValue = solutionValue;															//stores the value found
								previousTieBreaker = tieBreaker;	
							}
							else																					//if the solution is worsen
							{
								//stackSize[*itPiece] = 0;															//this stack won't be analyzed again in this procedure call (it would be, as the stack size match the lookahead)
								solution.erase(itPosition);															//rollback the reallocation
								solution.insert(it2, reallocatedPattern);					
							}							
						}
					}
				}
				stackSize[*itPiece] = 0;
			}
		}
	}

	return bestValue;
}

/*
Local Search: Tries to reduce discontinuities by anticipating the closure of stacks as much as possible inserting the related patterns in the best position w.r.t. the open stacks, eventually using a random order of pieces and patterns
*/
int backwardImprovementWRTOpenStacks(int bestValue)
{
	int j;
	int reallocatedPattern;
	int solutionValue;
	double previousTieBreaker;	
	stackSize = stackSizeOriginal;

	for(vector<int>::reverse_iterator it=solution.rbegin(); it != solution.rend()-1; ++it)
	{

	 	for(set<int>::iterator it2 = patternStacks[*it].begin(); it2 != patternStacks[*it].end(); ++it2)					//simulates the decrease on the demand of each piece after the sequencing of each pattern of the solution
			stackSize[*it2]--;

		for(vector<int>::iterator itPiece = pieces.begin(); itPiece != pieces.end(); ++itPiece)	
		{
			if(stackSizeOriginal[*itPiece] > 0)																				//some VLSI instances (namely, x6) have unused pieces
				lookahead = ((lookahead_counter) % stackSizeOriginal[*itPiece])+1;

			if ((stackSize[*itPiece] == lookahead) && (inputMatrix[*(it+1)][*itPiece] != 1))								//if a stack is about to open
			{
				vector<int> stackPatternsVector(stackPatterns[*itPiece].begin(), stackPatterns[*itPiece].end());
				
				if(RANDOM_PATTERNS_BI)
				{
					unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
					shuffle(stackPatternsVector.begin(), stackPatternsVector.end(), std::default_random_engine(seed));
				}

				for(vector<int>::iterator it3 = stackPatternsVector.begin(); it3 != stackPatternsVector.end(); ++it3)	
				//for(set<int>::iterator it3 = stackPatterns[*itPiece].begin(); it3 != stackPatterns[*itPiece].end(); ++it3) 	//searches for the pattern that will open that stack
				{
					vector<int>::iterator it_aux = find(solution.begin(), solution.end(), *it3);
					vector<int>::reverse_iterator it2(it_aux+1);

					if(it2 > it)
					{
						vector<int>::iterator itPosition = findBestOppositePositionWRTOpenStacks(it2.base()-1, it.base()-1);					//defines the position for reallocating the next pattern
						reallocatedPattern = *it2;
						solution.erase(it2.base()-1);
						solution.insert(itPosition, reallocatedPattern);													//reallocates the pattern
						previousTieBreaker = tieBreaker;
						solutionValue = evaluation(solution, false);														//evaluates the modified solution

						if(!BREAK_TIES)	
						{
							if(solutionValue <= bestValue)																		//if there's no worsening of the solution
							{
								bestValue = solutionValue;																		//stores the value found
							}
							else																								//if the solution is worsen
							{
								//stackSize[*itPiece] = 0;																		//this stack won't be analyzed again in this procedure call (it would be, as the stack size match the lookahead)
								solution.erase(itPosition);																		//rollback the reallocation
								solution.insert(it2.base()-1, reallocatedPattern);			
							}
						}
						else
						{
							if(solutionValue < bestValue || (solutionValue == bestValue && tieBreaker < previousTieBreaker))																		//if there's no worsening of the solution
							{
								bestValue = solutionValue;																		//stores the value found
								previousTieBreaker = tieBreaker;	
							}
							else																								//if the solution is worsen
							{
								//stackSize[*itPiece] = 0;																		//this stack won't be analyzed again in this procedure call (it would be, as the stack size match the lookahead)
								solution.erase(itPosition);																		//rollback the reallocation
								solution.insert(it2.base()-1, reallocatedPattern);			
							}							
						}
					}
				}	
				stackSize[*itPiece] = 0;																		//this stack won't be analyzed again in this procedure call (it would be, as the stack size match the lookahead)
			}
		}
	}

	return bestValue;
}

/*
Terminates all data structures. 
*/
void termination()
{
	int i;

	for(i=0; i<nPatternsOriginal; i++)
	{
		dominatedPatterns[i].clear();
		inputMatrix[i].clear();
	}

	for(i=0; i<nPieces; i++)
		adjacencyMatrix[i].clear();

	adjacencyMatrix.clear();	
	dominatedPatterns.clear();
	inputMatrix.clear();
	patternStacks.clear();
	stackPatterns.clear();
	pattern.clear();
	stack.clear();
	stackSize.clear();
	stackSizeOriginal.clear();
	stackSizeEvaluation.clear();
	solution.clear();
    alternativeSolution.clear();
	node.clear();
	nodeSequence.clear();
	degree.clear();
	Q.clear();

	bottleneckPatterns.clear();
	bottleneckRelatedPatterns.clear();

	pieces.clear();
}

/*
Prints the solution matrix (patterns (products) x pieces (customers)), including dominated patterns, to the output file
*/
void printPatternsPiecesMatrix(FILE *fp)
{
	fprintf(fp, "\nSolution Matrix (Patterns (Products) x Pieces (Customers)):\n");

	for(int i=0; i<nPatterns; i++)
	{
		for(int j=0; j<nPieces; j++)
			fprintf(fp, "%d ", inputMatrix[solution[i]][j]);

		fprintf(fp, "\n");

		if(dominatedPatterns[solution[i]].size() > 0)				  				//prints the dominated patterns right after the patterns they are dominated by	
			for (list<int>::iterator it=dominatedPatterns[solution[i]].begin(); it != dominatedPatterns[solution[i]].end(); ++it) 
			{
				for(int k=0; k<nPieces; k++)	
					fprintf(fp, "%d ", inputMatrix[*it][k]);

				fprintf(fp, "\n");
			}
	}
}

/*
Prints the solution matrix (pieces (customers) x patterns (products)), including dominated patterns, to the output file
*/
void printPiecesPatternsMatrix(FILE *fp)
{
	int i;

	fprintf(fp, "\nSolution Matrix (Pieces (Customers) x Patterns (Products)):\n");

	for(int j=nPieces-1; j>=0; j--)
	{
		for(i=0; i<nPatterns; i++)
		{
			fprintf(fp, "%d ", inputMatrix[solution[i]][j]);
		
			if(dominatedPatterns[solution[i]].size() > 0)				  				//prints the dominated patterns right after the patterns they are dominated by	
				for (list<int>::iterator it=dominatedPatterns[solution[i]].begin(); it != dominatedPatterns[solution[i]].end(); ++it) 
					fprintf(fp, "%d ", inputMatrix[*it][j]);
		}
		
		fprintf(fp, "\n");
	}
}

/*
Prints the solution information to the output file
*/
void printSolution(char *inputFileName, int FinalSolutionValue, double time)
{
	FILE *fpSolution;

    char outputFileName[256];

	sprintf(outputFileName, "Solution_%s", inputFileName);

	fpSolution = fopen(outputFileName, "w");				//file that contains the information about the solution of a problem instance

	fprintf(fpSolution, "Algorithm parameters\n");
/*
	fprintf(fpSolution, "READ_ORIGINAL:%s\n", READ_ORIGINAL?"true":"false");
	fprintf(fpSolution, "LOOKAHEAD:%s\n", LOOKAHEAD?"true":"false");
	fprintf(fpSolution, "LOOKAHEAD_YANASSE:%s\n", LOOKAHEAD_YANASSE?"true":"false");
	fprintf(fpSolution, "LOOKAHEAD_BECCENERI:%s\n", LOOKAHEAD_BECCENERI?"true":"false");
	fprintf(fpSolution, "CUTWIDTH:%s\n", CUTWIDTH?"true":"false");
	fprintf(fpSolution, "REDUCE_BOTTLENECK_LOOP:%s\n", REDUCE_BOTTLENECK_LOOP?"true":"false");
	fprintf(fpSolution, "REDUCE_BOTTLENECK_LOOP_SIZE:%s\n", REDUCE_BOTTLENECK_LOOP_SIZE?"true":"false");
	fprintf(fpSolution, "REDUCE_BOTTLENECK:%s\n", REDUCE_BOTTLENECK?"true":"false");
	fprintf(fpSolution, "REDUCE_BOTTLENECK_BEST_POSITION:%s\n", REDUCE_BOTTLENECK_BEST_POSITION?"true":"false");
	fprintf(fpSolution, "RANDOM_PIECES:%s\n", RANDOM_PIECES?"true":"false");
	fprintf(fpSolution, "RANDOM_PATTERNS_BOTTLENECK:%s\n", RANDOM_PATTERNS_BOTTLENECK?"true":"false");
	fprintf(fpSolution, "MULTIPLE_START_BFS:%s\n", MULTIPLE_START_BFS?"true":"false");
	fprintf(fpSolution, "THRESHOLD_PERCENTAGE:%.2f\n", THRESHOLD_PERCENTAGE);
*/	
	fprintf(fpSolution, "\nMaximum Open Stacks: %d\nExecution time: %lf seconds\n", FinalSolutionValue, time);
	fprintf(fpSolution, "Number of Customers (Pieces): %d\nNumber of Products (Patterns): %d\nProducts (Patterns) eliminated in the Pre-processing: %d\n", nPieces, nPatternsOriginal, nPatternsOriginal-nPatterns);
	fprintf(fpSolution, "Products (Patterns) Sequence:\n");

	for(int i=0; i<nPatterns; i++)
	{
		fprintf(fpSolution, "%d \n", solution[i]+1);								//prints the indexes (from 0 to nPatterns-1) of the patterns, one per line

		if(dominatedPatterns[solution[i]].size() > 0)				   				//prints the dominated patterns right after the patterns they are dominated by																					
			for (list<int>::iterator it=dominatedPatterns[solution[i]].begin(); it != dominatedPatterns[solution[i]].end(); ++it)
    			fprintf(fpSolution, "%d [dominated]\n", *it+1);						//after the index of the pattern the word [dominated] is written to indicate it
	}

	printPatternsPiecesMatrix(fpSolution);											//prints the patterns (products) x pieces (customers) solution matrix
	printPiecesPatternsMatrix(fpSolution);											//prints the patterns pieces (customers) x (products) solution matrix	
	
	fclose(fpSolution);
}

/*
Generates all permutations for a given instance, used for verification purposes
*/
int generateAllSolutions(char *inputFileName)
{
	int i = 0;
	int solutionValue;
	int bestSolutionValue = INT_MAX;

	patternSequencingBecceneri();
	
	sort(solution.begin(), solution.end());
	do 
	{
		solutionValue = evaluation(solution, false);							//evaluates the solution

		if(solutionValue < bestSolutionValue)
		{
			printSolution(inputFileName, solutionValue, 0);						//prints the solution to the file
			bestSolutionValue = solutionValue;
		}
	}while (next_permutation(solution.begin(), solution.end()));

	return bestSolutionValue;
}

/*
Sets the lowerbound as the size of the largest pattern
*/
void setLowerBound()
{
	for(int i=0; i<nPatterns; i++)
		if(pattern[i].getSize() > largestPatternSize)
			largestPatternSize = pattern[i].getSize();
}

/*
Comparison of local search procedures
*/
void localSearch(int *LookaheadSolutionValue, double *LookaheadRunningTime, char *inputFileName, int instance)
{
	int FinalSolutionValue = INT_MAX;
	int BecceneriSolutionValue = INT_MAX;
	int ImprovementSolutionValue = INT_MAX;

	readProblemMatrix(inputFileName);											//reads the problem data

	high_resolution_clock::time_point t1 = high_resolution_clock::now();		//time taking
	
	setLowerBound();															//determines the largest pattern size
   	dominancePreProcessing();													//preprocessing procedure, removes dominated patterns
   	buildMOSPGraph();															//builds the mosp graph

    if(!MULTIPLE_START_BFS)
    	BFS();																	//runs the BFS
    else
	   MultipleStartBFS();

	patternSequencingBecceneri();												//sequences the patterns, Becceneri's method
	BecceneriSolutionValue = evaluation(solution, true);						//evaluates the solution

	patternSequencingYanasse();													//sequences the patterns, Yanasse's method    
    FinalSolutionValue = evaluation(solution, true);				//evaluates the solution
    
	if(BecceneriSolutionValue<FinalSolutionValue)
	{
		patternSequencingBecceneri();											//sequences the patterns, Becceneri's method
		FinalSolutionValue = evaluation(solution, true);			//evaluates the solution
	}

	if(LS_RULE1 || LS_RULE2 || LS_BOTH_RULES || LS_ALL)
	{
		for(int i=0; i<nPieces; i++)	
			pieces.push_back(i);

		if(RANDOM_PIECES)
		{
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(pieces.begin(), pieces.end(), std::default_random_engine(seed));
		}		
	}


	//Local Search Procedures
	if(LS_REDUCE_BOTTLENECK)
	{
		for(int i=0 ; i<LOOP_SIZE; i++)
		{
			reduceBottleNeckConsecutive1s(FinalSolutionValue);
			bottleneckPatterns.clear();
			bottleneckRelatedPatterns.clear();			
			FinalSolutionValue = evaluation(solution, true);
			if (FinalSolutionValue == largestPatternSize)										//if lowerbound is reached, ends the heuristic
				goto ENDLS;
		}
	}
	else if(LS_REDUCE_BOTTLENECK_BEST_POSITION)
		{
			for(int i=0 ; i<LOOP_SIZE; i++)
			{
				//personalizar com findbestposition2 ou não
				reduceBottleNeckConsecutive1sBestOppositePosition(FinalSolutionValue);
				bottleneckPatterns.clear();
				bottleneckRelatedPatterns.clear();
				FinalSolutionValue = evaluation(solution, true);
				if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
					goto ENDLS;

			}
		}
		else if(LS_REMOVE_BOTTLENECK_RELATED_PATTERNS)
			{
				for(int i=0 ; i<LOOP_SIZE; i++)
				{
					reInsertBottleneckRelatedPatterns();
					bottleneckPatterns.clear();
					bottleneckRelatedPatterns.clear();
					FinalSolutionValue = evaluation(solution, true);
					if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
						goto ENDLS;
				}
			}
			else if(LS_REMOVE_BOTTLENECKS)
				{
					for(int i=0 ; i<LOOP_SIZE; i++)
					{
						reInsertBottlenecks();
						bottleneckPatterns.clear();
						bottleneckRelatedPatterns.clear();
						FinalSolutionValue = evaluation(solution, true);
						if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
							goto ENDLS;
					}					
				}
				else if(LS_RULE1)
					{
						for(int i=0 ; i<LOOP_SIZE; i++)
						{
							lookahead = 1;

							ImprovementSolutionValue = forwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution

 				            if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
				              FinalSolutionValue = ImprovementSolutionValue;					//updates it

							if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
								goto ENDLS;

							lookahead++;
						}
					}
					else if(LS_RULE2)
						{
							lookahead = 1;

							for(int i=0 ; i<LOOP_SIZE; i++)
							{
								ImprovementSolutionValue = backwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
	           
	           					if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
	              					FinalSolutionValue = ImprovementSolutionValue;					//updates it
								
								if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
									goto ENDLS;

								lookahead++;
							}
						}
						else if(LS_BOTH_RULES)
							{
								lookahead = 1;
								for(int i=0 ; i<LOOP_SIZE; i++)
								{
									ImprovementSolutionValue = forwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
	           
 				            		if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
				              			FinalSolutionValue = ImprovementSolutionValue;					//updates it

									ImprovementSolutionValue = backwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
			           
		 				            if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
						              FinalSolutionValue = ImprovementSolutionValue;					//updates it

									if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
										goto ENDLS;

									lookahead++;
								}
							}
							else  if(LS_ALL)
									{
										int previousSolution = INT_MAX;
										int plateau = 0;

										lookahead_counter = 1;
										
										for(int i=0 ; i<LOOP_SIZE; i++)
										{	
											//kOpt(4);				
											ImprovementSolutionValue = forwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
	           								//getchar();
		 				            		if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
						              			FinalSolutionValue = ImprovementSolutionValue;					//updates it

											ImprovementSolutionValue = backwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
					           
				 				            if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
								              FinalSolutionValue = ImprovementSolutionValue;					//updates it

								          	if(FinalSolutionValue < previousSolution)
								          	{
								          		previousSolution = FinalSolutionValue;		
								            	plateau = 0;
				 				            }
								          	else
								          		plateau++;
						              		
											reduceBottleNeckConsecutive1s(FinalSolutionValue);
											reduceBottleNeckConsecutive1sBestOppositePosition(FinalSolutionValue);
											reInsertBottleneckRelatedPatterns();
											reInsertBottlenecks();
											
											bottleneckPatterns.clear();
											bottleneckRelatedPatterns.clear();			
											
											FinalSolutionValue = evaluation(solution, true);

											if(!CUTWIDTH)
												if (FinalSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
													goto ENDLS;

											if(BEST_SOLUTION_PRUNING)
											{
												if(GMPLAN)
													if(FinalSolutionValue ==  GMPLAN_Solution[instance])
														goto ENDLS;

												if(SCOOP)
													if(FinalSolutionValue ==  SCOOP_Solution[instance])
														goto ENDLS;

												if(CHU_STUCKEY)
													if(FinalSolutionValue ==  CHU_STUCKEY_Solution[instance])
														goto ENDLS;

												if(CHALLENGE)
													if(FinalSolutionValue ==  CHALLENGE_Solution[instance])
														goto ENDLS;

												if(SMALL)	
													if(FinalSolutionValue ==  SMALL_Solution[instance])
														goto ENDLS;

												if(GRIDS)	
													if(FinalSolutionValue ==  GRIDS_Solution[instance])
														goto ENDLS;													

												if(FAGGIOLI_BENTIVOGLIO)	
													if(FinalSolutionValue ==  FB_Solution[instance])
														goto ENDLS;													
											}

											if(plateau ==  PLATEAU_LIMIT)	
											{
												cout<<"--->"<<i<<endl;
												goto ENDLS;
											}

											lookahead_counter++;	
										}
										cout<<"Terminou por vias normais"<<endl;
										getchar();								
									}
	ENDLS:

	high_resolution_clock::time_point t2 = high_resolution_clock::now(); 		//time taking
  	duration<double> time_span = duration_cast<duration<double> >(t2 - t1);

	*LookaheadSolutionValue = FinalSolutionValue;                             	//stores the solution value
	*LookaheadRunningTime =  time_span.count();									//stores the execution time

	printSolution(inputFileName, *LookaheadSolutionValue, *LookaheadRunningTime);//prints the solution to the file

	termination();																//terminates all data structures
}

/*
Main procedure of the constructive heuristic
Parameters:
LookaheadSolutionValue	stores the solution value obtained by the heuristic
LookaheadRunningTime	stores the exeution time of the heuristic
inputFileName			stores the name of the file containing the instance
*/
void mainMethod(int *LookaheadSolutionValue, double *LookaheadRunningTime, char *inputFileName)
{
	int i, j;
	int FinalSolutionValue = INT_MAX;
	int BecceneriSolutionValue = INT_MAX;
	int ImprovementSolutionValue;
    int threshold;

	readProblemMatrix(inputFileName);											//reads the problem data
	
	high_resolution_clock::time_point t1 = high_resolution_clock::now();		//time taking
	
	setLowerBound();															//determines the largest pattern size
   	dominancePreProcessing();													//preprocessing procedure, removes dominated patterns
   	buildMOSPGraph();															//builds the mosp graph
    
    if(!MULTIPLE_START_BFS)
    	BFS();																	//runs the BFS
    else
	   MultipleStartBFS();

	if(GENERATE_ALL_SOLUTIONS)
	{
		*LookaheadSolutionValue = generateAllSolutions(inputFileName);              //stores the solution value
		*LookaheadRunningTime =  0;													//stores the execution time
		termination();
		return;
	}

	if(LOOKAHEAD)
	{
	   	threshold = THRESHOLD_PERCENTAGE*max(nPatterns, 100);
		threshold = min(nPatterns, threshold);

	   	patternSequencingBecceneri();												//sequences the patterns, Becceneri's method			   	
	   	BecceneriSolutionValue = evaluation(solution, BOTTLENECK_CHECK);			//evaluates the solution

		patternSequencingYanasse();													//sequences the patterns, Yanasse's method    
    	FinalSolutionValue = evaluation(solution, BOTTLENECK_CHECK);				//evaluates the solution

		if(BecceneriSolutionValue>FinalSolutionValue)
			BecceneriSolutionValue = FinalSolutionValue;
		else
		{
			patternSequencingBecceneri();												//sequences the patterns, Becceneri's method
			BecceneriSolutionValue = evaluation(solution, BOTTLENECK_CHECK);			//evaluates the solution
		}

	   	if(REDUCE_BOTTLENECK)
   			reduceBottleNeckConsecutive1s(BecceneriSolutionValue);
   		if(REDUCE_BOTTLENECK_BEST_POSITION)
   			reduceBottleNeckConsecutive1sBestOppositePosition(BecceneriSolutionValue);

	   	if(REDUCE_BOTTLENECK_LOOP)   	
		   	for(i=0; i<REDUCE_BOTTLENECK_LOOP_SIZE; i++)
		    {
				BecceneriSolutionValue = evaluation(solution, true);
		    	
		    	if(REDUCE_BOTTLENECK_BEST_POSITION)
		    		reduceBottleNeckConsecutive1sBestOppositePosition(BecceneriSolutionValue);
		    	if(REDUCE_BOTTLENECK)
		    		reduceBottleNeckConsecutive1s(BecceneriSolutionValue);
		    	
		    	bottleneckPatterns.clear();
		    	bottleneckRelatedPatterns.clear();
		    }

	   	if (BecceneriSolutionValue == largestPatternSize)									//if lowerbound is reached, ends the heuristic
	   		goto END;

	   	lookahead = 1;	//determines how many pieces to look ahead on pattern reallocation
	   																				
		for(i=0; i<nPieces; i++)	
				pieces.push_back(i);

		if(RANDOM_PIECES)
		{
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(pieces.begin(), pieces.end(), std::default_random_engine(seed));
		}

	    for(i=0; i<threshold; i++)
	    {
	        lookahead = 1;															//starts by looking 1 piece ahead
	       
	        for(j=0; j<threshold; j++)
	        {
	           ImprovementSolutionValue = forwardImprovementWRTOpenStacks(BecceneriSolutionValue);	//applies scheduling rule 1 to the current solution

	           if(ImprovementSolutionValue < BecceneriSolutionValue)				//if the solution was improved
	              BecceneriSolutionValue = ImprovementSolutionValue;				//updates it
			
			   if (BecceneriSolutionValue == largestPatternSize)					//if lowerbound is reached, ends the heuristic
		     	  goto END;

	           ImprovementSolutionValue = backwardImprovementWRTOpenStacks(BecceneriSolutionValue);	//applies scheduling rule 2 to the current solution

	           if(ImprovementSolutionValue < BecceneriSolutionValue)				//if the solution was improved
	              BecceneriSolutionValue = ImprovementSolutionValue;				//updates it

			   if (BecceneriSolutionValue == largestPatternSize)					//if lowerbound is reached, ends the heuristic
		     	  goto END;
	     
	           lookahead++;															//increase the number of pieces to lookahead
	        }  
    	}

    	alternativeSolution = solution;
	}


   	//looking ahead: applying scheduling rules, first part
   	threshold = THRESHOLD_PERCENTAGE*max(nPatterns, 100);
	threshold = min(nPatterns, threshold);

	if(LOOKAHEAD_BECCENERI)
	{
	   	patternSequencingBecceneri();												//sequences the patterns, Becceneri's method	
	   	
		//reInsertBottlenecks();
		//reInsertBottleneckRelatedPatterns();

	   	BecceneriSolutionValue = evaluation(solution, BOTTLENECK_CHECK);			//evaluates the solution

	   	if(REDUCE_BOTTLENECK)
   			reduceBottleNeckConsecutive1s(BecceneriSolutionValue);
   		if(REDUCE_BOTTLENECK_BEST_POSITION)
   			reduceBottleNeckConsecutive1sBestOppositePosition(BecceneriSolutionValue);

	   	if(REDUCE_BOTTLENECK_LOOP)   	
		   	for(i=0; i<REDUCE_BOTTLENECK_LOOP_SIZE; i++)
		    {
				BecceneriSolutionValue = evaluation(solution, true);
		    	
		    	if(REDUCE_BOTTLENECK_BEST_POSITION)
		    		reduceBottleNeckConsecutive1sBestOppositePosition(BecceneriSolutionValue);
		    	if(REDUCE_BOTTLENECK)
		    		reduceBottleNeckConsecutive1s(BecceneriSolutionValue);
		    	
		    	bottleneckPatterns.clear();
		    	bottleneckRelatedPatterns.clear();
		    }
	   	
	   	if (BecceneriSolutionValue == largestPatternSize)							//if lowerbound is reached, ends the heuristic
	   		goto END;

	   	lookahead = 1;	
	   																				//determines how many pieces to look ahead on pattern reallocation
		//---

		for(i=0; i<nPieces; i++)	
				pieces.push_back(i);

		if(RANDOM_PIECES)
		{
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(pieces.begin(), pieces.end(), std::default_random_engine(seed));
		}

	    for(i=0; i<threshold; i++)
	    {
	        lookahead = 1;															//starts by looking 1 piece ahead
	       
	        for(j=0; j<threshold; j++)
	        {
	           //reInsertBottlenecks();
	           //reInsertBottleneckRelatedPatterns();
	           ImprovementSolutionValue = forwardImprovementWRTOpenStacks(BecceneriSolutionValue);	//applies scheduling rule 1 to the current solution

	           if(ImprovementSolutionValue < BecceneriSolutionValue)				//if the solution was improved
	              BecceneriSolutionValue = ImprovementSolutionValue;				//updates it
			
			   if (BecceneriSolutionValue == largestPatternSize)					//if lowerbound is reached, ends the heuristic
		     	  goto END;

	           ImprovementSolutionValue = backwardImprovementWRTOpenStacks(BecceneriSolutionValue);	//applies scheduling rule 2 to the current solution

	           if(ImprovementSolutionValue < BecceneriSolutionValue)				//if the solution was improved
	              BecceneriSolutionValue = ImprovementSolutionValue;				//updates it

			   if (BecceneriSolutionValue == largestPatternSize)					//if lowerbound is reached, ends the heuristic
		     	  goto END;
	     
	           lookahead++;															//increase the number of pieces to lookahead
	        }
	    }
	   
	   	alternativeSolution = solution;
	}
   	//end of first part

    //looking ahead: applying scheduling rules, second part
   	if(LOOKAHEAD_YANASSE)
	{		
	    patternSequencingYanasse();													//sequences the patterns, Yanasse's method
	    
	    FinalSolutionValue = evaluation(solution, BOTTLENECK_CHECK);				//evaluates the solution

	    if(REDUCE_BOTTLENECK)
    	 	reduceBottleNeckConsecutive1s(FinalSolutionValue);
    	if(REDUCE_BOTTLENECK_BEST_POSITION)
    		reduceBottleNeckConsecutive1sBestOppositePosition(FinalSolutionValue);

		if(REDUCE_BOTTLENECK_LOOP)
		    for(i=0; i<REDUCE_BOTTLENECK_LOOP_SIZE; i++)
		    {
				FinalSolutionValue = evaluation(solution, true);					//evaluates the solution
		    	
		    	if(REDUCE_BOTTLENECK_BEST_POSITION)
		    		reduceBottleNeckConsecutive1sBestOppositePosition(FinalSolutionValue);
    		    if(REDUCE_BOTTLENECK)
		    		reduceBottleNeckConsecutive1s(FinalSolutionValue);

		    	bottleneckPatterns.clear();
		    	bottleneckRelatedPatterns.clear();
		    }
	
	   	if (FinalSolutionValue == largestPatternSize)								//if lowerbound is reached, ends the heuristic
	   		goto END;

	    lookahead = 1;																//determines how many pieces to look ahead on pattern reallocation

	    for(i=0; i<threshold; i++)
	    {
	        lookahead = 1;															//starts by looking 1 piece ahead

	        for(j=0; j<threshold; j++)
	        {
	           ImprovementSolutionValue = forwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 1 to the current solution
	           
	           if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
	              FinalSolutionValue = ImprovementSolutionValue;					//updates it
	           
	           if (FinalSolutionValue == largestPatternSize)						//if lowerbound is reached, ends the heuristic
	   			  goto END;

	           ImprovementSolutionValue = backwardImprovementWRTOpenStacks(FinalSolutionValue);		//applies scheduling rule 2 to the current solution
	           
	           if(ImprovementSolutionValue < FinalSolutionValue)					//if the solution was improved
	              FinalSolutionValue = ImprovementSolutionValue;					//updates it

	           if (FinalSolutionValue == largestPatternSize)						//if lowerbound is reached, ends the heuristic
	   			  goto END;
	           
	           lookahead++;															//increase the number of pieces to lookahead
	        }
	    }
    }
    //end of second part


    if(FinalSolutionValue > BecceneriSolutionValue)								//determines which one of the solutions is the best
    {
        //FinalSolutionValue = BecceneriSolutionValue;        					//updates the solution value, if necessary
        solution = alternativeSolution;											//updates the solution, if necessary
    }

END:
    if(FinalSolutionValue > BecceneriSolutionValue)								//determines which one of the solutions is the best
        FinalSolutionValue = BecceneriSolutionValue;        					//updates the solution value, if necessary

	high_resolution_clock::time_point t2 = high_resolution_clock::now(); 		//time taking
  	duration<double> time_span = duration_cast<duration<double> >(t2 - t1);

	*LookaheadSolutionValue = FinalSolutionValue;                             	//stores the solution value
	*LookaheadRunningTime =  time_span.count();									//stores the execution time

	printSolution(inputFileName, *LookaheadSolutionValue, *LookaheadRunningTime);//prints the solution to the file

	termination();																//terminates all data structures
}
