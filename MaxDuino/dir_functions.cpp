#include <Arduino.h>
#include "file_utils.h"
#include "dir_functions.h"

extern bool dirEmpty;                      //flag if directory is completely empty

#ifdef SORT_DIRS
#include <DoubleLinkedList.h>

bool putSubdirsFirst;
DoubleLinkedList<dirEntry> dirEntries;




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

// returns pointer to filename for direntry at given position 
char* sdGetFileName( uint16_t pos )
{
    return ((dirEntries.get(pos))->name);
}
#endif //SORT_DIRS

// inserts directory entry into the list, keeping the list sorted by filename
void sdInsertSorted( char* name, uint16_t index, bool dir )
{
#ifdef SORT_DIRS
    dirEntry new_entry;

    strncpy( new_entry.name, name, 255 );
    new_entry.index = index;
    new_entry.isdir = dir;

    for (int f = 0; f < dirEntries.size(); f++)
    {
        if (dirEntries.getElement(f) > new_entry)
        {
        dirEntries.insert(new_entry, f);
        return;
        }
    }
    dirEntries.append(new_entry);
#endif
}





// returns sdFat index of directory 
 // has to be retreived from dirEntries  when dirs are sorted
uint16_t sdGetCurrentFileIndex( uint16_t index ) 
{
  #ifdef SORT_DIRS
      return dirEntries.get(currentFile)->index;
  #else
      return (index);
  #endif
}


 // returns file position based on sdFat index 
 // has to be a position in dirEntries when dirs are sorted or entry argumnent index otherwise 
uint16_t sdGetDirectoryPositionByIndex( uint16_t index ) 
{
#ifdef SORT_DIRS
  for (int f = 0; f < dirEntries.size(); f++) {
    if (dirEntries.get(f)->index == index ) {
      return (f);
    }
  }
return(0); //should never happen!
  
#else // no SORT_DIRS 
  return ( index ) ; // we're using indexes already if not sorting
#endif
}

// returns index of the next file in sdfat or position if dirs are to be sorted
uint16_t sdGetNextFile( uint16_t index ) 
{
    index ++;
  
#ifdef SORT_DIRS
  if (index >= dirEntries.size())
#else 
  if ( index > maxFile) 
#endif 
  { // return to zero if went beyond last file
    index = 0;
  }
  return index;
}

// returns index of the previous file in sdfat or position if dirs are to be sorted
uint16_t sdGetPreviousFile( uint16_t index )
{
#ifdef SORT_DIRS

  if ( index  == 0)
    return(  dirEntries.size() - 1 );

  return ( index - 1 );
  
#else // no SORT_DIRS
  // Rather than going "backwards", we actually look forward from entry 0 ,
  // because the SdFat can efficiently find "next entries"
  // much more easily than "previous entries"
  // So: first, load the zeroth entry.  SdFat will give us either this, or the next valid one > 0 .
  // If we're (un)lucky, the one it finds is actually our original currentFile
  // meaning there is no file prior to currentfile, in which case 'up' should just wrap to the last file
  
  uint16_t currentPositionNow = maxFile; // preload what happens in the wrap case
  uint16_t tryFindPrevFile;
  do
  {
    entry.close();
    tryFindPrevFile = currentPositionNow;
    entry.openNext(currentDir, O_RDONLY);
    // openNext will always open a valid file. curPosition is now updated to a valid file index
    // (so tryFindPrevFile is the index of the file preceding it; or maxFile if there was no file preceding currentFile
    currentPositionNow = currentDir->curPosition()/32-1;
  }
  while(currentPositionNow < index );

  return ( tryFindPrevFile );
#endif
}

// clears and initializes  directory variables when reading 
void sdInitDirectory() 
{
#ifdef SORT_DIRS
  dirEntries.clear();
#endif
  currentDir->rewind();
  maxFile = 0;
  dirEmpty = true;
}

// wrapper to return size of dir entries the index
uint16_t sdMaxFileIndex( uint16_t index )
{
    #ifdef SORT_DIRS
          return dirEntries.size();
    #else
        return index;
    #endif
}