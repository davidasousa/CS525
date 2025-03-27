#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include "pthread.h"
#include "sys/time.h"

// Constant Values
#define MAX_THREAD_COUNT 2
#define PART_1_LENGTH 10
#define PART_2_LENGTH 5
#define PART_3_LENGTH 6

// Defining Structures & BST Functions
typedef struct node {
	int val;
	node* left;
	node* right;
	pthread_mutex_t* node_mutex;
	pthread_mutex_t* parent_mutex;
} node;

typedef struct node_val { 
	node* head;
	int val;
} node_val;

node* 
create_node(int val, pthread_mutex_t* pmutex) {
	pthread_mutex_t* node_mutex = new pthread_mutex_t;
	pthread_mutex_init(node_mutex, nullptr);
	if(pmutex != nullptr) { pthread_mutex_unlock(pmutex); }
	return new node { val, nullptr, nullptr, node_mutex, pmutex }; 
}

// Helper Functions
// Print Tree With Formatting

void 
print_tree(node* head, int level) {
	if(head == nullptr) { return; }
	std::cout << "Level: " << level << " Value: " << head -> val;
	std::cout << std::endl << std::flush;

	std::cout << "Left " << std::flush;
	print_tree(head -> left, level + 1);
	std::cout << "Right " << std::flush;
	print_tree(head -> right, level + 1);
	std::cout << "Back " << std::flush;
}

void 
delete_tree(node* head) {
	if(head == nullptr) { return; }
	delete_tree(head -> left);
	delete_tree(head -> right);
	delete head -> node_mutex;
	delete head;
}

// End Helper Functions

// Insert In The Function -> Reduced Bottlenecks - Parallel Insert
void
insert(node* head, int val) {
	pthread_mutex_lock(head -> node_mutex);
	if(head -> parent_mutex != nullptr) { pthread_mutex_unlock(head -> parent_mutex); }

	if(head -> val > val) {
		if(head -> left == nullptr) { head -> left = create_node(val, head -> node_mutex); }
		else { insert(head -> left, val); }
	} else if(head -> val < val) {
		if(head -> right == nullptr) { head -> right = create_node(val, head -> node_mutex); }
		else { insert(head -> right, val); } 
	}
}

void*
thread_insert(void* arg) {
	node_val* args = (node_val*) arg;
	insert(args -> head, args -> val);
	return nullptr;
}

bool
lookup(node* head, int val) {
	pthread_mutex_lock(head -> node_mutex);
	if(head -> parent_mutex != nullptr) { pthread_mutex_unlock(head -> parent_mutex); }

	if(head -> val == val) { return true; }

	if(head -> val > val) {
		if(head -> left == nullptr) { return false;	} 
		else { return lookup(head -> left, val); }
	} else if(head -> val < val) {
		if(head -> right == nullptr) { return false; } 
		else { return lookup(head -> right, val); } 
	}
	return false;
}

void*
thread_lookup(void* arg) {
	node_val* args = (node_val*) arg;
	return (void*) lookup(args -> head, args -> val);
}

void
remove(node* head, int val) {
	pthread_mutex_lock(head -> node_mutex);
	if(head -> parent_mutex != nullptr) { pthread_mutex_unlock(head -> parent_mutex); }

	node* temp = nullptr;
	if(head -> val > val) { 
		if(head -> left == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return; 
		} else if(head -> left -> val == val) { 
			// Deleting Node 
			temp = head -> left;

			// If Node Has Children -> Shift Nodes
			if(temp -> left != nullptr) {
				temp -> left -> right = temp -> right;
				head -> left = temp -> left;
			} else if(temp -> right != nullptr) {
				temp -> right -> left = temp -> left;
				head -> left = temp -> right;
			} else { head -> left = nullptr; }

			delete temp -> node_mutex;
			delete temp;
			// Unlocking Mutex
			pthread_mutex_unlock(head -> node_mutex);
		} else { remove(head -> left, val); }
	} else if(head -> val < val) { 
		if(head -> right == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return; 
		} else if(head -> right -> val == val) { 
			// Deleting Node
			temp = head -> right;

			// If Node Has Children -> Shift Nodes
			if(temp -> left != nullptr) {
				temp -> left -> right = temp -> right;
				head -> right = temp -> left;
			} else if(temp -> right != nullptr) {
				temp -> right -> left = temp -> left;
				head -> right = temp -> right;
			} else { head -> right = nullptr; }

			head -> right = nullptr;
			delete temp -> node_mutex;
			delete temp;
			// Unlocking Mutex
			pthread_mutex_unlock(head -> node_mutex);
		} else { remove(head -> right, val); }
	}
	return;
}

