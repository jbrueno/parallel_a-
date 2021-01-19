#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <limits.h> // used for INT_MAX in find_min_h()


const int ROWS = 4;       // rows in the 4x4 grid representing the puzzle
const int COLS = 4;       // columns in the 4x4 grid representing the puzzle

typedef struct Board      // struct to hold a board and its associated values
{
	int board[ROWS][COLS];// the 2d array representing the n-puzzle 
	int h_score;          // the heuristic value given to the board
	struct Board* parent; // pointer to parent board if one exists (used to track sequence of moves taken)
} Board;

typedef struct Action     // struct to define a possible action that can be performed on a Board
{
	char* direction;      // the direction in which a board 'tile' can be shifted, possible
						  // values can be "up", "down", "left", or "right"
	int row;              // target row to move the tile
	int col;              // target column to move the tile
	int value;            // value of the tile to be moved
} Action;                 // an Action is when you swap a numbered tile with the 0 tile

typedef struct Node       // Nodes that will be in the linkedlist
{
	Board* board;         // Board struct
	struct Node* next;    // next node in the list
} Node;

typedef struct LinkedList // LinkedList to hold a list of Boards
{
	int size;             // size of the list
	Node* head;           // pointer to the first element in the list
	Node* tail;           // pointer to the last element in the list
} LinkedList;


/* Allocate memory for a Board struct.  Set the values all to the default which is
 * 0 for the heuristic value, NULL for the parent, and 0's for all values in the 
 * 2d array.  Return a pointer to the resulting Board. */
Board* make_empty_board(){
	Board* brd = (Board*)malloc(sizeof(Board));
	assert(brd);
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			brd->board[i][j] = 0;
		}
	}
	brd->h_score = 0;
	brd->parent = NULL;
	return brd;
}

/* free the allocated Board struct */
void destroy_board(Board* brd){ 
	free(brd);
}

/* prints all elements of a Board struct */
void print_board(Board* brd){    
	printf("H = %d\n", brd->h_score);
	printf("Board:\n");
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			printf("%-4d", brd->board[i][j]); // prints values left-justified
		}
		printf("\n");
	}
}

/* Returns 0 if the boards are equal and -1 otherwise */
int compare_boards(int x[ROWS][COLS], int y[ROWS][COLS]){
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			if(x[i][j] != y[i][j]){
				return -1;
			}
		}
	}
	return 0;
}

/* Copies the values from board A to board B */
void copy_board(Board* a, Board* b){
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			b->board[i][j] = a->board[i][j];
		}
	}
	b->h_score = a->h_score;
	b->parent = a->parent;
}

/* Creates and allocates space for a single node with given board */
Node* make_node(Board* brd){
	Node* result = malloc(sizeof(Node));
	assert(result);
	result->board = brd;
	result->next = NULL;
	return result;
}

/* Free memory allocated for the Node */
void destroy_node(Node* node){
	destroy_board(node->board);
	free(node);
}

/* Allocates space for a linkedlist.  Sets the head and tail 
 * pointers to NULL and initial size to 0. */
