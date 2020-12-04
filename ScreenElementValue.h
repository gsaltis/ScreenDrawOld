/*****************************************************************************
 * FILE NAME    : ScreenElementValue.h
 * DATE         : December 03 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _screenelementvalue_h_
#define _screenelementvalue_h_

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
 * Exported Type : ScreenElementValueColorType
 *****************************************************************************/
typedef enum ScreenElementValueColorType
{
 ScreenElementValueColorTypeNone,
 ScreenElementValueColorTypeString,
 ScreenElementValueColorTypeRGB
} ScreenElementValueColorType;

/*****************************************************************************!
 * Exported Type : ScreenElementValueType
 *****************************************************************************/
typedef enum ScreenElementValueType
{
 ScreenElementValueTypeNone,
 ScreenElementValueTypeString,
 ScreenElementValueTypeInt,
 ScreenElementValueTypeDimension,
 ScreenElementValueTypeColor
} ScreenElementValueType;

/*****************************************************************************!
 * Exported Type : ScreenElementValue
 *****************************************************************************/
struct _ScreenElementValue
{
  string                                tag;
  ScreenElementValueType                type;
  union
  {
    uint32_t                            valueInt;
    string                              valueString;
    struct {
      float                             dimensionValue;
      string                            dimensionType;
    }                                   valueDimension;
    union {
      ScreenElementValueColorType       colorType;
      string                            colorString;
      struct {
        uint32_t                        red;
        uint32_t                        green;
        uint32_t                        blue;
      } rgb;
    }                                   valueColor;
  };  
};
typedef struct _ScreenElementValue ScreenElementValue;

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateColorRGB
(string InTag, int InRed, int InGreen, int InBlue);

ScreenElementValue*
ScreenElementValueCreateColorString
(string InTag, string InValue);

ScreenElementValue*
ScreenElementValueCreateDimension
(string InTag, string InDimension);

ScreenElementValue*
ScreenElementValueCreateString
(string InTag, string InValue);

ScreenElementValue*
ScreenElementValueCreateInt
(string InTag, int InValue);

void
ScreenElementValueDestroy
(ScreenElementValue* InValue);

ScreenElementValue*
ScreenElementValueCreate
(ScreenElementValueType InType, string InTag);

#endif /* _screenelementvalue_h_*/
