#include <iostream>
#include "pthread.h"

// Defining Structures
typedef struct node {
	int value;
	node* left;
	node* right;
} node;

class bst {
private:
	node* head;

public:
	bst();
	~bst();
	void insert(int val, node* head);
	void remove(int val, node* head);
	bool lookup(int val, node* head);
};

// BST Methods
bst::bst() : head(nullptr) {}

bst::~bst() {}

void bst::insert(int val, node* head) {
	if(head == nullptr) { head = new node { val, nullptr, nullptr }; }
	if(head -> value > val) { insert(val, head -> left); }
	else { insert(val, head -> right); }
	return;
}

int main(int argc, char* argv[]) {
	return 0;
}
