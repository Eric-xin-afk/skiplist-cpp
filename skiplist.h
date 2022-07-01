#ifndef SKIPLIST_H
#define SKIPLIST_H

#include<iostream>
#include<stdlib.h>
#include<cmath>
#include<cstring>
#include<mutex>
#include<fstream>
using namespace std;

#define STORE_FILE "store/dumpFile"

mutex mtx;     //临界区的互斥锁
string delimiter = ":";


//Class template to implement skiplist node
template<typename K, typename V>
class Node{
public:
    Node(){}
    Node(K k, V v, int);
    ~Node();

    K get_key() const;
    V get_value() const;
    void set_value(V);

    //数组，用来维护不同层的下一个节点的指针
    Node<K, V> **forward;
    int node_level;

private:
    K key;
    V value;
};


//Class template to implement skiplist 
template<typename K, typename V>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const string& str, string* key, string* value);
    bool is_valid_string(const string& str);

private:
    //跳表的最大层数
    int _max_level;
    //跳表的当前层
    int _skip_list_level;
    //pointer to header node
    Node<K, V>* _header;
    //file operator
    ofstream _file_writer;
    ifstream _file_reader;
    //跳表当前的元素数
    int _element_count;
};




template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level){
    this->key = k;
    this->value = v;
    this->node_level = level;

    //level+1, because array index id from 0 to level
    this->forward = new Node<K, V>*[level+1];
    //fill forward array with 0(NULL)
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level+1) );
}

template<typename K, typename V>
Node<K, V>::~Node(){
    delete[] forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const{
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const{
    return value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v){
    this->value = v;
}





//创建新节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level){
    Node<K, V>* n = new Node<K, V>(k, v, level);
    return n;
}

//构建跳表,有参构造函数
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level){
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    //current header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}


//在跳表中插入给定的键和值,元素已存在返回1，插入成功返回0
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
*/

template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value){
    mtx.lock();
    Node<K, V>* current = this->_header;
    //create update array and initialize it
    //update is an array which put node that the node->forward[i] should be operated later
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level+1));

    //从跳表的最高层开始
    for(int i = _skip_list_level;i>=0;i--){
        while(current->forward[i]!=nullptr && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    //reach level 0 and forward pointer to right node, which is desired to insert key
    current = current->forward[0];
    //if current node has key equal to searched key, we get it
    if(current!=nullptr && current->get_key() == key){
        cout << "key: " << key << ", exits" << endl;
        mtx.unlock();
        return 1;
    }
    //if current node is NULL that means that we have reached to end of the level
    //if current node is not equal to key that means that we should insert node between update[0] and current node
    if(current==nullptr || current->get_key()!=key){
        //Generate a random level for node
        int random_level = get_random_level();
        //if random level is greater than skip list's current level, initialize update value with pointer to header
        if(random_level > _skip_list_level){
            for(int i = _skip_list_level+1;i<random_level+1;++i){
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        //create a node with random level generated
        Node<K, V>* inserted_node = create_node(key, value, random_level);

        //insert node
        for(int i = 0;i<=random_level;i++){
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        cout << "Successfully inserted key: " << key << ", value:" << value << endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

//Display skip list
template<typename K, typename V>
void SkipList<K, V>::display_list(){
    cout << "\n**********Skip List**************" << "\n";
    for(int i = 0;i<=_skip_list_level;i++){
        Node<K, V>* node = this->_header->forward[i];
        cout << "Level " << i << ": ";
        while(node){
            cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        cout << endl;
    }
}

//Dump file in memory to file
template<typename K, typename V>
void SkipList<K, V>::dump_file(){
    cout << "dump_file----------------------------" << endl;
    _file_writer.open(STORE_FILE);
    Node<K, V>* node = this->_header->forward[0];

    while(node){
        _file_writer << node->get_key() << ":" << node->get_value() << endl;
        cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return ;
}

//Load data from disk
template<typename K, typename V>
void SkipList<K, V>::load_file(){
    _file_reader.open(STORE_FILE);
    cout << "load file-----------------------------" << endl;
    string line;
    string* key = new string();
    string* value = new string();
    while(getline(_file_reader, line)){
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty()){
            continue;
        }
        insert_element(*key, *value);
        cout << "key: " << *key << "value: " << *value << endl;
    }
    _file_reader.close();
}


//Get current SkipList size : 当前跳表的节点数
template<typename K, typename V>
int SkipList<K, V>::size(){
    return _element_count;
}


template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const string& str, string *key, string *value){
    if(!is_valid_string(str)){
        return ;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}


template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const string& str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == string::npos){
        return false;
    }
    return true;
}

//Delete element from skip list
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key){
    mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level+1));

    //start from highest level of skiplist
    for(int i = _skip_list_level+1;i>=0;--i){
        if(current->forward[i]!=nullptr && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    if(current!=nullptr && current->get_key()==key){
        //start from the lowest level and delete the current node of each level
        for(int i = 0;i<=_skip_list_level;++i){
            //if at level i, next node is not target node, break the loop
            if(update[i]->forward[i] != current){
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
        free(current);
        current = nullptr;
        //Remove levels which have no elements
        while(_skip_list_level>0 && _header->forward[_skip_list_level]==nullptr){
            _skip_list_level--;
        }
        cout << "Successfully deleted key "<< key << endl;
        _element_count --;
    }
    mtx.unlock();
    return ;
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V>
bool SkipList<K, V>::search_element(K key){
    cout << "search_element-----------------" << endl;
    Node<K, V>* current = _header;
    //start from the highest of SkipList
    for(int i = _skip_list_level;i>=0;--i){
        while(current->forward[i]!=nullptr && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
    }
    //reached level 0, and advance pointer to right node, which we search
    current = current->forward[0];
    // if current node have key equal to searched key, we get it
    if (current && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

template<typename K, typename V>
SkipList<K, V>::~SkipList(){
    if(_file_writer.is_open()){
        _file_writer.close();
    }
    if(_file_reader.is_open()){
        _file_reader.close();
    }
    delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){
    int k = 1;
    while(rand() % 2){
        k++;
    }
    k = (k<_max_level) ? k : _max_level;
    return k;
}


#endif