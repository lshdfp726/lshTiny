#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lshDict.h"

#define INIT_TABLE_SIZE 100
#define EXPAND_FACTOR_THRESHOLD 0.6



unsigned int hash(const char* key) 
{
    unsigned int hash = 0;
    while(*key) 
    {
        hash = (hash * 31) + *key++;
    }
    return hash;
}

//成功返回0，失败返回-1
int init_dictionaryC(Dictionary* dict, unsigned int capacity)
{
    dict->size = 0;
    dict->capacity = capacity;
    dict->table = (Entry **)malloc(sizeof(Entry *) * dict->capacity);
    if (!dict->table) {
        fprintf(stderr, "init_dictionary: Memory allocation failed\n");
        return -1;
    }

    for (size_t i = 0; i < dict->capacity; i++) {
        dict->table[i] = NULL;
    }

    return 0;
}

//成功返回0，失败返回-1, 默认字典容量capacity = 100
int init_dictionary(Dictionary* dict)
{
    return init_dictionaryC(dict, INIT_TABLE_SIZE);
}

//成功返回0，失败返回-1
int expand_dictory(Dictionary* dict) 
{
    size_t new_capacity = dict->capacity * 2;
    Entry **new_table = (Entry **)malloc(sizeof(Entry *) * new_capacity);
    if (!new_table) 
    {
        fprintf(stderr, "expand_dictory: Memory allocation failed\n");
        return -1;
    }

    for (size_t i = 0; i < new_capacity; i++)
    {
        new_table[i] = NULL;
    }

    for (size_t i = 0; i < dict->capacity; i++)
    {
        Entry* current = dict->table[i];
        while (current != NULL)
        {
            Entry *next = current->next;
            unsigned int index = hash(current->key) % new_capacity;
            current->next = new_table[index]; //new_table 链表原来顺序 2->3->4 ,  current假设需要查到index = 0的位置,那么新的关系应该是 cur->2->3->4 
            new_table[index] = current;
            current = next;
        }
    }
    
    free(dict->table);
    dict->table = new_table;
    dict->capacity = new_capacity;   
    return 0; 
}

//成功返回0，失败返回-1
int add_entry(Dictionary *dict, const char* key, const char* value) 
{
    if ((double)dict->size / dict->capacity > EXPAND_FACTOR_THRESHOLD) 
    {
        expand_dictory(dict);
    }
    unsigned int index = hash(key) % dict->capacity;
    Entry* new_entry = (Entry *)malloc(sizeof(Entry));
    if (!new_entry) 
    {
        fprintf(stderr,"add_entry: Memory allocation failed\n");
        return -1;
    }
    new_entry->key = strdup(key); //copy 一份key 的副本，并返指向这个副本的指针
    new_entry->value = strdup(value);
    new_entry->next = dict->table[index];
    dict->table[index] = new_entry;
    dict->size ++;
    return 0;
}

const char* find_entry(Dictionary *dict, const char *key) 
{
    unsigned int index = hash(key) % dict->capacity;
    Entry* current = dict->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0) return current->value;
        current = current->next;
    }
    return NULL;
}

void remove_entry(Dictionary* dict, const char* key) 
{
    unsigned int index = hash(key) % dict->capacity;
    Entry* current = dict->table[index];
    Entry* prev = NULL;
    while (current != NULL)
    {
        if (strcmp(current ->key, key) == 0) 
        {
            if (prev == NULL) //旧关系: current->next  新关系: next
            {
                dict->table[index] = current->next;
            } 
            else //旧关系： prev->current->next  新关系: prev->next
            {
                prev->next = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            dict->size--;
            return ;
        } 
        prev = current;
        current = current->next;
    }
}

void free_dictionary(Dictionary *dict) 
{
    for (size_t i = 0; i < dict->capacity; i++)
    {
        Entry* current = dict->table[i];
        while (current != NULL)
        {
            Entry* next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
        dict->table[i] = NULL;
    }
    free(dict->table);
    dict->size = 0;
    dict->capacity = 0;
}

// int main() {
//     Dictionary dict;
//     init_dictionaryC(&dict, 3);
//     add_entry(&dict, "key1", "value1");
//     add_entry(&dict, "key2", "value2");
//     add_entry(&dict, "key3", "value3");


//     const char* value1 = find_entry(&dict, "key1");
//     const char* value2 = find_entry(&dict, "key2");

//     printf("Value for key1: %s\n", value1);
//     printf("Value for key2: %s\n", value2);

//     remove_entry(&dict, "key1");

//     const char *value1_after_remove = find_entry(&dict, "key1");
//     if (value1_after_remove == NULL) 
//     {
//         printf("Key 'key1' not found after removal\n");
//     }
//     free_dictionary(&dict);
// }