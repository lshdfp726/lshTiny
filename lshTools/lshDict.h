#ifndef LSHDICT_H
#define LSHDICT_H

typedef struct Entry 
{
    char* key;
    char* value;
    struct Entry* next;
} Entry;

typedef struct Dictionary 
{
    Entry** table;
    size_t size;
    size_t capacity;
} Dictionary;

//成功返回0，失败返回-1
int init_dictionaryC(Dictionary* dict, unsigned int capacity);
//成功返回0，失败返回-1, 默认字典容量capacity = 100
int init_dictionary(Dictionary* dict);
//成功返回0，失败返回-1, 内部对key、value进行一份拷贝
int add_entry(Dictionary *dict, const char* key, const char* value);
const char* find_entry(Dictionary *dict, const char *key);
void remove_entry(Dictionary* dict, const char* key);
void free_dictionary(Dictionary *dict);

#endif