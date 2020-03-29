#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <queue>
#include <atomic>
#include <cmath>
#include <stack>

using namespace std;

template <typename T>
class LeafNode {
public:
  T val;
  string hash;

  LeafNode(T _val, string _hash);
};

template <typename T>
LeafNode<T>::LeafNode(T _val, string _hash) {
  val = _val;
  hash = _hash;
}

class MerkleNode {
public:
  string hash;
  MerkleNode* left;
  MerkleNode* right;

  MerkleNode(string hash);
  MerkleNode();
};

MerkleNode::MerkleNode(string _hash) {
  hash = _hash;
  left = NULL;
  right = NULL;
}

MerkleNode::MerkleNode() {
  hash = -1;
  left = NULL;
  right = NULL;
}

// Helper function for populate()
MerkleNode* recursivePopulate(vector<MerkleNode*> hashedNodes, hash<string> hash) {
  vector<MerkleNode*> upperLevel;
  if(hashedNodes.size() == 1){
    return hashedNodes.at(0);
  }

  for (int i = 0; i < hashedNodes.size() - 1; i = i + 2) {

    stringstream ss;
    ss << (hashedNodes.at(i))->hash << (hashedNodes.at(i+1))->hash;
    stringstream output(ss.str());

    size_t temp = hash(ss.str());
    ss.str("");
    ss << temp;
    MerkleNode* node = new MerkleNode(ss.str());

    node->left = hashedNodes.at(i);
    node->right = hashedNodes.at(i+1);

    upperLevel.push_back(node);
  }

  if(hashedNodes.size() % 2 == 1) {
    stringstream ss;
    size_t temp = hash(hashedNodes.at(hashedNodes.size()-1)->hash);
    ss << temp;
    MerkleNode* node = new MerkleNode(ss.str());

    node->left = hashedNodes.at(hashedNodes.size()-1);
    upperLevel.push_back(node);
  }

  return recursivePopulate(upperLevel, hash);
}

// Returns root of merkle tree
template <typename T>
MerkleNode* populate(vector<LeafNode<T> > leaves) {
  vector<MerkleNode*> base;
  hash<string> hash;

  for (int i = 0; i < leaves.size(); i++) {
    MerkleNode* node = new MerkleNode(leaves.at(i).hash);

    base.push_back(node);
  }

  return recursivePopulate(base, hash);
}

// Compares two vectors of leaf values
template <typename T>
bool validate(vector<LeafNode<T> > a1, vector<LeafNode<T> > a2) {
  MerkleNode* firstRoot = populate(a1);
  MerkleNode* secondRoot = populate(a2);

  return (firstRoot->hash.compare(secondRoot->hash) == 0);
}

// Inserted a new Merkle Node into the right most of the root
MerkleNode* insertLeaf(MerkleNode* root, string passedHash) {
  MerkleNode* temp = root;
  MerkleNode* insertedNode = new MerkleNode(passedHash);

  hash<string> h;
  vector<MerkleNode*> leafMerkles;
  queue<MerkleNode*> q;
  q.push(root);
  while (!q.empty()) {
    MerkleNode* temp = q.front();
    if (temp->left == NULL && temp->right == NULL) {
      leafMerkles.push_back(temp);
    } else if (temp->right == NULL) {
      leafMerkles.push_back(temp->left);
    } else {
      q.push(temp->left);
      q.push(temp->right);
    }
    q.pop();
  }

  leafMerkles.push_back(insertedNode);
  return recursivePopulate(leafMerkles, h);
}

template <typename T>
class MerkleTree {
public:
  atomic<MerkleNode*> root;

  MerkleTree();
  bool insert_leaf(int index, string data);
  MerkleNode* get_signed_root();
  vector<LeafNode<T>> generate_proof(int index);
  bool verify_proof(vector<LeafNode<T>> proof, T data, MerkleNode* root);
};

