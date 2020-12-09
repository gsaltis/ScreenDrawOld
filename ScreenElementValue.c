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
#include "GeneralUtilities/MemoryManager.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopyDimension
(ScreenElementValue* InValue);

ScreenElementValue*
ScreenElementValueCopyColor
(ScreenElementValue* InValue);

ScreenElementValue*
ScreenElementValueCopyInt
(ScreenElementValue* InValue);

ScreenElementValue*
ScreenElementValueCopyString
(ScreenElementValue* InValue);

bool
ScreenElementValueSetString
(ScreenElementValue* InValue, string InString);

bool
ScreenElementValueSetInt
(ScreenElementValue* InValue, string InString);

bool
ScreenElementValueSetColor
(ScreenElementValue* InValue, string InString);

bool
ScreenElementValueSetDimension
(ScreenElementValue* InValue, string InString);

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
  value = ScreenElementValueCreate(ScreenElementValueTypeString, InTag);
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

/*****************************************************************************!
 * Function : ScreenElementValueListCreate
 *****************************************************************************/
ScreenElementValueList*
ScreenElementValueListCreate
()
{
  ScreenElementValueList*               valueList;

  valueList = (ScreenElementValueList*)GetMemory(sizeof(ScreenElementValueList));
  memset(valueList, 0x00, sizeof(ScreenElementValueList));
  return valueList;
}

/*****************************************************************************!
 * Function : ScreenElementValueListDestroy
 *****************************************************************************/
void
ScreenElementValueListDestroy
(ScreenElementValueList* InValueList)
{
  int                                   i;
  if ( NULL == InValueList ) {
    return;
  }

  if ( InValueList->valuesCount > 0 ) {
    for (i = 0; i < InValueList->valuesCount; i++) {
      ScreenElementValueDestroy(InValueList->values[i]);
    }
    FreeMemory(InValueList->values);
  }
  FreeMemory(InValueList);
}

/*****************************************************************************!
 * Function : ScreenElementValueListAppend
 *****************************************************************************/
void
ScreenElementValueListAppend
(ScreenElementValueList* InValueList, ScreenElementValue* InValue)
{
  int                                   i;
  ScreenElementValue**                  values;
  int                                   n;
  if ( NULL == InValueList || NULL == InValue ) {
    return;
  }
  n = InValueList->valuesCount;

  values = (ScreenElementValue**)GetMemory(sizeof(ScreenElementValue*) * (n + 1));
  for (i = 0; i < n; i++) {
    values[i] = InValueList->values[i];
  }
  if ( n > 0 ) {
    FreeMemory(InValueList->values);
  }
  values[n] = InValue;
  InValueList->values = values;
}

/*****************************************************************************!
 * Function : ScreenElementValueListRemoveByIndex
 *****************************************************************************/
void
ScreenElementValueListRemoveByIndex
(ScreenElementValueList* InValues, int InIndex, bool InDestroyValue)
{
  int                                   m;
  int                                   j;
  int                                   i;
  ScreenElementValue**                  values;
  int                                   n;
  if ( NULL == InValues ) {
    return;
  }
  if ( InValues->valuesCount == 0 ) {
    return;
  }

  if ( InIndex > InValues->valuesCount ) {
    return;
  }

  // We have a list of only one item and we are removing it
  if ( InValues->valuesCount == 1 && InIndex == 0 ) {
    FreeMemory(InValues->values);
    InValues->values = NULL;
    InValues->valuesCount = 0;
    return;
  }

  // This is the last item on the list on a list of more than one item
  if ( InValues->valuesCount == InIndex + 1 ) {
    n = InValues->valuesCount - 1;
    values = (ScreenElementValue**)GetMemory(sizeof(ScreenElementValue*) * n);
    for (i = 0; i < n; i++) {
      values[i] = InValues->values[i];
    }
    FreeMemory(InValues->values);
    InValues->values = values;
    InValues->valuesCount = n;
    return;
  }

  // Remove the first item on the list on a list of more than one item
  if ( InIndex == 0 ) {
    n = InValues->valuesCount - 1;
    values = (ScreenElementValue**)GetMemory(sizeof(ScreenElementValue*) * n);
    for (i = 1; i <= n; i++) {
      values[i-1] = InValues->values[i];
    }
    FreeMemory(InValues->values);
    InValues->values = values;
    InValues->valuesCount = n;
    return;
  }

  // Remove an item somewhere in the middle of the list
  n = InValues->valuesCount;
  m = n - 1;
  values = (ScreenElementValue**)GetMemory(sizeof(ScreenElementValue*) * m);
  j = 0;
  for (i = 1; i <= n; i++) {
    if ( i != InIndex ) {
      values[j] = InValues->values[i];
      j++;
    }
  }
  FreeMemory(InValues->values);
  InValues->values = values;
  InValues->valuesCount = m;
}

