#include "../include/redblacktree.h"
#include "../include/flags.h"

typedef enum color {RED, BLACK} Color;

typedef struct node *Node;

struct node
{
	void *data;                // Element stored in node
	Node left, right, parent;  // Pointers to children and parent
	Color color;               // Node's color (red or black)
};

struct rbtree
{
	Node root;                 // Root of the tree
	size_t size;               // Number of elements in tree
	cmpFunc cmp;
	destroyFunc destroy;
	int flag;
};


//////////////////////////////////// FUNCTIONS FOR NODES ////////////////////////////////////////

// Create and return a new node
static Node node_new(void *data, Node parent)
{
	Node node = malloc(sizeof(struct node));
	if (!node)
		return NULL;

	node->data = data;
	node->parent = parent;
	node->left = node->right = NULL;
	node->color = RED; // New node is colored red

	return node;
}

// Return node's color
static inline Color node_color(Node node)
{
	return node ? node->color : BLACK;
}

// Return true if node is the root
static inline bool node_is_root(Node node)
{
	return node->parent == NULL;
}

// Return true if node is a left child
static inline bool node_is_left(Node node)
{
	return node == node->parent->left;
}

// Return true if node is a right child
static inline bool node_is_right(Node node)
{
	return node == node->parent->right;
}

// Return node's sibling
static inline Node node_sibling(Node node)
{
	return node_is_left(node) ? node->parent->right : node->parent->left;
}

// Remove node and replace it with one of its children, return that child
static inline Node node_remove(Node node)
{
	// If node has a child that is not NULL, this will replace him
	Node child = !node->right ? node->left : node->right;

	if (node_is_left(node))
		node->parent->left = child;
	else
		node->parent->right = child;

	if (child)
		child->parent = node->parent;

	return child;
}

// Swap element in node with element in its predecessor and return predecessor
static Node node_swap_with_pred(Node node)
{
	// Predecessor is the rightmost node of left subtree
	Node pred = node->left;

	while (pred->right)
		pred = pred->right;

	// Swap elements
	void *temp = node->data;
	node->data = pred->data;
	pred->data = temp;

	return pred;
}

// Free memory allocated for node
static inline void node_destroy(Node node, destroyFunc destroy)
{
	if (destroy) destroy(node->data);
	free(node);
}



///////////////////////////////////// RECURSIVE FUNCTIONS /////////////////////////////////////////

// Return a deep copy of tree rooted at node
static Node node_copy_deep(Node node, copyFunc copy, destroyFunc destroy)
{
	// Create a copy of the old node
	Node node_copied = node_new(NULL, NULL);
	if (!node_copied) return NULL;

	node_copied->data = copy(node->data);
	node_copied->color = node->color;

	if (!node_copied->data)
	{
		free(node_copied);
		return NULL;
	}

	// If it has a left child, copy that subtree
	if (node->left)
	{
		node_copied->left = node_copy_deep(node->left, copy, destroy);

		// If NULL is returned, memory couldn't be allocated
		if (!node_copied->left)
		{
			node_destroy(node_copied, destroy);
			return NULL;
		}

		node_copied->left->parent = node_copied;
	}

	// Do the same for right child
	if (node->right)
	{
		node_copied->right = node_copy_deep(node->right, copy, destroy);

		if (!node->right)
		{
			node_destroy(node_copied, destroy);
			node_destroy(node_copied->left, destroy);
			return NULL;
		}

		node_copied->right->parent = node_copied;
	}

	return node_copied;
}

// Return a shallow copy of tree rooted at node
static Node node_copy_shallow(Node node)
{
	Node node_copied = node_new(node->data, NULL);
	if (!node_copied) return NULL;

	node_copied->color = node->color;

	if (node->left)
	{
		node_copied->left = node_copy_shallow(node->left);

		if (!node_copied->left)
		{
			free(node);
			return NULL;
		}

		node_copied->left->parent = node_copied;
	}

	if (node->right)
	{
		node_copied->right = node_copy_shallow(node->right);

		if (!node_copied->right)
		{
			free(node_copied);
			free(node_copied->left);
			return NULL;
		}

		node_copied->right->parent = node_copied;
	}

	return node_copied;
}

// Free memory allocated for tree rooted at node
static void node_destroy_all(Node node, destroyFunc destroy)
{
	if (!node)
		return;

	node_destroy_all(node->left, destroy);
	node_destroy_all(node->right, destroy);
	node_destroy(node, destroy);
}



//////////////////////////////////// BALANCING FUNCTIONS ////////////////////////////////////////

// Perform right rotation on grandparent and return node that replaces it in the tree
static Node right_rotation(Node grandparent)
{
	Node parent = grandparent->left;
	Node right_child = parent->right;

	// Parent takes grandparent's place
	if (!node_is_root(grandparent))
	{
		if (node_is_left(grandparent))
			grandparent->parent->left = parent;
		else
			grandparent->parent->right = parent;
	}

	parent->parent = grandparent->parent;

	// Grandparent becomes right child of parent
	parent->right = grandparent;
	grandparent->parent = parent;

	// Grandparent takes previous right child of parent as left child
	grandparent->left = right_child;

	if (right_child)
		right_child->parent = grandparent;

	return parent;
}

