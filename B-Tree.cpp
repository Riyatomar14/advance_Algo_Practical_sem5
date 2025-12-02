#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

class BTreeNode {
public:
    int order;   
    int minKeys; 
    bool isLeaf;
    vector<int> keys;
    vector<BTreeNode*> children;

    BTreeNode(int _order, bool _isLeaf)
        : order(_order), isLeaf(_isLeaf) 
    {
        minKeys = ceil(order / 2.0) - 1;
    }

    void traverse();
    void insertNonFull(int key);
    void splitChild(int i);

    // deletion
    void remove(int key);
    int findKey(int key);
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    int getPredecessor(int idx);
    int getSuccessor(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);

    ~BTreeNode() {
        for (auto c : children)
            delete c;
    }
};

class BTree {
public:
    BTreeNode* root;
    int order;

    BTree(int _order) : root(nullptr), order(_order) {}

    void insert(int key);
    void remove(int key);
    void traverse() {
        if (root) root->traverse();
    }

    ~BTree() {
        delete root;
    }
};

////////////////////
///  TRAVERSE   ///
////////////////////
void BTreeNode::traverse() {
    int i;
    for (i = 0; i < keys.size(); i++) {
        if (!isLeaf)
            children[i]->traverse();
        cout << keys[i] << " ";
    }
    if (!isLeaf)
        children[i]->traverse();
}

////////////////////
///  INSERTION   ///
////////////////////
void BTree::insert(int key) {

    if (!root) {
        root = new BTreeNode(order, true);
        root->keys.push_back(key);
        return;
    }

    if (root->keys.size() == order - 1) {
        BTreeNode* newRoot = new BTreeNode(order, false);
        newRoot->children.push_back(root);
        newRoot->splitChild(0);

        int i = 0;
        if (newRoot->keys[0] < key)
            i++;

        newRoot->children[i]->insertNonFull(key);
        root = newRoot;
    }
    else {
        root->insertNonFull(key);
    }
}

void BTreeNode::insertNonFull(int key) {
    int i = keys.size() - 1;

    if (isLeaf) {
        keys.insert(upper_bound(keys.begin(), keys.end(), key), key);
    }
    else {
        while (i >= 0 && keys[i] > key)
            i--;

        i++;

        if (children[i]->keys.size() == order - 1) {

            splitChild(i);

            if (keys[i] < key)
                i++;
        }

        children[i]->insertNonFull(key);
    }
}

void BTreeNode::splitChild(int i) {

    BTreeNode* y = children[i];
    BTreeNode* z = new BTreeNode(order, y->isLeaf);

    int mid = order / 2;

    z->keys.assign(y->keys.begin() + mid + 1, y->keys.end());
    y->keys.erase(y->keys.begin() + mid + 1, y->keys.end());

    if (!y->isLeaf) {
        z->children.assign(y->children.begin() + mid + 1, y->children.end());
        y->children.erase(y->children.begin() + mid + 1, y->children.end());
    }

    children.insert(children.begin() + i + 1, z);
    keys.insert(keys.begin() + i, y->keys[mid]);
    y->keys.erase(y->keys.begin() + mid);
}

////////////////////
///   DELETION   ///
////////////////////
int BTreeNode::findKey(int key) {
    int idx = 0;
    while (idx < keys.size() && keys[idx] < key)
        idx++;
    return idx;
}

void BTree::remove(int key) {
    if (!root)
        return;

    root->remove(key);

    if (root->keys.size() == 0) {
        BTreeNode* old = root;
        if (!root->isLeaf)
            root = root->children[0];
        else
            root = nullptr;

        delete old;
    }
}

void BTreeNode::remove(int key) {
    int idx = findKey(key);

    if (idx < keys.size() && keys[idx] == key) {
        if (isLeaf)
            removeFromLeaf(idx);
        else
            removeFromNonLeaf(idx);
    }
    else {
        if (isLeaf) {
            cout << "Key " << key << " not found.\n";
            return;
        }

        bool flag = (idx == keys.size());

        if (children[idx]->keys.size() <= minKeys) {

            if (idx > 0 && children[idx - 1]->keys.size() > minKeys)
                borrowFromPrev(idx);

            else if (idx < keys.size() &&
                     children[idx + 1]->keys.size() > minKeys)
                borrowFromNext(idx);

            else {
                if (idx < keys.size())
                    merge(idx);
                else
                    merge(idx - 1);
            }
        }

        if (flag && idx > keys.size())
            children[idx - 1]->remove(key);
        else
            children[idx]->remove(key);
    }
}

void BTreeNode::removeFromLeaf(int idx) {
    keys.erase(keys.begin() + idx);
}

void BTreeNode::removeFromNonLeaf(int idx) {
    int k = keys[idx];

    if (children[idx]->keys.size() > minKeys) {
        int pred = getPredecessor(idx);
        keys[idx] = pred;
        children[idx]->remove(pred);
    }
    else if (children[idx + 1]->keys.size() > minKeys) {
        int succ = getSuccessor(idx);
        keys[idx] = succ;
        children[idx + 1]->remove(succ);
    }
    else {
        merge(idx);
        children[idx]->remove(k);
    }
}

int BTreeNode::getPredecessor(int idx) {
    BTreeNode* cur = children[idx];
    while (!cur->isLeaf)
        cur = cur->children.back();
    return cur->keys.back();
}

int BTreeNode::getSuccessor(int idx) {
    BTreeNode* cur = children[idx + 1];
    while (!cur->isLeaf)
        cur = cur->children.front();
    return cur->keys.front();
}

void BTreeNode::borrowFromPrev(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx - 1];

    child->keys.insert(child->keys.begin(), keys[idx - 1]);

    if (!child->isLeaf) {
        child->children.insert(child->children.begin(),
                               sibling->children.back());
        sibling->children.pop_back();
    }

    keys[idx - 1] = sibling->keys.back();
    sibling->keys.pop_back();
}

void BTreeNode::borrowFromNext(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx + 1];

    child->keys.push_back(keys[idx]);

    if (!child->isLeaf) {
        child->children.push_back(sibling->children.front());
        sibling->children.erase(sibling->children.begin());
    }

    keys[idx] = sibling->keys.front();
    sibling->keys.erase(sibling->keys.begin());
}

void BTreeNode::merge(int idx) {

    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx + 1];

    child->keys.push_back(keys[idx]);

    for (int val : sibling->keys)
        child->keys.push_back(val);

    if (!child->isLeaf) {
        for (auto c : sibling->children)
            child->children.push_back(c);
    }

    keys.erase(keys.begin() + idx);
    children.erase(children.begin() + idx + 1);

    delete sibling;
}

////////////////////
///     MAIN     ///
////////////////////
int main() {

    BTree t(5);  

    vector<int> vals = {10, 20, 5, 6, 12, 30, 7, 17};

    for (int v : vals)
        t.insert(v);

    cout << "Traversal before deletion: ";
    t.traverse();
    cout << "\n";

    t.remove(6);
    cout << "After removing 6: ";
    t.traverse();
    cout << "\n";

    t.remove(13);
    cout << "After removing 13 (not present): ";
    t.traverse();
    cout << "\n";

    t.remove(7);
    cout << "After removing 7: ";
    t.traverse();
    cout << "\n";

    return 0;
}
