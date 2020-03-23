#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <atomic>

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

// Helper function for populate()
MerkleNode* recursivePopulate(vector<MerkleNode*> hashedNodes, hash<string> hash) {
  vector<MerkleNode*> upperLevel;

  if(hashedNodes.size() == 1)
    return hashedNodes.at(0);

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

// Thread function for running random commands on the root
void run(atomic<MerkleNode*> *atomicRoot, vector<LeafNode<int>> result, vector<LeafNode<int>> tester, vector<int> random, ResourceAllocatorBase* allocator) {
  hash<string> hash;

  vector<int> resources = {2, 7, 8, 3};
  unsigned numOfRes = 4;
  MRLock<Bitset> mrlock(resources.size());
  MRResourceAllocator res(numOfRes);
  LockableBase* resourceLock = res.CreateLockable(resources);

  resourceLock->Lock();
  MerkleNode* root = atomicRoot->load();
  for(int i = 0; i < 4; i++) {
    if(random[i] == 2)
      root = populate(result);
    else if(random[i] == 1)
      root = populate(tester);
    else if(random[i] == 0)
      cout << "Are the trees the same: " << validate(tester, result) << endl;
    else {
      root = insertLeaf(root, "123456789");
    }
  }
  atomicRoot->store(root);
  resourceLock->Unlock();


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
  result.push_back(tempD);

  vector<LeafNode<int> > tester;
  tester.push_back(tempA);
  tester.push_back(tempB);
  tester.push_back(tempC);

  atomic<MerkleNode*> atom;
  atom.store(populate(result));

  vector<thread> threads(4);
  vector<int> random(4);
  ResourceAllocatorBase* resourceAlloc = new MRResourceAllocator(2);

  for(int i = 0; i < 4; i++){
    random[i] = rand()%4;
  }

  for(int i = 0; i < 4; i++)
    threads[i] = thread(run, &atom, result, tester, random, resourceAlloc);

  for(int i = 0; i < 4; i++)
    threads[i].join();

}