// Perform left rotation on grandparent and return node that replaces it in the tree
static Node left_rotation(Node grandparent)
{
	Node parent = grandparent->right;
	Node left_child = parent->left;

	// Parent takes grandparent's place
	if (!node_is_root(grandparent))
	{
		if (node_is_left(grandparent))
			grandparent->parent->left = parent;
		else
			grandparent->parent->right = parent;
	}

	parent->parent = grandparent->parent;

	// Grandparent becomes left child of parent
	parent->left = grandparent;
	grandparent->parent = parent;

	// Grandparent takes previous left child of parent as right child
	grandparent->right = left_child;

	if (left_child)
		left_child->parent = grandparent;

	return parent;
}

// Fix double red problem starting from child
static void fix_double_red(Node child)
{
	Node parent = child->parent;

	while (child->color == RED && parent->color == RED)
	{
		Node grandparent = parent->parent;
		Node uncle = node_sibling(parent);

		// If child's uncle is black, perform a trinode restructuring
		if (node_color(uncle) == BLACK)
		{
			Node new_parent;

			// If parent is on the left
			if (node_is_left(parent))
			{
				// If child is on the right, perform a double left-right rotation
				// Otherwise, perform only a right rotation

				if (node_is_right(child))
					left_rotation(parent);

				new_parent = right_rotation(grandparent);
			}

			// If parent is on the right
			else
			{
				// If child is on the left, perform a double right-left rotation
				// Otherwise, perform only a left rotation

				if (node_is_left(child))
					right_rotation(parent);

				new_parent = left_rotation(grandparent);
			}

			// Recolor nodes
			new_parent->color = BLACK;
			new_parent->left->color = RED;
			new_parent->right->color = RED;
			break;
		}

		// If child's uncle is red, do a recoloring
		else
		{
			parent->color = uncle->color = BLACK;

			if (!node_is_root(grandparent))
				grandparent->color = RED;

			child = grandparent;
			parent = child->parent;
		}
	}
}

// Fix double black problem starting from child
static void fix_double_black(Node child, Node parent, Node sibling)
{
	while (true)
	{
		// If child's sibling is red, perform an adjustment operation
		// This changes the child's sibling to black and the two other cases need to be considered
		if (node_color(sibling) == RED)
		{
			// Recolor sibling and parent
			sibling->color = BLACK;
			parent->color = RED;

			// Perform a rotation
			if (node_is_left(sibling))
			{
				right_rotation(parent);
				sibling = parent->left;
			}
			else
			{
				left_rotation(parent);
				sibling = parent->right;
			}
		}

		// If child's sibling is black and has a red child, perform a trinode restructuring
		// Different rotations are performed if the red child is on the left or right of the sibling.
		if (node_color(sibling->left) == RED)
		{
			Node new_parent; // Node that will replace current parent of child after rotations

			if (node_is_right(sibling))
			{
				right_rotation(sibling);
				new_parent = left_rotation(parent);
			}
			else
				new_parent = right_rotation(parent);

			new_parent->color = parent->color;
			new_parent->left->color = BLACK;
			new_parent->right->color = BLACK;
			break; // Double black problem is fixed
		}
		else if (node_color(sibling->right) == RED)
		{
			Node new_parent;

			if (node_is_left(sibling))
			{
				left_rotation(sibling);
				new_parent = right_rotation(parent);
			}
			else
				new_parent = left_rotation(parent);

			new_parent->color = parent->color;
			new_parent->left->color = BLACK;
			new_parent->right->color = BLACK;
			break;
		}

		// If child's sibling is black and both its children are black, do a recoloring
		else
		{
			// Color sibling red
			sibling->color = RED;
			
			// If parent is red or is the root, it is colored black and the problem is fixed
			if (parent->color == RED || node_is_root(parent))
			{
				parent->color = BLACK;
				break;
			}

			// If parent is black, it is colored double black and the problem reappears
			child = parent;
			parent = child->parent;
			sibling = node_sibling(child);
		}
	}
}



///////////////////////////////////////// MAIN FUNCTIONS ///////////////////////////////////////////

RBTree rbt_init(cmpFunc cmp, destroyFunc destroy)
{
	if (!cmp)
		return NULL;

	RBTree tree = malloc(sizeof(struct rbtree));
	if (!tree)
		return NULL;

	tree->root = NULL;
	tree->size = 0;
	tree->cmp = cmp;
	tree->destroy = destroy;
	tree->flag = OK;

	return tree;
}

bool rbt_empty(RBTree rbt)
{
	return (rbt->size == 0);
}

size_t rbt_size(RBTree rbt)
{
	return rbt->size;
}