/*****************************************************************************!
 * Function : ScreenElementValueListRemoveByValue
 *****************************************************************************/
void
ScreenElementValueListRemoveByValue
(ScreenElementValueList* InValues, ScreenElementValue* InValue, bool InDestroyValue)
{
  int                                   i;
  if ( NULL == InValues || NULL == InValue ) {
    return;
  }

  for (i = 0; i < InValues->valuesCount; i++) {
    if ( InValues->values[i] == InValue ) {
      break;
    }
  }
  if ( i == InValues->valuesCount ) {
    return;
  }

  ScreenElementValueListRemoveByIndex(InValues, i, InDestroyValue);
}

/*****************************************************************************!
 * Function : ScreenElementValueListFindByName
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueListFindByName
(ScreenElementValueList* InValues, string InName)
{
  int                                   i;
  if ( NULL == InValues || NULL == InName ) {
    return NULL;
  }

  if ( NULL == InValues->values ) {
    return NULL;
  }

  for (i = 0; i < InValues->valuesCount; i++) {
    if ( StringEqual(InValues->values[i]->tag, InName) ) {
      return InValues->values[i];
    }
  }
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetValue
 *****************************************************************************/
bool
ScreenElementValueSetValue
(ScreenElementValue* InElement, string InValue)
{
  if ( NULL == InElement || NULL == InValue ) {
    return false;
  }

  switch (InElement->type) {
    case ScreenElementValueTypeNone : {
      break;
    }
    case ScreenElementValueTypeString : {
      return ScreenElementValueSetString(InElement, InValue);
    }
    case ScreenElementValueTypeInt : {
      return ScreenElementValueSetInt(InElement, InValue);
    }
    case ScreenElementValueTypeDimension : {
      return ScreenElementValueSetDimension(InElement, InValue);
    }
    case ScreenElementValueTypeColor : {
      return ScreenElementValueSetColor(InElement, InValue);
    }
  }
  return false;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetString
 *****************************************************************************/
bool
ScreenElementValueSetString
(ScreenElementValue* InValue, string InString)
{
  if ( NULL == InValue || NULL == InString ) {
    return false;
  }
  if ( InValue->type != ScreenElementValueTypeString ) {
    return false;
  }
  if ( InValue->valueString ) {
    FreeMemory(InValue->valueString);
  }
  InValue->valueString = StringCopy(InString);
  return true;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetString
 *****************************************************************************/
bool
ScreenElementValueSetInt
(ScreenElementValue* InValue, string InString)
{
  if ( NULL == InValue || NULL == InString ) {
    return false;
  }
  if ( InValue->type != ScreenElementValueTypeString ) {
    return false;
  }
  if ( InValue->valueString ) {
    FreeMemory(InValue->valueString);
  }
  InValue->valueInt = atoi(InString);
  return true;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetString
 *****************************************************************************/
bool
ScreenElementValueSetDimension
(ScreenElementValue* InValue, string InString)
{
  string                                valueType;
  int                                   valueInt;
  float                                 valueFloat;
  string                                s2;
  int                                   n;
  string                                end;
  string                                start;
  ScreenElementValueDimensionType       type;

  valueType = StringCopy("px");
  start = InString;
  end = start;
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
  
  n = end - start;
  s2 = StringNCopy(start, n);
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
    InValue->valueDimension.value.dimensionFloat = valueFloat;
  } else {
    InValue->valueDimension.value.dimensionInt = valueInt;
  }
  InValue->valueDimension.dimensionSuffix = valueType;
  InValue->valueDimension.dimensionType   = type;
  return true;
}

/*****************************************************************************!
 * Function : ScreenElementValueSetString
 *****************************************************************************/
bool
ScreenElementValueSetColor
(ScreenElementValue* InValue, string InString)
{
  InValue->valueColor.colorType = ScreenElementValueColorTypeString;
  InValue->valueColor.colorString = StringCopy(InString);
  return true;
}


/*****************************************************************************!
 * Function : ScreenElementValueCopy
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopy
(ScreenElementValue* InValue)
{
  if ( NULL == InValue ) {
    return NULL;
  }

  switch (InValue->type) {
    case ScreenElementValueTypeNone : {
      return NULL;
    }
    case ScreenElementValueTypeString : {
      return ScreenElementValueCopyString(InValue);
    }
    case ScreenElementValueTypeInt : {
      ScreenElementValueCopyInt(InValue);
    }
    case ScreenElementValueTypeColor : {
      ScreenElementValueCopyColor(InValue);
    }
    case ScreenElementValueTypeDimension : {
      ScreenElementValueCopyDimension(InValue);
    }
  }
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementValueCopyString
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopyString
(ScreenElementValue* InValue)
{
  ScreenElementValue*                   value;
  if ( NULL == InValue ) {
    return NULL;
  }
  value = (ScreenElementValue*)GetMemory(sizeof(ScreenElementValue));
  value->type = InValue->type;
  value->valueString = StringCopy(InValue->valueString);
  return value;
}

/*****************************************************************************!
 * Function : ScreenElementValueCopyInt
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopyInt
(ScreenElementValue* InValue)
{
  ScreenElementValue*                   value;
  if ( NULL == InValue ) {
    return NULL;
  }
  value = (ScreenElementValue*)GetMemory(sizeof(ScreenElementValue));
  value->type = InValue->type;
  value->valueInt = InValue->valueInt;
  return value;  
}

/*****************************************************************************!
 * Function : ScreenElementValueCopyColor
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopyColor
(ScreenElementValue* InValue)
{
  ScreenElementValue*                   value;
  if ( NULL == InValue ) {
    return NULL;
  }
  value = (ScreenElementValue*)GetMemory(sizeof(ScreenElementValue));
  value->type = InValue->type;
  switch (value->valueColor.colorType) {
    case ScreenElementValueColorTypeNone : {
      break;
    }
    case ScreenElementValueColorTypeString : {
      value->valueColor.colorString = StringCopy(InValue->valueColor.colorString);
      break;
    }
    case ScreenElementValueColorTypeRGB : {
      value->valueColor.rgb.red         = InValue->valueColor.rgb.red;
      value->valueColor.rgb.green       = InValue->valueColor.rgb.green;
      value->valueColor.rgb.blue        = InValue->valueColor.rgb.blue;
      break;
    }
  }
  return value;  
}

/*****************************************************************************!
 * Function : ScreenElementValueCopyDimension
 *****************************************************************************/
ScreenElementValue*
ScreenElementValueCopyDimension
(ScreenElementValue* InValue)
{
  ScreenElementValue*                   value;
  if ( NULL == InValue ) {
    return NULL;
  }
  value = (ScreenElementValue*)GetMemory(sizeof(ScreenElementValue));
  value->type = InValue->type;

  switch (value->valueDimension.dimensionType) {
    case ScreenElementValueDimensionTypeNone : {
      
      break;
    }
    case ScreenElementValueDimensionTypeInt : {
      value->valueDimension.value.dimensionInt = InValue->valueDimension.value.dimensionInt;
      break;
    }
    case ScreenElementValueDimensionTypeFloat : {
      value->valueDimension.value.dimensionFloat = InValue->valueDimension.value.dimensionFloat;      
      break;
    }
  }
  value->valueDimension.dimensionSuffix = StringCopy(value->valueDimension.dimensionSuffix);
  return value;
}
