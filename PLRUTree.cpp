#include <iostream>
#include <vector>
#include <cmath>
#include <queue>
using namespace std;

class Node {
public:
    unsigned long long value;
    Node* left;
    Node* right;
    Node(unsigned long long v) {
        value = v;
        left = nullptr;
        right = nullptr;
    }
};

class PLRUTree {
public:
    PLRUTree(int numCacheLines) {
        root = createTree(numCacheLines);
        levels = log2(numCacheLines) + 1;
        //int numNodes = (1 << numCacheLines) - 1;

    }

    Node* createTree(int n) {
        Node* rootNode = new Node(0);
        queue<Node*> q{{rootNode}};
        int i = 1;

        while (i < n) {
            Node* curr = q.front();
            q.pop();

            curr->left = new Node(0ULL);
            q.push(curr->left);
            i++;

            if (i < n) {
                curr->right = new Node(0ULL);
                q.push(curr->right);
                i++;
            }
        }

        return rootNode;
    }

    
    void insert(unsigned long long tag, bool hit) {
        Node* curr = root;
        int level = 0;
        while (level < levels - 2) {
            if (curr->value == 1ULL) {
                curr->value = 0ULL;
                curr = curr->right;
            } else {
                curr->value = 1ULL;
                curr = curr->left;
            }
            level++;
        }
        if (!hit) {
            curr->value = tag;
        }
    }

    bool searchCache(unsigned long long tag) {
        Node* temp = root;
        return searchTree(temp, tag);
    }
    
    bool searchTree(Node* root, unsigned long long value) {
        if (root == nullptr) {
            return false;
        }
        if (root->value == value) {
            return true;
        }
        return searchTree(root->left, value) || searchTree(root->right, value);
    }


    void printTree() {
        Node* rootNode = root;
        if (!rootNode) {
            return;
        }

        queue<Node*> q{{rootNode}};

        int i = 0;
        while (i < levels - 1) {
            int level_size = q.size();

            for (int i = 0; i < level_size; i++) {
                Node* curr = q.front();
                q.pop();
                cout << curr->value << " ";

                if (curr->left) {
                    q.push(curr->left);
                }
                if (curr->right) {
                    q.push(curr->right);
                }
            }
            i++;
            cout << endl;
        }
    }

private:
    Node* root;
    int levels;
};
