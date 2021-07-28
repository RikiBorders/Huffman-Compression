/***********************************************
* Program Stem: Alistair Campbell
*
* Author: Riki Borders
*
* Description: Encode & decode file data utilizing huffman trees.
*************************************************/

#include <iostream>
#include <fstream>
#include <cassert>

// Function signature declarations
void error(const std::string &);
void encode(int argc, char **argv); // the encoder program
void decode(int argc, char **argv); // the decoder program

// Class and struct declarations
struct hnode {
  unsigned char character;
  size_t count;
  hnode * left;
  hnode * right;
  hnode(unsigned char ch, size_t c, hnode* l = nullptr, hnode *r = nullptr) :
    character(ch), count(c), left(l), right(r) {}
};


bool hnodeless(hnode *t1, hnode *t2);
// True if t1's count is less than t2's count or (t1's count equals t2's
// count and t1's character is less than t2's character

void write_tree(hnode *tree);
// Write data in a linear fashion: Lc for a leaf, It1t2 for an internal node.

void get_codes(hnode *tree, std::string *codes, std::string pathtaken);
// Traverse the tree, keeping track of the path (sequence of 0s and 1s) to each
// leaf. At the leaves, assign to codes[tree->character] the path taken.

void write_header(size_t num_characters, hnode *tree);
// Write the num_characters in decimal form using cout << without newline.
// Then call write_tree.

void write_content(char *filename, std::string *codes);
// Open filename for reading, read each character. Accumulate the code for each
// character read.  Handle the accumulation this way:
// ** While the accumulated bit string has at least 8 bits, remove the front
// 8 bits, convert that with bitstring_to_char, and write out a single byte. **
// After all the characters have been read from the file, if there are
// any remaining accumulated bits, add zeros to the end to make a multiple
// of 8 bits, then similarly handle the accumulation.

unsigned char bitstring_to_char(std::string s);

std::string char_to_bitstring(unsigned char ch);

// And these signatures are useful for the decode program:

hnode *read_tree();
// read the tree from standard input

void read_header(size_t & num_characters, hnode * & tree);
// read the header information from standard input.

hnode *read_bytes_to_leaf(hnode *root, std::string & bit_buffer);
// Follow a path in the tree from the root to a leaf according to
// the bits in bit_buffer.  The best algorithm modifies bit_buffer
// as you go, so that once a bit is used, it is removed from the buffer.
// Whenever the buffer is exhausted, a character is read from standard
// input, (std::cin.get()), converted to an 8-bit string, and appended
// to the buffer.  If any bits remain in the buffer when the leaf is reached
// they are retained for the next call to this function.  A pointer to the
// found leaf is returned.


bool hnodeless(hnode *t1, hnode *t2){
  if((t1->count <= t2->count) && (t1->character < t2->character))
    return true;
  else
    return false;

}

void write_tree(hnode *tree){
  if((!tree->left) && (!tree->right))
    std::cout << "L" << tree->character;
  else{ //Follow internal nodes to leaves (left to right)
    std::cout << "I";
    write_tree(tree->left);
    write_tree(tree->right);
  }
}

void get_codes(hnode *tree, std::string *codes, std::string pathtaken){
  char left = '0';
  char right = '1';
  if((!tree->left) and (!tree->right)){ //save path taken for leaf
    codes[tree->character] = pathtaken;
  }else{ //update the path and continue searching
    get_codes(tree->left, codes, pathtaken+left);
    get_codes(tree->right, codes, pathtaken+right);
  }
}

void write_header(size_t num_characters, hnode *tree){
  std::cout << num_characters;
  write_tree(tree);
}

unsigned char bitstring_to_char(std::string s){
  unsigned char c = 0; //initialize char and count
  unsigned char v = 128;

  for(size_t i = 0; i < 8; i++){
    if(s[i] == '1')
      c += v;
    v /= 2;
  }
  return c;
}


std::string char_to_bitstring(unsigned char ch){
  std::string s = ""; //initialize the bitstring and count
  unsigned char v = 128;
  for(size_t i = 0; i < 8; i++){
    if(ch >= v){
      s += '1';
      ch = ch - v;
    }
    else
      s += '0';
    v = v / 2;
  }

  return s;
}


