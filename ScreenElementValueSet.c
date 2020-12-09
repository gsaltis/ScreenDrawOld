/*****************************************************************************
 * FILE NAME    : ScreenElementValueSet.c
 * DATE         : December 08 2020
 * PROJECT      : 
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/

/*****************************************************************************!
 * Global Headers
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "ScreenElementValueSet.h"
#include "GeneralUtilities/MemoryManager.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/

/*****************************************************************************!
 * Local Data
 *****************************************************************************/


/*****************************************************************************!
 * Function : ScreenElementValueSetCreate
 *****************************************************************************/
ScreenElementValueSet*
ScreenElementValueSetCreate
(string InName)
{
  ScreenElementValueSet*                set;

  if ( NULL == InName ) {
    return NULL;
  }
  
  set = (ScreenElementValueSet*)GetMemory(sizeof(ScreenElementValueSet));
  memset(set, 0x00, sizeof(ScreenElementValueSet));
  set->name = StringCopy(InName);
  set->values = ScreenElementValueListCreate();
  return set;
}


/*****************************************************************************!
 * Function : ScreenElementValueSetDestroy
 *****************************************************************************/
void
ScreenElementValueSetDestroy
(ScreenElementValueSet* InSet)
{
  if ( NULL == InSet ) {
    return;
  }
  FreeMemory(InSet->name);
  ScreenElementValueListDestroy(InSet->values);
  FreeMemory(InSet->values);
}

/*****************************************************************************!
 * Function : ScreenElementValueSetListCreate
 *****************************************************************************/
ScreenElementValueSetList*
ScreenElementValueSetListCreate
()
{
  ScreenElementValueSetList*            sets;

  sets = (ScreenElementValueSetList*)GetMemory(sizeof(ScreenElementValueSetList*));
  memset(sets, 0x00, sizeof(ScreenElementValueSetList));
  return sets;
}


/*****************************************************************************!
 * Function : ScreenElementValueSetListDestroy
 *****************************************************************************/
void
ScreenElementValueSetListDestroy
(ScreenElementValueSetList* InSets)
{
  int                                   i;
  if ( NULL == InSets ) {
    return;
  }
  for (i = 0; i < InSets->setsCount; i++) {
    ScreenElementValueSetDestroy(InSets->sets[i]);
  }
  if ( InSets->setsCount ) {
    FreeMemory(InSets->sets);
  }
  FreeMemory(InSets);
}

/*****************************************************************************!
 * Function : ScreenElementValueSetListAppend
 *****************************************************************************/
void
ScreenElementValueSetListAppend
(ScreenElementValueSetList* InSets, ScreenElementValueSet* InSet)
{
  int                                   i;
  ScreenElementValueSet**               sets;
  int                                   n;
  if ( NULL == InSets || NULL == InSet ) {
    return;
  }

  if ( InSets->sets == NULL ) {
    InSets->sets = (ScreenElementValueSet**)GetMemory(sizeof(ScreenElementValueSet*));
    InSets->sets[0] = InSet;
    InSets->setsCount = 1;
    return;
  }

  n = InSets->setsCount + 1;
  sets = (ScreenElementValueSet**)GetMemory(sizeof(ScreenElementValueSet*) * n);
  for (i = 0; i < InSets->setsCount; i++) {
    sets[i] = InSets->sets[i];
  }
  sets[InSets->setsCount] = InSet;
  FreeMemory(InSets->sets);
  InSets->sets = sets;
  InSets->setsCount = n;
}


/*****************************************************************************!
 * Function : ScreenElementValueSetListFindByName
 *****************************************************************************/
ScreenElementValueSet*
ScreenElementValueSetListFindByName
(ScreenElementValueSetList* InList, string InName)
{
  int                                   i;
  if ( NULL == InList || NULL == InName ) {
    return NULL;
  }
  if ( NULL == InList->sets ) {
    return NULL;
  }
  for (i = 0; i < InList->setsCount; i++) {
    if ( StringEqual(InList->sets[i]->name, InName) ) {
      return InList->sets[i];
    }
  }
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetFindByName
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueSetFindByName
(ScreenElementValueSet* InSet, string InName)
{
  if ( NULL == InSet || NULL == InName ) {
    return NULL;
  }

  if ( NULL == InSet->values ) {
    return NULL;
  }

  return ScreenElementValueListFindByName(InSet->values, InName);
}

/*****************************************************************************!
 * Function : ScreenElementValueSetAppend
 *****************************************************************************/
void
ScreenElementValueSetAppend
(ScreenElementValueSet* InSet, ScreenElementValue* InValue)
{
  if ( NULL == InSet || NULL == InValue ) {
    return;
  }

  ScreenElementValueListAppend(InSet->values, InValue);
}
