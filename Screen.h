/*****************************************************************************
 * FILE NAME    : Screen.h
 * DATE         : December 03 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _screen_h_
#define _screen_h_

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
#include "ScreenElement.h"
#include "JSONOut.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Type : Screen
 *****************************************************************************/
struct _Screen
{
  string                                name;
  ScreenElement**                       elements;
  int                                   elementsCount;
};
typedef struct _Screen Screen;

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
ScreenElement*
ScreenFindElementByName
(Screen* InScreen, string InName);

void
ScreenAddElement
(Screen* InScreen, ScreenElement* InElement);

void
ScreenDestroy
(Screen* InScreen);

Screen*
ScreenCreate
(string InName);

#endif /* _screen_h_*/