template <typename T>
MerkleTree<T>::MerkleTree() {
  vector<LeafNode<string> > init;

  hash<string> hash;
  stringstream convert;
  convert << hash("");
  string hashedValue = convert.str();

  for (int i = 0; i < pow(2, 2); i++) {
    LeafNode<string> temp("", hashedValue); // Whatever # of nodes here
    init.push_back(temp);
  }

  root.store(populate(init));
}

template <typename T>
bool MerkleTree<T>::insert_leaf(int index, string data) {
  int last = pow(2, 2) - 1;
  int first = 0;
  int middle = 0;
  int depth = 0; // Depth needs to be check to make sure it's right level

  MerkleNode* temp = root.load();
  hash<string> hash;
  stack<MerkleNode*> siblings;
  stack<MerkleNode*> parents;

  while (first <= last) {
    middle = (first + last) / 2;
    if (middle < index) {
      first = middle + 1;

      parents.push(temp);
      siblings.push(temp->left);
      temp = temp->right;

      depth++;
    } else if (middle > index) {
      last = middle - 1;

      parents.push(temp);
      siblings.push(temp->right);
      temp = temp->left;

      depth++;
    } else {
      if(depth != 2) {
        parents.push(temp);
        siblings.push(temp->right);
        temp = temp->left;

        while (temp->right) {
          parents.push(temp);
          siblings.push(temp->left);
          temp = temp->right;
        }
      }
      cout << parents.size() << endl;
      temp->hash = data;

      // Traverse back up
      while (!parents.empty()) {
        MerkleNode* head = parents.top();

        stringstream ss;
        ss << temp->hash << siblings.top()->hash;
        stringstream output(ss.str());

        size_t hashedTemp = hash(ss.str());
        ss.str("");
        ss << hashedTemp;

        head->hash = ss.str();
        parents.pop();
        siblings.pop();
        temp = head;
      }
      return 1;
    }
  }

  // Above hash values must be updated

  return 0;
}

template <typename T>
MerkleNode* MerkleTree<T>::get_signed_root() {
  return root.load();
}

template <typename T>
vector<LeafNode<T>> MerkleTree<T>::generate_proof(int index) {

}

template <typename T>
bool MerkleTree<T>::verify_proof(vector<LeafNode<T>> proof, T data, MerkleNode* root) {

}

int main() {

  // Create list of nodes with precreated hashes through hash<T>
  // populate Merkle Tree with list
  hash<string> test;

  string A = "A";
  string B = "B";
  string C = "C";

  stringstream convert;
  convert << test(A);
  string hashA = convert.str();

  convert.str("");
  convert << test(B);
  string hashB = convert.str();

  convert.str("");
  convert << test(C);
  string hashC = convert.str();

  LeafNode<int> tempA(1, hashA);
  LeafNode<int> tempB(2, hashB);
  LeafNode<int> tempC(3, hashC);

  vector<LeafNode<int> > result;
  result.push_back(tempA);
  result.push_back(tempB);

  vector<LeafNode<int> > tester;
  tester.push_back(tempA);
  tester.push_back(tempB);
  tester.push_back(tempC);

  MerkleNode* root = populate(result);
  MerkleNode* newRoot = insertLeaf(root, "D");

  cout << "D: Before MerkleTree" << endl;
  MerkleTree<int> tree;
  tree.insert_leaf(0, "123");

  MerkleNode* treeNode = tree.get_signed_root();
  while(treeNode->left->left){
    treeNode = treeNode->left;
  }

  cout << "First Hash: " << treeNode->left->hash << " Second Hash: " << treeNode->right->hash << endl;
  cout << "Head hash: " << treeNode->hash << endl;
  cout << "MerkleRoot: " << tree.get_signed_root()->hash << endl;
  tree.insert_leaf(1, "777");
  cout << "First Hash: " << treeNode->left->hash << " Second Hash: " << treeNode->right->hash << endl;
  cout << "Head hash: " << treeNode->hash << endl;
  cout << "MerkleRoot: " << tree.get_signed_root()->hash << endl;


  cout << "Are the trees the same: " << validate(result, tester) << endl;
}
