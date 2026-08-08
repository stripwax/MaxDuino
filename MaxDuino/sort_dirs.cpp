#include <Arduino.h>
#include "sort_dirs.h"

bool operator< (dirEntry const &lhs, dirEntry const &rhs )
{
    if (strcasecmp( lhs.name, rhs.name) < 0){
        return true;
    }
    return false;
}

bool operator> (dirEntry const &lhs, dirEntry const &rhs )
{
    if (strcasecmp( lhs.name, rhs.name) > 0){
        return true;
    }
    return false;
}

bool operator>= (dirEntry const &lhs, dirEntry const &rhs )
{
    if (strcasecmp( lhs.name, rhs.name) >= 0){
        return true;
    }
    return false;
}

bool operator<= (dirEntry const &lhs, dirEntry const &rhs )
{
    if (strcasecmp( lhs.name, rhs.name) <= 0){
        return true;
    }
    return false;
}