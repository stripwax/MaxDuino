#include <Arduino.h>
#include "sort_dirs.h"

extern bool putSubdirsFirst;

bool operator< (dirEntry const &lhs, dirEntry const &rhs )
{
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