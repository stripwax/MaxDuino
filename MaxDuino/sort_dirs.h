#ifndef SORT_DIRS_H_INCLUDED

#define SORT_DIRS_H_INCLUDED

#ifdef SORT_DIRS

typedef struct
{
    char name[256];
    bool isdir;
    uint16_t index;
} dirEntry;

bool operator< (dirEntry const &lhs, dirEntry const &rhs );
bool operator> (dirEntry const &lhs, dirEntry const &rhs );
bool operator>= (dirEntry const &lhs, dirEntry const &rhs );
bool operator<= (dirEntry const &lhs, dirEntry const &rhs );

#endif

#endif //SORT_DIRS_H_INCLUDED