LinkedList* make_list(){
	LinkedList* list = malloc(sizeof(LinkedList));
	assert(list);
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

/* Free memory allocated for the linkedlist */
void free_list(LinkedList* list){
	Node* tmp =  list->head;
	while(tmp != list->tail){
		Node* delete = tmp;
		tmp = tmp->next;
		destroy_node(delete);
	}
	free(list->tail);
	free(list);
}

/* Prints the given list in an easily readable manner */
void print_list(LinkedList* list){
	Node* tmp = list->head;
	while(tmp != list->tail){
		Node* next = tmp;
		print_board(tmp->board);
		tmp = tmp->next;
	}
	print_board(list->tail->board);
}

/* Prints a node and all of its parents.  Used to print and visualize
 * the sequence taken to reach a solution in a_star_search */
 void print_parents(Node* node){
 	int steps = 0; // keep track of how many moves it took to solve
 	printf("\n----- SOLUTION SEQUENCE -----\n");
 	while(node->board->parent != NULL){
 		printf("\n");
 		print_board(node->board);
 		node->board = node->board->parent;
 		steps++;
 	}
 	printf("\n----- SOLUTION SEQUENCE -----\n");
 	printf("\n----------------------------------\n");
 	printf("\nNumber of moves to solution: %d\n", steps);
 	printf("\n----------------------------------\n\n");
 }

/* Add the given node to the end of the given linkedlist*/
void push(LinkedList* ll, Node* node){
	if(ll->size == 0){
		ll->head = node;
		ll->tail = node;
		ll->size++;
	}else if(ll->size == 1){
		ll->head->next = node;
		ll->tail = node;
		ll->size++;
	}else{
		ll->tail->next = node;
		ll->tail = node;
		ll->size++;
	}
}

/* Remove the first node in the given linkedlist */
void pop(LinkedList* list){
	if(list->size == 1){
		destroy_node(list->head);
		list->head = NULL;
		list->tail = NULL;
		list->size--;
	}else if(list->size == 2){
		destroy_node(list->head);
		list->head = list->tail;
		list->size--;
	}else{
		Node* tmp = list->head->next;
		destroy_node(list->head);
		list->head = tmp;
		list->size--;
	}
}

/* Delete the given node from the LinkedList */
void remove_node(LinkedList* list, Node* node){
	if(node != NULL){
	if(list->size == 1){
		list->head = NULL;
		list->tail = NULL;
		list->size--;
	} else if(list->size == 2){
		if(list->head == node){
			list->head = list->tail;
			list->size--;
		} else {
			list->tail = list->head;
			list->size--;
		}
	} else {
		if(list->head == node){
			Node* tmp = list->head;
			list->head = list->head->next;
			list->size--;
		} else if (list->tail == node){
			Node* prev = list->head;
			while(prev->next->next != NULL){ // find tails previous node 
				prev = prev->next;
			}
			list->tail = prev;
			list->size--;
		} else {
			Node* prev = list->head;
			while(prev->next != node && prev->next != NULL){ // find previous node of node to be deleted
				prev = prev->next;
			}
			prev->next = prev->next->next;
			list->size--;
		}
	}
	}
}

/* Returns the node in the list with the minimum h_score value */
Node* find_min_h(LinkedList* list){
	int min = INT_MAX;
	Node* tmp = list->head;
	Node* result;
	while(tmp != list->tail){
		if(min > tmp->board->h_score){  // if h is smaller than current min, set new min
			min = tmp->board->h_score;
			result = tmp;
		}
		tmp = tmp->next;
	}
	if(min > list->tail->board->h_score){
		min = list->tail->board->h_score;
		result = list->tail;
	}
	return result;
}

/* Returns 0 if the given node is in the list and -1 otherwise */
int in_list(LinkedList* list, Node* node){
	if(list->head == NULL) return -1;

	Node* tmp = list->head;
	while(tmp != list->tail){
		if(compare_boards(tmp->board->board, node->board->board) == 0){ // if boards are equal, return node
			return 0;
		}
		tmp = tmp->next;
	}
	if(compare_boards(list->tail->board->board, node->board->board) == 0){
		return 0;
	}
	return -1; 
}


/* Create and return an Action struct with the given values */
Action make_action(char* direction, int row, int col, int value){
	Action a;
	a.direction = direction;
	a.row = row;
	a.col = col;
	a.value = value;
	return a;
}

/* Print all values of an Action in a readable manner */
void print_action(Action act){
	printf("Move %d in Row: %d  Column: %d %s\n", act.value, act.row, act.col, act.direction);
}

/* Calculates the number of misplaced tiles on the board and
 * returns the number.  Can be added to h_score with manhattan
 * distance for a faster but less optimal solution. */
int misplaced(Board *brd){
	int expected = 1; // holds index of current expected value
	int sum = 0;      // holds the solution
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			if((i == 3 && j == 3)) break;
			if(brd->board[i][j] != expected){
				sum++;
				expected++;
			}
		}
	}
	return sum;
}

/* Calculates the Manhattan Distance for the given board and returns
 * it in the form of an int.  The Manhattan Distance is the sum of
 * the distances that all the tiles are misplaced on the board.
 * (the 0 or null tile is not counted)  */
int manhattan_distance(Board* brd){
	int sum = 0;
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			int value = brd->board[i][j];
			if(value != 0){
				int goal_x = value / ROWS;
				int goal_y = value % COLS;
				int dx = i - goal_x;
				int dy = j - goal_y;
				sum += (abs(dx) + abs(dy));
			}
		}
	}
	return sum;
}

/* Takes in a pointer to a board and returns an array of all possible
 * actions that can be applied to the board. Allocates an array of length
 * 4 that contains actions */
