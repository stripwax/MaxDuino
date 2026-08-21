#ifndef SORT_DIRS_H_INCLUDED

#define SORT_DIRS_H_INCLUDED

// sd directory helper functions
uint16_t sdGetCurrentFileIndex( uint16_t index );
uint16_t sdGetDirectoryPositionByIndex( uint16_t index );
uint16_t sdGetNextFile( uint16_t index );
uint16_t sdGetPreviousFile( uint16_t index );
void sdInitDirectory();
uint16_t sdMaxFileIndex( uint16_t index );
void sdInsertSorted( char* name, uint16_t index, bool dir );



extern uint16_t currentFile;

#ifdef SORT_DIRS

#include <DoubleLinkedList.h>

typedef struct
{
    char name[filenameLength+1];
    bool isdir;
    uint16_t index;
} dirEntry;

char* sdGetFileName( uint16_t pos );
//void sdInsertSorted(DoubleLinkedList<dirEntry> *list, dirEntry *newentry);


extern bool putSubdirsFirst;
extern DoubleLinkedList<dirEntry> dirEntries;

bool operator< (dirEntry const &lhs, dirEntry const &rhs );
bool operator> (dirEntry const &lhs, dirEntry const &rhs );
bool operator>= (dirEntry const &lhs, dirEntry const &rhs );
bool operator<= (dirEntry const &lhs, dirEntry const &rhs );


#endif //SORT_DIRS

#endif //SORT_DIRS_H_INCLUDED