void*
thread_delete(void* arg) {
	node_val* args = (node_val*) arg;
	remove(args -> head, args -> val);
	return nullptr;
}

// Main Functions
int 
main(int argc, char* argv[]) {
	// Timing Variables
	struct timeval start, end, diff;
	// Threads
	pthread_t threads[MAX_THREAD_COUNT];

	gettimeofday(&start, nullptr);	

	// Question 1 Tasks - Array Of Insertion Calls
	// Main Thread Initializes Head
	node* head = create_node(5, nullptr);
	void* (*ins)(void*) = thread_insert;
	std::vector<node_val> q1_args = {
		node_val { head, 5 },
		node_val { head, 2 },
		node_val { head, 8 },
		node_val { head, 1 },
		node_val { head, 9 },
		node_val { head, 3 },
		node_val { head, 6 },
		node_val { head, 7 },
		node_val { head, 4 },
		node_val { head, 10 },	
	};

	int task_count = 1;
	while(task_count < PART_1_LENGTH) {
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_create(&threads[tidx], nullptr, ins, (void*) &q1_args[task_count++]);
			if(task_count == PART_1_LENGTH) { break; }
		}
			
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_join(threads[tidx], nullptr);		
		}
	}

	gettimeofday(&end, nullptr);	
	timersub(&end, &start, &diff);
	std::cout << "\nPart 1, Total uS Parallelized: " << diff.tv_usec << std::endl;
	
	std::cout << "\n" << std::flush;
	print_tree(head, 0);
	std::cout << "\n" << std::flush;

	// End Part 1

	// Start Part 2
	void* (*del)(void*) = thread_delete;
	std::vector<node_val> q2_args = {
		node_val { head, 7 },
		node_val { head, 0 },
		node_val { head, 6 },
		node_val { head, 2 },
		node_val { head, 10 },
	};

	task_count = 0;
	while(task_count < PART_2_LENGTH) {
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_create(&threads[tidx], nullptr, del, (void*) &q2_args[task_count++]);
			if(task_count == PART_2_LENGTH) { break; }
		}
			
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_join(threads[tidx], nullptr);		
		}
	}

	gettimeofday(&end, nullptr);	
	timersub(&end, &start, &diff);
	std::cout << "\nPart 2, Total uS Parallelized: " << diff.tv_usec << std::endl;

	std::cout << "\n" << std::flush;
	print_tree(head, 0);
	std::cout << "\n" << std::flush;

	// End Part 2

	// Start Part 3
	void* (*lkup)(void*) = thread_lookup;
	std::vector<node_val> q3_args = {
		node_val { head, 1 },
		node_val { head, 6 },
		node_val { head, 10 },
		node_val { head, 5 },
		node_val { head, 8 },
		node_val { head, 0 },
	};

	task_count = 0;
	while(task_count < 1) {
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_create(&threads[tidx], nullptr, lkup, (void*) &q3_args[task_count++]);
			if(task_count == 1) { break; }
		}
			
		for(int tidx = 0; tidx < MAX_THREAD_COUNT; tidx++) {
			pthread_join(threads[tidx], nullptr);		
		}
	}

	gettimeofday(&end, nullptr);	
	timersub(&end, &start, &diff);
	std::cout << "\nPart 3, Total uS Parallelized: " << diff.tv_usec << std::endl;

	std::cout << "\n" << std::flush;
	print_tree(head, 0);
	std::cout << "\n" << std::flush;

	delete_tree(head);
	return 0;
}