Action* possible_actions(Board* brd){
	int max_actions = 4; // there will be at most 4 possible moves
	Action* poss_actions = malloc(max_actions * sizeof(Action));
	assert(poss_actions);
	int index = 0; // keeps track of number of assigned elements to array

	for(int i = 0; i < max_actions; i++){ // initialize the array with NULL actions
		poss_actions[i] = make_action(NULL, -1, -1, -1);
	}
	
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			// check if you can move right
			if(j != 3){
				if(brd->board[i][j] != 0 && brd->board[i][j+1] == 0){
					Action act = make_action("RIGHT", i, j, brd->board[i][j]);
					poss_actions[index] = act;
					index++;
				}
			}
			// check if you can move left
			if(j != 0){
				if(brd->board[i][j] != 0 && brd->board[i][j-1] == 0){
					Action act = make_action("LEFT", i, j, brd->board[i][j]);
					poss_actions[index] = act;
					index++;
				}
			}
			// check if you can move up
			if(i != 0){
				if(brd->board[i][j] != 0 && brd->board[i-1][j] == 0){
					Action act = make_action("UP", i, j, brd->board[i][j]);
					poss_actions[index] = act;
					index++;
				}
			}
			// check if you can move down
			if(i != 3){
				if(brd->board[i][j] != 0 && brd->board[i+1][j] == 0){
					Action act = make_action("DOWN", i, j, brd->board[i][j]);
					poss_actions[index] = act;
					index++;
				}
			}
		}
	}
	return poss_actions;
}

/* Takes in an Action and a Board.  Returns a copy of
 * the given board after the given Action is applied to it. 
 * The Action results in the given tile being swapped with the
 * tile in the given direction. Allocates space for a single Board */
Board* result(Action act, Board* brd) {
	int move;                           // represents the tile to be moved  
	Board* result = make_empty_board(); // allocate space for a new Board
	for(int i = 0; i < ROWS; i++){      // copy the 2d array into the new board
		for(int j = 0; j < COLS; j++){
			result->board[i][j] = brd->board[i][j];
		}
	}
	// set the parent board to the Board before action is applied
	result->parent = brd;

	if(strcmp(act.direction, "RIGHT") == 0){
		move = result->board[act.row][act.col];
		result->board[act.row][act.col] = result->board[act.row][act.col + 1];
		result->board[act.row][act.col + 1] = move;
	} else if(strcmp(act.direction, "LEFT") == 0){
		move = result->board[act.row][act.col];
		result->board[act.row][act.col] = result->board[act.row][act.col - 1];
		result->board[act.row][act.col - 1] = move;
	} else if(strcmp(act.direction, "UP") == 0){
		move = result->board[act.row][act.col];
		result->board[act.row][act.col] = result->board[act.row - 1][act.col];
		result->board[act.row - 1][act.col] = move;
	} else { // "DOWN"
		move = result->board[act.row][act.col];
		result->board[act.row][act.col] = result->board[act.row + 1][act.col];
		result->board[act.row + 1][act.col] = move;
	}
	// set the h_score to be the manhattan distance of the board
	result->h_score = manhattan_distance(result); //

	return result;
}

/* Takes in a board and returns an array of boards that are the
 * result of applying all possible actions to the given board. 
 * Allocates space for an array of size 4 containing Board structs */
LinkedList* expand(Board* brd){
	int index = 0;                               // current index of acts array
	Action* acts = possible_actions(brd);        // list of possible actions to perform on brd
	LinkedList* list = make_list();              // linkedlist to hold the boards after actions are performed

	while(acts[index].direction != NULL){        // for each action, add resulting board to the end of list
		Board* tmp = result(acts[index], brd);
		Node* node = make_node(tmp);
		push(list, node);
		index++;
		if(index == 4) break;
	}
	free(acts);
	return list;
}

/* Informed search algorithm that finds a path or solution
 * from the start board to the goal board. The heuristic function 
 * used is the Manhattan Distance which is calculated and assigned 
 * in the result() function.
 *
 */
