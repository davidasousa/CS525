#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include "pthread.h"
#include "sys/time.h"

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
	struct timeval start, end, diff;
	// Task Count For Iteration
	int task_count = 0;
	// Function Pointers For Calling Threads
	void* (*ins)(void*) = thread_insert;
	void* (*del)(void*) = thread_delete;
	void* (*lkup)(void*) = thread_lookup;
	// Random Generator
	int seed = 0;
	std::mt19937 gen(seed);
	std::uniform_real_distribution<> dist(0.0, 1.0);
	
	for(int thread_count = 1; thread_count <= 2; thread_count++) {
		gettimeofday(&start, nullptr);	
		// Threads
		pthread_t* threads = new pthread_t[thread_count];

		// Number Array
		int total_numbers = 3 * pow(10, 6);
		std::vector<int> nums(total_numbers);
		std::iota(nums.begin(), nums.end(), 0);

		// Shuffling The Array
		std::vector<int> v1, v2, v3;
		v1 = randomize_vec(nums, 3, gen); // Third Of Shuffled Nums
		v2 = randomize_vec(v1, 2, gen); // Half Of Shuffed V1
		v3 = randomize_vec(nums, 2, gen);	// Half Of Shuffled Nums

		// Creating The Three Heads
		node* d1 = create_node(0, nullptr);
		node* d2 = create_node(0, nullptr);
		node* d3 = create_node(0, nullptr);

		// Inserting All V1 Values To D1
		std::vector<node_val> d1_ins_args;
		for(int z : v1) { d1_ins_args.push_back(node_val {d1, z}); }
		while(task_count < v1.size()) {
			for(int tidx = 0; tidx < thread_count; tidx++) {
				pthread_create(&threads[tidx], nullptr, ins, (void*) &d1_ins_args[task_count++]);
				if(task_count == v1.size()) { break; }
			}	
			for(int tidx = 0; tidx < thread_count; tidx++) {
				pthread_join(threads[tidx], nullptr);		
			}
		}

		// Deleting All V2 Nodes From D2
		std::vector<node_val> d2_del_args;
		for(int z : v2) { d2_del_args.push_back(node_val {d2, z}); }
		while(task_count < v2.size()) {
			for(int tidx = 0; tidx < thread_count; tidx++) {
				pthread_create(&threads[tidx], nullptr, del, (void*) &d2_del_args[task_count++]);
				if(task_count == v1.size()) { break; }
			}	
			for(int tidx = 0; tidx < thread_count; tidx++) {
				pthread_join(threads[tidx], nullptr);		
			}
		}

		// Performing The Final Complex Operation From D3
		std::vector<node_val> d3_args;
		std::vector<bool> is_lookup;
		for(int z : v3) {
			if(dist(gen) >= 0.5) { 
				d3_args.append(node_val{d3, z}); 
				is_lookup.push_back(true); // Lookup
			} 
			else { 
				d3_args.append(node_val{d3, z}); 
				is_lookup.push_back(false); // Delete
			}
		}
		
		// Printing Output
		gettimeofday(&end, nullptr);	
		timersub(&end, &start, &diff);
		double total_time = diff.tv_sec + ((float) diff.tv_usec) / 1000000;
		std::cout << "\nTotal Sec With " << thread_count;
		std::cout << " Threads: " << total_time << std::endl;

		// Freeing Memory
		delete[] threads;
		delete_tree(d1);
		delete_tree(d2);
		delete_tree(d3);
	}

	return 0;
}