void write_content(char *filename, std::string *codes){
  std::string bitstring;
  unsigned char ch;
  std::string front;
  std::ifstream input(filename);

  while(input) {
    ch = input.get();
    bitstring += codes[ch]; //add character's path to bitstring

    if(bitstring.length() == 8){ //convert 8 bits to characters
      std::cout << bitstring_to_char(bitstring);
      bitstring = "";
    }else if (bitstring.length() > 8){ //if bigger than 8, take front 8 bits
      front = bitstring.substr(0, 8);
      std::cout << bitstring_to_char(front);
      bitstring = bitstring.substr(8);
    }
  }

  //Handle remaining bits by adding zeroes
  if (bitstring.length() != 0){
    while(bitstring.length() != 8)
      bitstring += '0';
    std::cout << bitstring_to_char(bitstring);
  }

  input.close();
}


hnode *read_bytes_to_leaf(hnode *root, std::string & bit_buffer){
  unsigned char x;
  if((!root->left) && (!root->right)){ //return found leaf
    return root;
  }

  if(bit_buffer.length() == 0){ //if buffer is exhausted, read in a character
    x = std::cin.get();
    bit_buffer += char_to_bitstring(x);
  }

  if(bit_buffer[0] == '1'){ //move to next node and update buffer
    bit_buffer.erase(0, 1);
    root = read_bytes_to_leaf(root->right, bit_buffer);

  }else if(bit_buffer[0] == '0'){
    bit_buffer.erase(0, 1);
    root = read_bytes_to_leaf(root->left, bit_buffer);
  }


  return root;
}

hnode *read_tree(){
  hnode * tree;
  std::string i = "";
  i = std::cin.get();
  if(i == "L"){ //handle leaf
    tree = new hnode(std::cin.get(), 0, nullptr, nullptr);
  } else if(i == "I") //handle internal node
    tree = new hnode(0, 0, read_tree(), read_tree());

  return tree;
}


void read_header(size_t & num_characters, hnode * & tree){
  std::cin >> num_characters; //get the characters
  tree = read_tree(); //read in the tree
}


class priority_queue {
private:
  struct node {
    hnode *data;
    node *next;
    node(hnode *d, node *n) : data(d), next(n) {}
  };

  static void _add(hnode *item, node * & list)
  {
    if (!list or hnodeless(item, list->data))
      list = new node(item, list);
    else
      _add(item, list->next);
  }

  node * _front;
  size_t _size;
public:
  priority_queue() { _size = 0; _front = nullptr; }
  size_t size() { return _size; }
  void add(hnode * item)
  {
    _add(item, _front);
    _size++;
  }

  hnode *remove()
  {
    hnode *item = _front->data;
    node *old = _front;
    _front = _front->next;
    delete old;
    _size--;
    return item;
  }
};


void get_counts(char * filename, size_t *counts, size_t & num_characters)
{
  for (size_t i=0; i < 256; i++)
    counts[i] = 0;
  std::ifstream input(filename);
  if (!input)
    error(std::string("Can't open ") + filename + " for reading");
  while(input) {
    int ch = input.get();
    if (input) {
      counts[ch]++;
      num_characters++;
    }
  }
  input.close();
}


void encode(int argc, char **argv)
{
  if (argc != 2)
    error(std::string("Usage: encode filename"));
  size_t counts[256];
  std::string codes[256];
  size_t num_characters = 0;
  get_counts(argv[1], counts, num_characters);
  priority_queue queue; // contains hnode *
  for (size_t i=0; i<256; i++)
    if (counts[i] > 0) {
      queue.add(new hnode(i, counts[i]));
    }
  while (queue.size() > 1) {
    hnode * left = queue.remove();
    hnode * right = queue.remove();
    hnode * baby = new hnode(left->character, left->count + right->count,
                             left, right);
    queue.add(baby);
  }
  hnode *tree = queue.remove();
  get_codes(tree, codes, "");
  write_header(num_characters, tree);
  write_content(argv[1], codes);
}

void decode(int argc , char **/* argv */)
{
  if (argc != 1)
    error(std::string("Usage: decode"));
  size_t num_characters;
  hnode *tree;
  read_header(num_characters, tree);
  size_t count = 0;
  std::string bit_buffer = "";
  while (count < num_characters) {
    hnode *leaf = read_bytes_to_leaf(tree, bit_buffer);
    std::cout << leaf->character;
    count++;
  }
}

void error(const std::string & message)
{
  std::cout << message << std::endl;
  exit(1);
}

int main(int argc, char **argv)
{
  if (std::string(argv[0]) == "./encode")
    encode(argc, argv);
  else if (std::string(argv[0]) == "./decode")
    decode (argc, argv);
  else
    error(std::string("Usage: encode file OR decode"));
}