int a_star_search(Board* start, Board* goal){
	int iters = 0; // counts # of iterations
	LinkedList* closed_list = make_list(); // list of nodes already
	LinkedList* open_list = make_list();   // list of nodes to explore
	Node* first = make_node(start);
	first->board->h_score = manhattan_distance(first->board);
	push(open_list, first);

	if(compare_boards(start->board, goal->board) == 0){
		printf("---SOLUTION FOUND---\n");
		printf("initial board was equal to the goal board\n");
		return 0;
	}
    
	while(open_list->size != 0){               // while the open_list is not empty
		Node* remove = find_min_h(open_list);  // explore board on list with the lowest h_score
		Board* b = make_empty_board();
		copy_board(remove->board, b);
		Node* current = make_node(b);          // current is a copy of the removed board
		push(closed_list, remove);             // add node to closed list
		remove_node(open_list, remove);        // remove node from the list

		LinkedList* check_list = expand(current->board);
		Node* check = check_list->head;

		if(compare_boards(current->board->board, goal->board) == 0){ // if solution was found print solution
			printf("\n----------SOLUTION FOUND----------\n");
			printf("\nITERATIONS: %d\n", iters);
			print_parents(current);
			printf("INITIAL BOARD:\n");
			print_board(start);
			printf("\nFINAL BOARD:\n");
			print_board(goal);
			printf("\n----------SOLUTION FOUND-----------\n");

			free_list(check_list);
			free_list(closed_list);
			free_list(open_list);
			return 0;
		}

		while(check != NULL){
			Node *copy;
			Board* x = make_empty_board();
			copy_board(check->board, x);
			copy = make_node(x);

			if(in_list(closed_list, copy) == 0){         // if the node is in the closed list, go to next iteration
				check = check->next;
				continue;
			} else if (in_list(open_list, copy) == 0){   // if the node is in the open list, check is better h exists
				Node* tmp = open_list->head;
				while(tmp != NULL){  // find the node with matching board in the open list
					if(compare_boards(tmp->board->board, copy->board->board) == 0){
						break;
					}
					tmp = tmp->next;
				}
				if(tmp->board->h_score > copy->board->h_score){ // if existing board has a worse h value, update it
					tmp->board->parent = copy->board->parent;
					tmp->board->h_score = copy->board->h_score;
					check = check->next;
					continue;
				} else {
					check = check->next;
					continue;
				}
			}    
            push(open_list, copy); // node is not on open or closed list, so add it to the open list to be explored
			check = check->next;
		} // while
		iters++;
	} // while
	printf("\n-------SOLUTION NOT FOUND-------\n");
	printf("\n\n\n\nITERATIONS:  %d\n\n\n\n\n", iters);

	free_list(closed_list);
	free_list(open_list);
	return -1;
}







int main(){

	/* GOAL is the goal board for all of the given test board,
	 * the numbers go in order from top left to bottom right with 
	 * the first space(index[0][0]) being empty(represented by a 0) */
	Board* GOAL = make_empty_board();
	GOAL->board[0][0] = 0;
	GOAL->board[0][1] = 1;
	GOAL->board[0][2] = 2;
	GOAL->board[0][3] = 3;
	GOAL->board[1][0] = 4;
	GOAL->board[1][1] = 5;
	GOAL->board[1][2] = 6;
	GOAL->board[1][3] = 7; 
	GOAL->board[2][0] = 8;
	GOAL->board[2][1] = 9;
	GOAL->board[2][2] = 10;
	GOAL->board[2][3] = 11;
	GOAL->board[3][0] = 12;
	GOAL->board[3][1] = 13;
	GOAL->board[3][2] = 14;
	GOAL->board[3][3] = 15;


	/* This board is the initial board given to the A* search.  In order
	 * to reach the GOAL state there is a minimun of 80 moves required.
	 * This is the highest number of moves required to solve any 15-puzzle */
	Board *TEST = make_empty_board();
	TEST->board[0][0] = 15;
	TEST->board[0][1] = 11;
	TEST->board[0][2] = 13;
	TEST->board[0][3] = 12;
	TEST->board[1][0] = 14;
	TEST->board[1][1] = 10;
	TEST->board[1][2] = 8;
	TEST->board[1][3] = 9; 
	TEST->board[2][0] = 7;
	TEST->board[2][1] = 2;
	TEST->board[2][2] = 5;
	TEST->board[2][3] = 1;
	TEST->board[3][0] = 3;
	TEST->board[3][1] = 6;
	TEST->board[3][2] = 4;
	TEST->board[3][3] = 0;


	clock_t start = clock();

	a_star_search(TEST, GOAL); // run sequential search, find path from TEST to GOAL

	clock_t end = clock();
	double total_time = (double)(end - start) / CLOCKS_PER_SEC;
	printf("\nTotal Time: %f seconds\n\n", total_time);

	destroy_board(GOAL);
}




