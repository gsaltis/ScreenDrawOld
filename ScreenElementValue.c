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
#include "JSONOut.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
JSONOut*
ScreenElementValueStringToJSON
(ScreenElementValue* InValue);

JSONOut*
ScreenElementValueIntToJSON
(ScreenElementValue* InValue);

JSONOut*
ScreenElementValueDimensionToJSON
(ScreenElementValue* InValue);

JSONOut*
ScreenElementValueColorToJSON
(ScreenElementValue* InValue);

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
      FreeMemory(InValue->valueDimension.dimensionSuffix);
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
  int                                   valueInt;
  int                                   n;
  ScreenElementValue*                   value;
  string                                valueType;
  double                                valueFloat;
  string                                s2;
  string                                end;
  string                                s;
  ScreenElementValueDimensionType       type;
  
  value = ScreenElementValueCreate(ScreenElementValueTypeDimension, InTag);
  valueFloat = 0;
  valueType = StringCopy("px");
  s = InDimension;
  end = s;
  type = ScreenElementValueDimensionTypeInt;
  while (isdigit(*end)) {
    end++;
  }
  if ( *end == '.' ) {
    type = ScreenElementValueDimensionTypeFloat;
    end++;
    while (isdigit(*end)) {
      end++;
    }
  }
  
  n = end - s;
  s2 = StringNCopy(s, n);
  if ( type == ScreenElementValueDimensionTypeFloat ) {
    valueFloat = atof(s2);
  } else {
    valueInt = atoi(s2);
  }
   
  if (*end) {
    FreeMemory(valueType);
  }
  FreeMemory(s2);
  valueType = StringCopy(end);
  if ( type == ScreenElementValueDimensionTypeFloat ) {
    value->valueDimension.value.dimensionFloat = valueFloat;
  } else {
    value->valueDimension.value.dimensionInt = valueInt;
  }
  value->valueDimension.dimensionSuffix = valueType;
  value->valueDimension.dimensionType   = type;
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

/*****************************************************************************!
 * Function : ScreenElementValueToJSON
 *****************************************************************************/
JSONOut*
ScreenElementValueToJSON
(ScreenElementValue* InValue)
{
  if ( NULL == InValue ) {
    return NULL;
  }
  switch (InValue->type) {
    case ScreenElementValueTypeNone : {
      break;
    }
    case ScreenElementValueTypeString : {
      return ScreenElementValueStringToJSON(InValue);
    }
    case ScreenElementValueTypeInt : {
      return ScreenElementValueIntToJSON(InValue);
    }
    case ScreenElementValueTypeDimension : {
      return ScreenElementValueDimensionToJSON(InValue);
    }
    case ScreenElementValueTypeColor : {
      return ScreenElementValueColorToJSON(InValue);
    }
  }
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementValueStringToJSON
 *****************************************************************************/
JSONOut*
ScreenElementValueStringToJSON
(ScreenElementValue* InValue)
{
  if ( NULL == InValue ) {
    return NULL;
  }
  return JSONOutCreateString(InValue->tag, InValue->valueString);
}

/*****************************************************************************!
 * Function : ScreenElementValueIntToJSON
 *****************************************************************************/
JSONOut*
ScreenElementValueIntToJSON
(ScreenElementValue* InValue)
{
  if ( NULL == InValue ) {
    return NULL;
  }
  return JSONOutCreateInt(InValue->tag, InValue->valueInt);  
}

/*****************************************************************************!
 * Function : ScreenElementValueDimensionToJSON
 *****************************************************************************/
JSONOut*
ScreenElementValueDimensionToJSON
(ScreenElementValue* InValue)
{
  char                                  s[32];
  if ( NULL == InValue ) {
    return NULL;
  }
  if ( InValue->valueDimension.dimensionType == ScreenElementValueDimensionTypeFloat ) {
    sprintf(s, "%f%s", InValue->valueDimension.value.dimensionFloat, InValue->valueDimension.dimensionSuffix);
  } else {
    sprintf(s, "%d%s", InValue->valueDimension.value.dimensionInt, InValue->valueDimension.dimensionSuffix);
  }
  return JSONOutCreateString(InValue->tag, s);
}

/*****************************************************************************!
 * Function : ScreenElementValueColorToJSON
 *****************************************************************************/
JSONOut*
ScreenElementValueColorToJSON
(ScreenElementValue* InValue)
{
  char                                  s[32];
  if ( NULL == InValue ) {
    return NULL;
  }
  switch (InValue->valueColor.colorType) {
    case ScreenElementValueColorTypeNone : {
      s[0] = 0x00;
      break;
    }
    case ScreenElementValueColorTypeRGB : {
      sprintf(s, "rgb(%d, %d, %d)",
              InValue->valueColor.rgb.red,
              InValue->valueColor.rgb.green,
              InValue->valueColor.rgb.blue);              
      break;
    }
    case ScreenElementValueColorTypeString : {
      sprintf(s, "%s", InValue->valueColor.colorString);
      break;
    }
  }
  return JSONOutCreateString(InValue->tag, s);  
}

/*****************************************************************************!
 * Function : ScreenElementValueCreateColor
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCreateColor
(string InTag, string InValue)
{
  if ( NULL == InTag || NULL == InValue ) {
    return NULL;
  }

  if ( StringBeginsWith(InValue, "rgb") || StringBeginsWith(InValue, "RGB") ) {
    return ScreenElementValueCreateColorString(InTag, InValue);
  }
  return ScreenElementValueCreateColorString(InTag, InValue);
}
