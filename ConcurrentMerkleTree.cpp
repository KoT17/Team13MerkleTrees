#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "mrlock.h"
#include "mrlockable.h"

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

class MerkleTree {
public:
  MerkleNode root;

  MerkleTree(MerkleNode _root);
};

MerkleTree::MerkleTree(MerkleNode _root) {
  root = _root;
}

MerkleNode* recursivePopulate(vector<MerkleNode*> hashedNodes, hash<string> hash) {
  vector<MerkleNode*> upperLevel;
  cout << "D: Recursing through hashedNodes with size (" << hashedNodes.size() << ")" << endl;
  if(hashedNodes.size() == 1){
    cout << "D: Root Node has been reached at address (" << hashedNodes.at(0) << ") and hash (" << hashedNodes.at(0)->hash << ")" << endl;
    return hashedNodes.at(0);
  }

  for (int i = 0; i < hashedNodes.size() - 1; i = i + 2) {

    stringstream ss;
    cout << "Hash 1: " << (hashedNodes.at(i))->hash << endl;
    cout << "Hash 2: " << (hashedNodes.at(i+1))->hash << endl;
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
  cout << "D: Entering Populate Function" << endl;
  vector<MerkleNode*> base;
  hash<string> hash;


  for (int i = 0; i < leaves.size(); i++) {
    MerkleNode* node = new MerkleNode(leaves.at(i).hash);

    base.push_back(node);
  }

  cout << "D: Entering Recursive Function" << endl;
  return recursivePopulate(base, hash);
}

template <typename T>
bool validate(vector<LeafNode<T> > a1, vector<LeafNode<T> > a2) {
  MerkleNode* firstRoot = populate(a1);
  MerkleNode* secondRoot = populate(a2);

  cout << "Hash 1: " << firstRoot->hash << endl;
  cout << "Hash 2: " << secondRoot->hash << endl;
  return (firstRoot->hash.compare(secondRoot->hash) == 0);
}

void run(vector<LeafNode<int>> result, vector<LeafNode<int>> tester, vector<int> random, ResourceAllocatorBase* allocator) {
  MerkleNode* root;
  vector<ResourceAllocatorBase::ResourceIdVec> resourceIdVec;
  vector<int> temp(10);
  
  for(int i = 0; i < resourceIdVec.size(); i++) {
    temp.clear();

    for(int i = 0; i < 9; i++) {
      temp.push_back(rand()%100);
    }

    resourceIdVec[i].assign(temp.begin(), temp.end());
  }

  for(unsigned i = 0; i < 2; i++) {
    LockableBase* resourceLock = allocator->CreateLockable(resourceIdVec[i]);
    resourceLock->lock();
  }

  for(int i = 0; i < 4; i++) {
    cout << "random: " << random[i] << endl;

    if(random[i] == 2)
        root = populate(result);
    else if(random[i] == 1)
        root = populate(tester);
    else
        validate(tester, result);
  }
}

int main() {

  // Create list of nodes with precreated hashes through hash<T>
  // populate Merkle Tree with list
  hash<string> test;
  srand(time(NULL));

  string A = "A";
  string B = "B";
  string C = "C";
  string D = "D";

  stringstream convert;
  convert << test(A);
  cout << convert.str() << endl;
  string hashA = convert.str();

  convert.str("");
  convert << test(B);
  string hashB = convert.str();

  convert.str("");
  convert << test(C);
  string hashC = convert.str();

  convert.str("");
  convert << test(D);
  string hashD = convert.str();

  LeafNode<int> tempA(1, hashA);
  LeafNode<int> tempB(2, hashB);
  LeafNode<int> tempC(3, hashC);
  LeafNode<int> tempD(4, hashD);

  vector<LeafNode<int> > result;
  result.push_back(tempA);
  result.push_back(tempB);
  result.push_back(tempC);
  //result.push_back(tempD);

  vector<LeafNode<int> > tester;
  tester.push_back(tempA);
  tester.push_back(tempB);
  tester.push_back(tempC);
  //tester.push_back(tempC);

  MerkleNode* root = populate(result);

  vector<thread> threads(4);
  vector<int> random(4);
  ResourceAllocatorBase* resourceAlloc = new MRResourceAllocator(2);
 
  for(int i = 0; i < 4; i++)
    random[i] = rand()%3;

  for(int i = 0; i < 4; i++)
    threads[i] = thread(run, result, tester, random, resourceAlloc);

  // MRLock<Bitset> lock = new MRLock<Bitset>((uint32_t) threads.at(0));


  for(int i = 0; i < 4; i++)
    threads[i].join();

  cout << validate(result, tester) << endl;
  cout << "Root hash: " << root->hash << " Left Hash: " << root->left->hash << " Right Hash: " << root->right->hash << endl;

}
