/*****************************************************************************
 * FILE NAME    : ScreenElementValue.c
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
#include <ctype.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "ScreenElementValue.h"
#include "GeneralUtilities/MemoryManager.h"
#include "GeneralUtilities/String.h"

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
 * Function : ScreenElementValueCreate
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreate
(ScreenElementValueType InType, string InTag)
{
  ScreenElementValue*                   value;

  if ( NULL == InTag ) {
    return NULL;
  }
  
  value = (ScreenElementValue*)GetMemory(sizeof(ScreenElementValue));
  memset(value, 0x00, sizeof(ScreenElementValue));
  value->type = InType;
  value->tag = StringCopy(InTag);
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueDestroy
 *****************************************************************************/
void
ScreenElementValueDestroy
(ScreenElementValue* InValue)
{
  if ( NULL == InValue ) {
    return;
  }
  switch (InValue->type) {
    case ScreenElementValueTypeNone : {
      break;
    }
    
    case ScreenElementValueTypeString : {
      FreeMemory(InValue->valueString);
      break;
    }
    
    case ScreenElementValueTypeInt : {
      break;
    }
    
    case ScreenElementValueTypeDimension : {
      FreeMemory(InValue->valueDimension.dimensionType);
      break;
    }
    
    case ScreenElementValueTypeColor : {
      if ( InValue->valueColor.colorType == ScreenElementValueColorTypeString ) {
        FreeMemory(InValue->valueColor.colorString);
      }
      break;
    }
  }
  FreeMemory(InValue);
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateInt
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateInt
(string InTag, int InValue)
{
  ScreenElementValue*                   value;

  value = ScreenElementValueCreate(ScreenElementValueTypeInt, InTag);
  value->valueInt = InValue;
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateString
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateString
(string InTag, string InValue)
{
  ScreenElementValue*                   value;
  value = ScreenElementValueCreate(ScreenElementValueTypeInt, InTag);
  value->valueString = StringCopy(InValue);
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateDimension
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateDimension
(string InTag, string InDimension)
{
  int                                   n;
  ScreenElementValue*                   value;
  string                                valueType;
  double                                valueFloat;
  string                                s2;
  string                                end;
  string                                s;

  value = ScreenElementValueCreate(ScreenElementValueTypeDimension, InTag);
  valueFloat = 0;
  valueType = StringCopy("px");
  s = InDimension;
  end = s;
  while (isdigit(*end)) {
    end++;
  }
  if ( *end == '.' ) {
    end++;
    while (isdigit(*end)) {
      end++;
    }
  }
  
  n = end - s;
  s2 = StringNCopy(s, n);
  valueFloat = atof(s2);
  if (*end) {
    FreeMemory(valueType);
  }
  FreeMemory(s2);
  valueType = StringCopy(end);
  value->valueDimension.dimensionValue = valueFloat;
  value->valueDimension.dimensionType = valueType;
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateColorString
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateColorString
(string InTag, string InValue)
{
  ScreenElementValue*                   value;
  value = ScreenElementValueCreate(ScreenElementValueTypeColor, InTag);
  value->valueColor.colorType = ScreenElementValueColorTypeString;
  value->valueColor.colorString = StringCopy(InValue);
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateColorRGB
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateColorRGB
(string InTag, int InRed, int InGreen, int InBlue)
{
  ScreenElementValue*                   value;
  value = ScreenElementValueCreate(ScreenElementValueTypeColor, InTag);
  value->valueColor.colorType = ScreenElementValueColorTypeRGB;
  value->valueColor.rgb.red = InRed;
  value->valueColor.rgb.green = InGreen;
  value->valueColor.rgb.blue = InBlue;
  return value;  
}
