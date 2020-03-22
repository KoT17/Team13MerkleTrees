#include <iostream>
#include <string>
#include <vector>
#include <functional>

using namespace std;

template <typename T>
class LeafNode {
public:
  T val;
  size_t hash;

  LeafNode(T _val, size_t _hash);
};

template <typename T>
LeafNode<T>::LeafNode(T _val, size_t _hash) {
  val = _val;
  hash = _hash;
}

class MerkleNode {
public:
  size_t hash;
  MerkleNode* left;
  MerkleNode* right;

  MerkleNode(size_t hash);
  MerkleNode();
};

MerkleNode::MerkleNode(size_t _hash) {
  hash = _hash;
  left = NULL;
  right = NULL;
}

MerkleNode::MerkleNode() {
  hash = -1;
  left = NULL;
  right = NULL;
}

class MerkleTree {
public:
  MerkleNode root;

  MerkleTree(MerkleNode _root);
};

MerkleTree::MerkleTree(MerkleNode _root) {
  root = _root;
}

MerkleNode* recursivePopulate(vector<MerkleNode*> hashedNodes, hash<size_t> hash) {
  vector<MerkleNode*> upperLevel;
  cout << "D: Recursing through hashedNodes with size (" << hashedNodes.size() << ")" << endl;
  if(hashedNodes.size() == 1){
    cout << "D: Root Node has been reached at address (" << hashedNodes.at(0) << ") and hash (" << hashedNodes.at(0)->hash << ")" << endl;
    return hashedNodes.at(0);
  }

  for (int i = 0; i < hashedNodes.size() - 1; i = i + 2) {
    MerkleNode* node = new MerkleNode(hash((hashedNodes.at(i))->hash + (hashedNodes.at(i+1))->hash));

    node->left = hashedNodes.at(i);
    node->right = hashedNodes.at(i+1);

    upperLevel.push_back(node);
  }

  if(hashedNodes.size() % 2 == 1) {
    MerkleNode* node = new MerkleNode(hash(hashedNodes.at(hashedNodes.size()-1)->hash));

    node->left = hashedNodes.at(hashedNodes.size()-1);
    upperLevel.push_back(node);
  }

  return recursivePopulate(upperLevel, hash);
}

// Returns root of merkle tree
template <typename T>
MerkleNode* populate(vector<LeafNode<T> > leaves) {
  cout << "D: Entering Populate Function" << endl;
  vector<MerkleNode*> base;
  hash<size_t> hash;

  for (int i = 0; i < leaves.size(); i++) {
    MerkleNode* node = new MerkleNode(leaves.at(i).hash);

    base.push_back(node);
  }

  cout << "D: Entering Recursive Function" << endl;
  return recursivePopulate(base, hash);
}

int main() {

  // Create list of nodes with precreated hashes through hash<T>
  // populate Merkle Tree with list

  hash<string> test;

  string A = "A";
  string B = "B";
  string C = "C";
  string D = "D";
  size_t hashA = test(A);
  size_t hashB = test(B);
  size_t hashC = test(C);
  size_t hashD = test(D);

  LeafNode<int> tempA(1, hashA);
  LeafNode<int> tempB(2, hashB);
  LeafNode<int> tempC(3, hashC);
  LeafNode<int> tempD(4, hashD);

  vector<LeafNode<int> > result;
  result.push_back(tempA);
  result.push_back(tempB);
  result.push_back(tempC);
  result.push_back(tempD);

  MerkleNode* root = populate(result);

  cout << "Root hash: " << root->hash << " Left Hash: " << root->left->hash << " Right Hash: " << root->right->hash << endl;

}
