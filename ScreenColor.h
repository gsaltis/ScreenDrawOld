/*****************************************************************************
 * FILE NAME    : ScreenColor.h
 * DATE         : December 08 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _screencolor_h_
#define _screencolor_h_

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

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Type : ScreenColorType
 *****************************************************************************/
typedef enum ScreenColorType
{
 ScreenColorTypeNone,
 ScreenColorTypeString,
 ScreenColorTypeRGB

} ScreenColorType;

/*****************************************************************************!
 * Exported Type : ScreenColor
 *****************************************************************************/
struct _ScreenColor
{
  ScreenColorType                       type;
  union {
    string                              colorString;
    struct {
      int                               red;
      int                               green;
      int                               color;
      float                             alpha;
    } rgba;
  } color;
};
typedef struct _ScreenColor ScreenColor;

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/

#endif /* _screencolor_h_*/