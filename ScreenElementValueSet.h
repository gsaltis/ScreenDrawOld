/*****************************************************************************
 * FILE NAME    : ScreenElementValueSet.h
 * DATE         : December 08 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _screenelementvalueset_h_
#define _screenelementvalueset_h_

/*****************************************************************************!
 * Global Headers
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "GeneralUtilities/String.h"
#include "ScreenElementValue.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Type : ScreenElementValueSet
 *****************************************************************************/
struct _ScreenElementValueSet
{
  string                                name;
  ScreenElementValueList*               values;
};
typedef struct _ScreenElementValueSet ScreenElementValueSet;

/*****************************************************************************!
 * Exported Type : ScreenElementValueSetList
 *****************************************************************************/
struct _ScreenElementValueSetList
{
  ScreenElementValueSet**               sets;
  int                                   setsCount;
};
typedef struct _ScreenElementValueSetList ScreenElementValueSetList;

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
void
ScreenElementValueSetAppend
(ScreenElementValueSet* InSet, ScreenElementValue* InValue);

ScreenElementValue*
ScreenElementValueSetFindByName
(ScreenElementValueSet* InSet, string InName);

ScreenElementValueSet*
ScreenElementValueSetListFindByName
(ScreenElementValueSetList* InList, string InName);

void
ScreenElementValueSetListAppend
(ScreenElementValueSetList* InSets, ScreenElementValueSet* InSet);

void
ScreenElementValueSetListDestroy
(ScreenElementValueSetList* InSets);

ScreenElementValueSetList*
ScreenElementValueSetListCreate
();

void
ScreenElementValueSetDestroy
(ScreenElementValueSet* InSet);

ScreenElementValueSet*
ScreenElementValueSetCreate
(string InName);

#endif /* _screenelementvalueset_h_*/
