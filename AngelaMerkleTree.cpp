#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <queue>
#include <atomic>
#include <cmath>
#include <stack>
#include <algorithm>
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>

#include "Nodes.h"

using namespace std;
using namespace chrono;

MerkleNode* populate(vector<LeafNode> leaves);
string findEncoding(int index, int depth);

class MerkleTree {
public:
  atomic<MerkleNode*> root;
  vector<string> conflicts;

  MerkleTree();
  bool insert_leaf(int index, string data);
  MerkleNode* get_signed_root();
  Proof* generate_proof(int index);
  bool verify_proof(Proof* proof, string data, MerkleNode* root);
  void batch_update(vector<Transaction> trans, int ind);
  vector<string> find_conflicts(vector<Transaction> trans);
  void update(Transaction trans);
};


// Constructor of Merkle Tree
MerkleTree::MerkleTree() {
  vector<LeafNode> init;

  hash<string> hash;
  stringstream convert;
  convert << hash("");
  string hashedValue = convert.str();

  for (int i = 0; i < pow(2, 20); i++) {
    LeafNode temp("", hashedValue);
    init.push_back(temp);
  }

  root.store(populate(init));
}


// Insert function to change leaf at index with data
bool MerkleTree::insert_leaf(int index, string data) {
  int last = pow(2, 20) - 1;
  int first = 0;
  int middle = 0;
  int depth = 0;

  MerkleNode* temp = root.load();
  hash<string> hash;
  stack<MerkleNode*> siblings;
  stack<MerkleNode*> parents;

  // Modified BST with stack travsal
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
      if(depth != 20) {
        parents.push(temp);
        siblings.push(temp->right);
        temp = temp->left;

        while (temp->right) {
          parents.push(temp);
          siblings.push(temp->left);
          temp = temp->right;
        }
      }

      size_t hashed = hash(data);
      stringstream ss;
      ss << hashed;
      LeafNode* leaf = new LeafNode(data, ss.str());

      temp->hash = ss.str();
      temp->leaf = leaf;

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

  return 0;
}

// Returns Merkle Root of tree
MerkleNode* MerkleTree::get_signed_root() {
  return root.load();
}

// Retrieves the proof object from index
Proof* MerkleTree::generate_proof(int index) {
  int last = pow(2, 20) - 1;
  int first = 0;
  int middle = 0;
  int depth = 0;

  MerkleNode* temp = root.load();
  hash<string> hash;
  stack<MerkleNode*> siblings;

  while (first <= last) {
    middle = (first + last) / 2;
    if (middle < index) {
      first = middle + 1;
      siblings.push(temp->left);
      temp = temp->right;
      depth++;
    } else if (middle > index) {
      last = middle - 1;
      siblings.push(temp->right);
      temp = temp->left;
      depth++;
    } else {
      if(depth != 20) {
        siblings.push(temp->right);
        temp = temp->left;

        while (temp->right) {
          siblings.push(temp->left);
          temp = temp->right;
        }
      }

      vector<MerkleNode*> proofHashes;
      while (!siblings.empty()) {
        MerkleNode* temp = siblings.top();
        proofHashes.push_back(temp);
        siblings.pop();
      }

      stringstream ss;
      ss <<  hash("");

      if (temp->hash.compare(ss.str()) == 0)
        return NULL;
      Proof* result = new Proof(temp->leaf->val, temp->hash, proofHashes);
      return result;
    }
  }
  return NULL;
}

// Verifies if proof values are inside of data
bool MerkleTree::verify_proof(Proof* proof, string data, MerkleNode* root) {
  if (proof == NULL)
    return false;

  hash<string> hash;
  size_t hashedData = hash(data);
  int size;
  size_t newHash;
  stringstream ss;

  if (proof) {
    size = proof->siblingHashes.size();
    ss << hashedData << proof->siblingHashes.at(0)->hash;
    newHash = hash(ss.str());
    ss.str("");

    for (int i = 1; i < size; i++) {
      ss << newHash << proof->siblingHashes.at(i)->hash;
      newHash = hash(ss.str());
      ss.str("");
    }

    ss << newHash;

    return (ss.str().compare(root->hash) == 0);
  }

  return false;
}

vector<string> MerkleTree::find_conflicts(vector<Transaction> trans) {
  int depth = 20;
  vector<string> encoded;
  vector<string> conflicts;
  stringstream ss;

  for (int i = 0; i < trans.size(); i++) {
    string encoding = findEncoding(trans.at(i).index, depth);
    encoded.push_back(encoding);
  }


  for (int i = 0; i < encoded.size() - 1; i++) {
    for (int j = i + 1; j < encoded.size(); j++){

      for (int z = 0; z < depth; z++){
        if (encoded.at(i).substr(z, 1).compare(encoded.at(j).substr(z, 1)) == 0)
          ss << encoded.at(i).substr(z, 1);
        else
          break;
      }

      string potentialConflict = ss.str();
      if (potentialConflict.empty()) {
        potentialConflict = "-1";
      }
      if (!(find(conflicts.begin(), conflicts.end(), potentialConflict) != conflicts.end())) {
        conflicts.push_back(potentialConflict);
      }
      ss.str("");
    }
  }
  return conflicts;
}

