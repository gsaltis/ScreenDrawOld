/*****************************************************************************
 * FILE NAME    : ScreenColor.c
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
#include <regex.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "ScreenColor.h"
#include "GeneralUtilities/MemoryManager.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
ScreenColor*
ScreenColorCreateRGBA
(string InString);

ScreenColor*
ScreenColorCreateString
(string InString);

/*****************************************************************************!
 * Local Data
 *****************************************************************************/



/*****************************************************************************!
 * Function : ScreenColorCreate
 *****************************************************************************/
ScreenColor*
ScreenColorCreate
(string InString)
{
  if ( NULL == InString ) {
    return NULL;
  }
  if ( StringBeginsWith(InString, "#") ) {
    return ScreenColorCreateString(InString);
  }
  if ( StringBeginsWith(InString, "rgb") ) {
    return ScreenColorCreateRGBA(InString);
  }
  return ScreenColorCreateString(InString);
}

/*****************************************************************************!
 * Function : ScreenColorCreateString
 *****************************************************************************/
ScreenColor*
ScreenColorCreateString
(string InString)
{
  ScreenColor*                          color;
  
  color = (ScreenColor*)GetMemory(sizeof(ScreenColor));
  memset(color, 0x00, sizeof(ScreenColor));
  color->type = ScreenColorTypeString;
  color->color.colorString = StringCopy(InString);
  return color;
}

/*****************************************************************************!
 * Function : ScreenColorCreateRGBA
 *****************************************************************************/
ScreenColor*
ScreenColorCreateRGBA
(string InString)
{
  ScreenColor*                          color;
  color = (ScreenColor*)GetMemory(sizeof(ScreenColor));
  memset(color, 0x00, sizeof(ScreenColor));
  color->type = ScreenColorTypeString;
  color->color.colorString = StringCopy(InString);
  return color;
}
