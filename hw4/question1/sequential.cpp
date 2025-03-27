#include <iostream>
#include "pthread.h"
#include "sys/time.h"

#define MAX_THREAD_COUNT 2
#define MAX_MUTEX_COUNT 2

// Defining Structures & BST Functions
typedef struct node {
	int val;
	node* left;
	node* right;
} node;

node* insert(node* head, int val) {
	if(head == nullptr) { return new node { val, nullptr, nullptr }; }
	if(head -> val == val) { return head; }
	if(head -> val > val) { head -> left = insert(head -> left, val); }
	if(head -> val < val) { head -> right = insert(head -> right, val); }
	return head;
}

bool lookup(node* head, int val) {
	if(head == nullptr) { return false; }
	if(head -> val == val) { return true; }
	if(head -> val > val) { return lookup(head -> left, val); }
	else { return lookup(head -> right, val); }
}

node* remove(node* head, int val) {
	if(head == nullptr) { return head; }
	if(head -> val == val) { delete head; return nullptr; }
	if(head -> val > val) { head -> left = remove(head -> left, val); }
	else { head -> right = remove(head -> right, val); }
	return head;
}

// Helper Functions
void printTree(node* head) {
	if(head == nullptr) { return; }
	printTree(head -> left);
	std::cout << head -> val << " ";
	printTree(head -> right);
}

void deleteTree(node* head) {
	if(head == nullptr) { return; }
	deleteTree(head -> left);
	deleteTree(head -> right);
	delete head;
}

// Main Functions
int main(int argc, char* argv[]) {
	struct timeval start, end, diff;
	gettimeofday(&end, nullptr);	

	node* head = nullptr;
	head = insert(head, 5);
	head = insert(head, 2);
	head = insert(head, 8);
	head = insert(head, 1);
	head = insert(head, 9);
	head = insert(head, 3);
	head = insert(head, 6);
	head = insert(head, 7);
	head = insert(head, 4);
	head = insert(head, 10);

	gettimeofday(&end, nullptr);	
	timersub(&end, &start, &diff);

	std::cout << "Total uS Sequentially: " << diff.tv_usec << std::endl;
	deleteTree(head);
	return 0;
}