bool rbt_insert(RBTree rbt, void *data)
{
	// If tree is empty, create root
	if (!rbt->root)
	{
		rbt->root = node_new(data, NULL);
		ERR_ALLOC(rbt, rbt->root);

		// Root is colored black
		rbt->root->color = BLACK;
		rbt->size++;
		return true;
	}

	Node parent = rbt->root; // Parent of new element

	// Search tree to insert new element
	while (true)
	{
		int cmp_val = rbt->cmp(data, parent->data);

		if (!cmp_val) // Element already in tree
			return false;

		if (cmp_val < 0)
		{
			// If left node of parent is NULL, insert element
			if (!parent->left)
			{
				parent->left = node_new(data, parent);
				ERR_ALLOC(rbt, parent->left)

				// Fix double red if it appears
				fix_double_red(parent->left);
				break;
			}

			parent = parent->left;
		}
		else
		{
			// Do the same to insert in right node
			if (!parent->right)
			{
				parent->right = node_new(data, parent);
				ERR_ALLOC(rbt, parent->right)

				fix_double_red(parent->right);
				break;
			}

			parent = parent->right;
		}
	}

	// If root has changed, change pointer
	if (!node_is_root(rbt->root))
		rbt->root = rbt->root->parent;

	rbt->size++;
	return true;
}

void *rbt_search(RBTree rbt, void *data)
{
	Node cur_node = rbt->root;

	while (cur_node)
	{
		int cmp_val = rbt->cmp(data, cur_node->data);

		if (!cmp_val)
			return cur_node->data;

		cur_node = cmp_val < 0 ? cur_node->left : cur_node->right;
	}

	return NULL;
}

bool rbt_remove(RBTree rbt, void *data)
{
	if (!rbt->size)
		return false;

	// If root has at most one child	
	if (rbt->size < 3)
	{
		// If element is in the root, change root to child
		if (!rbt->cmp(data, rbt->root->data))
		{
			Node old_root = rbt->root;
			rbt->root = !rbt->root->right ? rbt->root->left : rbt->root->right;
			node_destroy(old_root, rbt->destroy);

			if (rbt->root)
			{
				rbt->root->color = BLACK;
				rbt->root->parent = NULL;
			}

			rbt->size--;
			return true;
		}
	}

	Node del_node = rbt->root; // Node that will be removed

	// Search tree for node storing element
	while (del_node)
	{
		int cmp_val = rbt->cmp(data, del_node->data);

		if (!cmp_val)
		{
			// If node does not have a NULL child, swap it with its predecessor in the tree and delete predecessor
			if (del_node->right && del_node->left)
				del_node = node_swap_with_pred(del_node);

			// Parent and sibling of node that is deleted
			Node parent = del_node->parent;
			Node sibling = node_sibling(del_node);

			// Remove node and save the child that replaces it
			Node child = node_remove(del_node);

			// If deleted node was black, depth property is violated
			if (del_node->color == BLACK)
			{
				// If child is red, color it black
				if (node_color(child) == RED)
					child->color = BLACK;
				// If child is black, a double black appears
				else
					fix_double_black(child, parent, sibling);
			}

			node_destroy(del_node, rbt->destroy);
			rbt->size--;

			// If root has changed, change pointer
			if (!node_is_root(rbt->root))
				rbt->root = rbt->root->parent;

			return true;
		}

		 del_node = cmp_val < 0 ? del_node->left : del_node->right;
	}

	return false;
}

void *rbt_min(RBTree rbt)
{
	ERR_EMPTY(rbt)
	Node min_node = rbt->root;

	// Return rightmost element of tree
	while (min_node->right)
		min_node = min_node->right;

	return min_node->data;
}

void *rbt_max(RBTree rbt)
{
	ERR_EMPTY(rbt)
	Node max_node = rbt->root;

	// Return leftmost element of tree
	while (max_node->left)
		max_node = max_node->left;

	return max_node->data;
}

void *rbt_copy(RBTree rbt, copyFunc copy)
{
	RBTree rbt2 = rbt_init(rbt->cmp, rbt->destroy);
	ERR_ALLOC(rbt, rbt2)

	rbt2->size = rbt->size;

	if (copy)
		rbt2->root = node_copy_deep(rbt->root, copy, rbt->destroy);
	else
		rbt2->root = node_copy_shallow(rbt->root);

	if (!rbt2->root)
	{
		rbt_destroy(rbt2);
		rbt->flag = ALLOC;
		return NULL;
	}

	return rbt2;
}

void rbt_clear(RBTree rbt)
{
	node_destroy_all(rbt->root, rbt->destroy);
	rbt->root = NULL;
	rbt->size = 0;
}

void rbt_destroy(RBTree rbt)
{
	node_destroy_all(rbt->root, rbt->destroy);
	free(rbt);
}

void rbt_set_destroy(RBTree rbt, destroyFunc destroy)
{
	rbt->destroy = destroy;
}

int rbt_flag(RBTree rbt)
{
	return rbt->flag;
}