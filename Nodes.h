#include <string>
#include <vector>
#include <atomic>

using namespace std;

class LeafNode {
public:
  string val;
  string hash;

  LeafNode(string _val, string _hash);
};


LeafNode::LeafNode(string _val, string _hash) {
  val = _val;
  hash = _hash;
}

class MerkleNode {
public:
  string hash;
  string encoding; // Needed>
  LeafNode* leaf;
  MerkleNode* left;
  MerkleNode* right;
  mutex mtx;
  int visited;

  MerkleNode(string _hash, string _encoding, LeafNode* _leaf);
  MerkleNode(string _hash, LeafNode* _leaf);
  MerkleNode(string hash);
  MerkleNode();
};

MerkleNode::MerkleNode(string _hash, string _encoding, LeafNode* _leaf) {
  hash = _hash;
  leaf = _leaf;
  encoding = _encoding;
  visited = 0;
  left = NULL;
  right = NULL;
}

MerkleNode::MerkleNode(string _hash, LeafNode* _leaf) {
  hash = _hash;
  leaf = _leaf;
  encoding = "-1";
  visited = 0;
  left = NULL;
  right = NULL;
}

MerkleNode::MerkleNode(string _hash) {
  hash = _hash;
  leaf = NULL;
  encoding = "-1";
  visited = 0;
  left = NULL;
  right = NULL;
}

MerkleNode::MerkleNode() {
  hash = -1;
  leaf = NULL;
  encoding = "-1";
  left = NULL;
  right = NULL;
}

class Transaction {
public:
  string val;
  int index;

  Transaction(string _val, int _index);
};


Transaction::Transaction(string _val, int _index) {
  val = _val;
  index = _index;
}


class Proof {
public:
  string val;
  string hash;
  vector<MerkleNode*> siblingHashes;

  Proof(string _val, string _hash, vector<MerkleNode*> _siblingHashes);
};

Proof::Proof(string _val, string _hash, vector<MerkleNode*> _siblingHashes) {
  val = _val;
  hash = _hash;
  siblingHashes = _siblingHashes;
}
