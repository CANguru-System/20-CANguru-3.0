/*
 * Demo of a more complex sorting using a compound key and a lambda expression.
 * This version uses shellSortKnuth(), but all the other sorting algorithms have
 * the exact same function signature.
 *
 */

#include <Arduino.h>
#include <List.hpp>

#define B

const uint8_t macLen = 6;

// Record of a person's name and score.
struct macType
{
  uint8_t m[macLen];
};

// List of names and their scores. This table is const so cannot be modified.
// The names and this table could be stored in PROGMEM, but that would
// significantly clutter up the code with type conversions which makes this
// example harder to understand.
const uint16_t ARRAY_SIZE = 8;
macType macs[ARRAY_SIZE] = {
#ifdef A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x07},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x06},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x05},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
#endif
#ifdef B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
    {0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF},
    {0x30, 0xAE, 0xA4, 0x89, 0x92, 0x71},
    {0xAE, 0x30, 0xA4, 0x89, 0x92, 0x71},
    {0xFF, 0x00, 0x00, 0x00, 0x01, 0x00},
    {0xA4, 0x89, 0x30, 0xAE, 0x92, 0x71},
    {0x92, 0x71, 0x30, 0xAE, 0xA4, 0x89},
    {0xA4, 0x89, 0x30, 0xAE, 0x71, 0x92},
#endif
};

struct ptrType
{
  macType *recordPtrs;
  uint64_t intValue;
};
List<ptrType> ptrsList;

// Print the records given an array of pointers to Record.
void printMACs()
{
  ptrType ptr;
  for (uint16_t i = 0; i < ptrsList.getSize(); i++)
  {
    ptr = ptrsList.get(i);
    const macType *mac = ptrsList.get(i).recordPtrs;
    char chs[50];
    sprintf(chs, "%d - %X:%X:%X:%X:%X:%X", i, mac->m[0], mac->m[1], mac->m[2], mac->m[3], mac->m[4], mac->m[5]);
    Serial.println(chs);
  }
}

// Fill the recordPtrs list with the pointers to macs[]. The original
// macs[] is immutable, so we need a helper array to perform the sorting.
void fillRecords()
{
  const uint8_t shift = 8;
  for (uint16_t i = 0; i < ARRAY_SIZE; i++)
  {
    uint64_t intValue = 0;
    for (uint8_t j = 0; j < macLen; j++)
    {
      intValue = (intValue << shift) + macs[i].m[j];
    }
    macType *recordPtrs = &macs[i];
    ptrType ptr;
    ptr.intValue = intValue;
    ptr.recordPtrs = recordPtrs;
    ptrsList.add(ptr);
  }
}

/**************************************************************************/
/*!
    @brief  Sorts a list of elements.
    @param  list        Array of elements to be sorted
    @param  n           Length of the array
*/
/**************************************************************************/
void InsertionSort()
{
  ptrType key;
  int32_t j = 0;

  for (int32_t i = 1; i < ptrsList.getSize(); i++)
  {
    key = ptrsList[i];
    j = i - 1;

    // Move elements of list[0..i-1], that are
    // greater than key, to one position ahead
    // of their current position
    while (j >= 0 && ptrsList[j].intValue > key.intValue)
    {
      //      list[j + 1] = list[j];
      ptrsList.remove(j + 1);
      ptrsList.addAtIndex(j + 1, ptrsList.get(j));
      j = j - 1;
    }
    // list[j + 1] = key;
    ptrsList.remove(j + 1);
    ptrsList.addAtIndex(j + 1, key);
  }
};

//-----------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);

  fillRecords();

  // Print original ordering.
  Serial.println(F("Unsorted"));
  Serial.println(F("--------"));
  printMACs();
  Serial.println();

  // Sort by score then name. Use a helper function for readability because the
  // predicate is more complex, but an inlined lambda expression would work
  // perfectly fine.
  Serial.println(F("Sorted"));
  Serial.println(F("------"));
  InsertionSort();
  printMACs();
  Serial.println();
}

void loop() {}