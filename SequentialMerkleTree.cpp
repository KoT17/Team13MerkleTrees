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
  hash = NULL;
  left = NULL;
  right = NULL;
}

class MerkleTree {
public:
  MerkleNode root;


};

MerkleNode recursivePopulate(vector<MerkleNode> hashedNodes, hash<size_t> hash) {
  vector<MerkleNode> upperLevel;

  if(hashedNodes.size() == 1){
    cout << "Root hash: " << hashedNodes.at(0).hash << " Left Hash: " << hashedNodes.at(0).left->hash << " Right Hash: " << hashedNodes.at(0).right->hash << endl;
    return hashedNodes.at(0); // I believe this returns the root once it was created in the last cycle
  }

  for (int i = 0; i < hashedNodes.size() - 1; i = i + 2) {
    cout << "MN: " << i  << endl;

    cout << "Left Hashed Node: " << (&hashedNodes.at(i))->hash << endl;
    MerkleNode node(hash((&hashedNodes.at(i))->hash + (&hashedNodes.at(i+1))->hash));

    node.left = &hashedNodes.at(i);
    node.right = &hashedNodes.at(i+1);

    cout << "LEFT NODE AFTER HASH: " << node.left->hash << endl;

    upperLevel.push_back(node);
  }

  if(hashedNodes.size() % 2 == 1) {
    MerkleNode node(hash(hashedNodes.at(hashedNodes.size()-1).hash));

    node.left = &hashedNodes.at(hashedNodes.size()-1);
    upperLevel.push_back(node);
  }


  for (int i = 0; i < upperLevel.size(); i++)
    cout << upperLevel.at(i).hash << " ";
  cout << endl;

  cout << "Root hash: " << upperLevel.at(0).hash << " Left Hash: " << upperLevel.at(0).left->hash << " Right Hash: " << upperLevel.at(0).right->hash << endl;
  return recursivePopulate(upperLevel, hash);
}

// Returns root of merkle tree
template <typename T>
MerkleNode populate(vector<LeafNode<T> > leaves) {
  vector<MerkleNode> base;
  hash<size_t> hash;

  for (int i = 0; i < leaves.size(); i++) {
    cout << "Leaf: " << i  << endl;
    MerkleNode node(leaves.at(i).hash);

    base.push_back(node);
  }

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
  //cout << resHash + resHash << endl;

  LeafNode<string> tempA(A, hashA);
  LeafNode<string> tempB(B, hashB);
  LeafNode<string> tempC(C, hashC);
  LeafNode<string> tempD(D, hashD);

  vector<LeafNode<string> > result;
  result.push_back(tempA);
  result.push_back(tempB);
  result.push_back(tempC);
  result.push_back(tempD);

  cout << "TempA hash: " << hashA << " TempB hash: " << hashB << endl;

  MerkleNode root = populate(result);

  cout << "Root hash: " << root.hash << " Left Hash: " << root.left->hash << " Right Hash: " << root.right->hash << endl;


}
