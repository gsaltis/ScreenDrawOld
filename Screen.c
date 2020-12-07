/*****************************************************************************
 * FILE NAME    : Screen.c
 * DATE         : December 03 2020
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
#include "Screen.h"
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
 * Function : ScreenCreate
 *****************************************************************************/
Screen*
ScreenCreate
(string InName)
{
  Screen*                               screen;
  if ( NULL == InName ) {
    return NULL;
  }

  screen = (Screen*)GetMemory(sizeof(Screen));
  memset(screen, 0x00, sizeof(Screen));
  screen->name = StringCopy(InName);
  return screen;
}

/*****************************************************************************!
 * Function : ScreenDestroy
 *****************************************************************************/
void
ScreenDestroy
(Screen* InScreen)
{
  int                                   i;
  if ( NULL == InScreen ) {
    return;
  }

  for (i = 0; i < InScreen->elementsCount; i++) {
    ScreenElementDestroy(InScreen->elements[i]);
  }
  FreeMemory(InScreen->name);
  FreeMemory(InScreen);
}

/*****************************************************************************!
 * Function : ScreenAddElement
 *****************************************************************************/
void
ScreenAddElement
(Screen* InScreen, ScreenElement* InElement)
{
  ScreenElement**                       elements;
  int                                   i;
  int                                   n;
  if ( NULL == InScreen || NULL == InElement ) {
    return;
  }

  n = InScreen->elementsCount + 1;

  elements = (ScreenElement**)GetMemory(sizeof(ScreenElement*) * n);
  for ( i = 0; i < InScreen->elementsCount ; i++ ) {
    elements[i] = InScreen->elements[i];
  }
  if ( InScreen->elements ) {
    FreeMemory(InScreen->elements);
  }
  elements[InScreen->elementsCount]  = InElement;
  InScreen->elements = elements;
  InScreen->elementsCount = n;
}

/*****************************************************************************!
 * Function : ScreenFindElementByName
 *****************************************************************************/
ScreenElement*
ScreenFindElementByName
(Screen* InScreen, string InName)
{
  int                                   i;
  if ( NULL == InScreen || NULL == InName ) {
    return NULL;
  }

  for (i = 0; i < InScreen->elementsCount; i++) {
    if ( StringEqual(InScreen->elements[i]->name, InName) ) {
      return InScreen->elements[i];
    }
  }
  return NULL;
}
