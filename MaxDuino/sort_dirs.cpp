#include <Arduino.h>
#include "file_utils.h"

#ifdef SORT_DIRS
#include "sort_dirs.h"

bool operator< (dirEntry const &lhs, dirEntry const &rhs )
{
    extern bool putSubdirsFirst;
    
    if ( putSubdirsFirst ) {
        if ( lhs.isdir != rhs.isdir ) {
            if ( lhs.isdir > rhs.isdir )  {
                return true;
            } else {
                return false;
            }
        }
    }

    if (strcasecmp( lhs.name, rhs.name) < 0){
        return true;
    }
    return false;
}

bool operator> (dirEntry const &lhs, dirEntry const &rhs )
{
    extern bool putSubdirsFirst;

    if ( putSubdirsFirst ) {
        if ( lhs.isdir != rhs.isdir ) {
            if ( lhs.isdir < rhs.isdir )  {
                    return true;
            } else {
                return false;
            }
        }
    }

    if (strcasecmp( lhs.name, rhs.name) > 0){
        return true;
    }
    return false;
}

bool operator>= (dirEntry const &lhs, dirEntry const &rhs )
{
    extern bool putSubdirsFirst;

    if ( putSubdirsFirst ) {
        if ( lhs.isdir != rhs.isdir ) {
            if ( lhs.isdir > rhs.isdir )  {
                    return true;
            } else {
                return false;
            }
        }
    }

    if (strcasecmp( lhs.name, rhs.name) >= 0){
        return true;
    }
    return false;
}

bool operator<= (dirEntry const &lhs, dirEntry const &rhs )
{
    extern bool putSubdirsFirst;

    if ( putSubdirsFirst ) {
        if ( lhs.isdir != rhs.isdir ) {
            if ( lhs.isdir < rhs.isdir )  {
                    return true;
            } else {
                return false;
            }
        }
    }

    if (strcasecmp( lhs.name, rhs.name) <= 0){
        return true;
    }
    return false;
}
#endif