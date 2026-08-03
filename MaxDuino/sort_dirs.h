#ifndef SORT_DIRS_H_INCLUDED

#define SORT_DIRS_H_INCLUDED
typedef struct
{
    char name[256];
    uint16_t index;
} dirEntry;

bool operator< (dirEntry const &lhs, dirEntry const &rhs );
bool operator> (dirEntry const &lhs, dirEntry const &rhs );
bool operator>= (dirEntry const &lhs, dirEntry const &rhs );
bool operator<= (dirEntry const &lhs, dirEntry const &rhs );

#endif //SORT_DIRS_H_INCLUDED