void MerkleTree::batch_update(vector<Transaction> trans, int ind) {
  this->conflicts = find_conflicts(trans);
  for (int i = ind; i < trans.size(); i++)
    update(trans.at(i));
}

void MerkleTree::update(Transaction trans) {
  int last = pow(2, 20) - 1;
  int first = 0;
  int middle = 0;
  int depth = 0;
  bool isInConflict = false;

  MerkleNode* temp = root.load();
  MerkleNode* prev = temp;

  // Parent retrieval
  while (first <= last) {
    middle = (first + last) / 2;
    if (middle < trans.index) {
      first = middle + 1;
      prev = temp;
      temp = temp->right;
      depth++;
    } else if (middle > trans.index) {
      last = middle - 1;
      prev = temp;
      temp = temp->left;
      depth++;
    } else {
      if(depth != 20) {
        prev = temp;
        temp = temp->left;

        while (temp->right) {
          prev = temp;
          temp = temp->right;
        }
      }
      break;
    }
  }

  // Concurrent parent conflict resolving
  for (int i = 0; i < this->conflicts.size(); i++) {
    if (prev->encoding.compare(this->conflicts.at(i)) == 0)
      isInConflict = true;
  }

  if (isInConflict) {
    prev->mtx.lock();
    if (prev->visited == 0) {
      prev->visited = 1;
      return;
    }
    else {
      insert_leaf(trans.index, trans.val);
    }
    prev->mtx.unlock();
  }
  else {
    insert_leaf(trans.index, trans.val);
  }

  return;
}

// Returns encoding of Node based on index and depth of Node
string findEncoding(int index, int depth) {
  if (depth == 0)
    return "-1";

  vector<int> encode(depth);

  for (int i = 0; index > 0; i++) {
    encode.at(i) = index % 2;
    index = index / 2;
  }

  stringstream ss;
  for (int i = encode.size() - 1; i > -1; i--)
    ss << encode.at(i);
  return ss.str();
}

// Used for base sparse tree creation
MerkleNode* recursivePopulate(vector<MerkleNode*> hashedNodes, hash<string> hash, int depth) {
  vector<MerkleNode*> upperLevel;
  if(hashedNodes.size() == 1){
    hashedNodes.at(0)->encoding = "-1";
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
    node->encoding = findEncoding(i / 2, depth);

    upperLevel.push_back(node);
  }

  return recursivePopulate(upperLevel, hash, depth - 1);
}

// Returns root of merkle tree
MerkleNode* populate(vector<LeafNode> leaves) {
  vector<MerkleNode*> base;
  hash<string> hash;
  int depth = 20;

  for (int i = 0; i < leaves.size(); i++) {
    MerkleNode* node = new MerkleNode(leaves.at(i).hash, findEncoding(i, depth), &leaves.at(i));

    base.push_back(node);
  }

  return recursivePopulate(base, hash, depth - 1);
}

// Thread function
void run(int randNum[4], int id, MerkleTree *tree, vector<Transaction> trans, int st) {
  Proof* proof;
  bool res = false;

  for (int i = 0; i < 4; i++) {
    if (randNum[i] == 0 || randNum[i] == 4) {
      tree->insert_leaf(trans.at(i).index, trans.at(i).val);
    }
    else if (randNum[i] == 1 || randNum[i] == 5 || randNum[i] == 6) {
      tree->get_signed_root();
    }
    else if (randNum[i] == 2 || randNum[i] == 7 || randNum[i] == 8 || randNum[i] == 9) {
      proof = tree->generate_proof(trans.at(i).index);
      res = tree->verify_proof(proof, trans.at(i).val, tree->get_signed_root());
    }
    else if (randNum[i] == 3) {
      tree->batch_update(trans, st);
    }
  }
}

int main() {
  vector<thread> threads;
  int randNum[4];
  vector<Transaction> trans;

  srand (time(NULL));
  int st = 0;

  for (int i = 0; i < 4; i++) {
    randNum[i] = rand() % 10;
  }

  Transaction tA("A", 0);
  Transaction tB("B", 3);
  Transaction tC("C", 7);
  Transaction tD("D", 5);
  Transaction tE("E", 2);

  trans.push_back(tA);
  trans.push_back(tB);
  trans.push_back(tC);
  trans.push_back(tD);
  trans.push_back(tE);

  MerkleTree tree;

  auto start = high_resolution_clock::now();
  for (int tnum = 0; tnum < 1; tnum++)
  {
    threads.push_back(thread(run, randNum, tnum, &tree, trans, st));
    st++;
  }

  for_each(threads.begin(), threads.end(), mem_fn(&thread::join));

  auto end = high_resolution_clock::now();
  cout << "Time: " << duration_cast<nanoseconds>(end - start).count() << "ns" << endl;
}
