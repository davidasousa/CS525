#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include "pthread.h"
#include "sys/time.h"

#define THREAD_COUNT 4

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
	if(head -> parent_mutex != nullptr) { 
		pthread_mutex_unlock(head -> parent_mutex);
	}

	if(head -> val == val) { 
		pthread_mutex_unlock(head -> node_mutex);
		return true; 
	} else if(head -> val > val) {
		if(head -> left == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return false;	
		} 
		else { return lookup(head -> left, val); }
	} else if(head -> val < val) {
		if(head -> right == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return false; 
		} 
		else { return lookup(head -> right, val); } 
	}
	return false;
}

void*
thread_lookup(void* arg) {
	node_val* args = (node_val*) arg;
	return (void*) lookup(args -> head, args -> val);
}

// Recursive Insertion For Deletion To Insert No Nodes Are Lost
void recursive_insert(node* head, node* ins) {
	if(head == nullptr) { return; }
	recursive_insert(head -> left, ins);
	recursive_insert(head -> right, ins);
	insert(ins, head -> val);
	delete head -> node_mutex;
	delete head;	
}

void
remove(node* head, int val) {
	pthread_mutex_lock(head -> node_mutex);
	if(head -> parent_mutex != nullptr) { pthread_mutex_unlock(head -> parent_mutex); }
	
	node* del = nullptr; // Node Being Deleted
	node* swap = nullptr; // Node Taking The Place Of Del
	if(head -> val > val) { 
		if(head -> left == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return; 
		} else if(head -> left -> val == val) { // Deleting Head -> Left
			del = head -> left;

			if(del -> left != nullptr) {
				swap = del -> left; 
				//swap -> right = del -> right; // Take The Right Of Del & Insert To Swap
				recursive_insert(del -> right, swap);
			} else if(del -> right != nullptr) {
				// Del -> Left Is Nullptr
				swap = del -> right; 
				swap -> left = nullptr;
			}
			head -> left = swap; // Head Left Is Now Swap
			if(swap != nullptr) { swap -> parent_mutex = head -> node_mutex; }

			pthread_mutex_unlock(head -> node_mutex);
			delete del -> node_mutex;
			delete del;
		} else { remove(head -> left, val); }
	} else if(head -> val < val) { 
		if(head -> right == nullptr) { 
			pthread_mutex_unlock(head -> node_mutex);
			return; 
		} else if(head -> right -> val == val) { 
			del = head -> right;

			if(del -> left != nullptr) {
				swap = del -> left; 
				swap -> right = del -> right; // Take The Right Of Del & Insert To Swap
				recursive_insert(del -> right, swap);
			} else if(del -> right != nullptr) {
				// Del -> Left Is Nullptr
				swap = del -> right; 
				swap -> left = nullptr;
			}
			head -> right = swap; 
			if(swap != nullptr) { swap -> parent_mutex = head -> node_mutex; }

			pthread_mutex_unlock(head -> node_mutex);
			delete del -> node_mutex;
			delete del;
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

// Frac -> Fraction Of The Original Array To Be Chosen
std::vector<int> 
randomize_vec(std::vector<int>& vec, int frac, std::mt19937 gen) {
	std::shuffle(vec.begin(), vec.end(), gen);

	int new_size = vec.size() / frac;
	std::vector<int> new_vec;
	new_vec.assign(vec.begin(), vec.begin() + new_size);
	return new_vec;
}

// Main Functions
int 
main(int argc, char* argv[]) {
	int task_count = 0;
	int threads_used = 0;

	struct timeval start, end, diff;
	gettimeofday(&start, nullptr);	

	// 4 Thread Implementation

	// Thread Information
	pthread_t* threads = new pthread_t[THREAD_COUNT];
	void* (*ins)(void*) = thread_insert;
	void* (*del)(void*) = thread_delete;
	void* (*lkup)(void*) = thread_lookup;

	int iter = 0;

	// Part 1
	node* head = create_node(9, nullptr); // Head Created Sequentially
	std::vector<node_val> p1_args = {
		node_val { head, 9 },
		node_val { head, 7 },
		node_val { head, 8 },
		node_val { head, 1 },
		node_val { head, 5 },
		node_val { head, 3 },
		node_val { head, 6 },
		node_val { head, 2 },
		node_val { head, 4 },
		node_val { head, 10 },
		node_val { head, -9 },
		node_val { head, -7 },
		node_val { head, -8 },
		node_val { head, -1 },
		node_val { head, -5 },
		node_val { head, -3 },
		node_val { head, -6 },
		node_val { head, -2 },
		node_val { head, -4 },
		node_val { head, -10 }
	};

	while(iter * 5 < p1_args.size()) {
		for(int tidx = 0; tidx < THREAD_COUNT; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_create(&threads[tidx], nullptr, ins, (void*) &p1_args[tidx * 5 + iter]);
			threads_used++;
		}
			
		for(int tidx = 0; tidx < threads_used; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_join(threads[tidx], nullptr);		
		}
		threads_used = 0;
		iter++;
	}

	// Part 2
	std::vector<node_val> p2_args = {
		node_val { head, 9 },
		node_val { head, 100 },
		node_val { head, 8321 }, // Values That Dont Exist In The Tree
		node_val { head, 1 },
		node_val { head, 5 },
		node_val { head, 33121 },
		node_val { head, 6 },
		node_val { head, 232 },
		node_val { head, 4 },
		node_val { head, 10 },
		node_val { head, -91231 },
		node_val { head, -7 },
		node_val { head, -83213 },
		node_val { head, -1 },
		node_val { head, -5123 },
		node_val { head, -3321 },
		node_val { head, -6 },
		node_val { head, -2 },
		node_val { head, -41231 },
		node_val { head, -10 }
	};

	while(iter * 5 < p2_args.size()) {
		for(int tidx = 0; tidx < THREAD_COUNT; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_create(&threads[tidx], nullptr, del, (void*) &p2_args[tidx * 5 + iter]);
			threads_used++;
		}
			
		for(int tidx = 0; tidx < threads_used; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_join(threads[tidx], nullptr);		
		}
		threads_used = 0;
		iter++;
	}
	

	// Part 3
	iter = 0;
	std::vector<node_val> p3_args = {
		node_val { head, 9 },
		node_val { head, 7 },
		node_val { head, 8 },
		node_val { head, 1 },
		node_val { head, 5 },
		node_val { head, 3 },
		node_val { head, 6 },
		node_val { head, 2 },
		node_val { head, 4 },
		node_val { head, 10 },
		node_val { head, -9 },
		node_val { head, -7 },
		node_val { head, -8 },
		node_val { head, -1 },
		node_val { head, -5 },
		node_val { head, -3 },
		node_val { head, -6 },
		node_val { head, -2 },
		node_val { head, -4 },
		node_val { head, -10 }
	};

	while(iter * 5 < p3_args.size()) {
		for(int tidx = 0; tidx < THREAD_COUNT; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_create(&threads[tidx], nullptr, lkup, (void*) &p3_args[tidx * 5 + iter]);
			threads_used++;
		}
			
		for(int tidx = 0; tidx < threads_used; tidx++) {
			if(tidx == 0 && iter == 0) { continue; }
			pthread_join(threads[tidx], nullptr);		
		}
		threads_used = 0;
		iter++;
	}

	gettimeofday(&end, nullptr);	
	timersub(&end, &start, &diff);
	double total_time = diff.tv_sec + ((float) diff.tv_usec) / 1000000;

	std::cout << "Total Batched 4 Threads: " << total_time << " Sec" << std::endl;

	// Freeing Memory
	delete[] threads;
	delete_tree(head);

	return 0;
}
