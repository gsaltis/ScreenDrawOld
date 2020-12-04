/*****************************************************************************
 * FILE NAME    : ScreenElement.c
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
#include "ScreenElement.h"
#include "GeneralUtilities/String.h"
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
 * Function : ScreenElementCreate
 *****************************************************************************/
ScreenElement*
ScreenElementCreate
(ScreenElementType InType, string InName )
{
  ScreenElement*                        element;

  element = (ScreenElement*)GetMemory(sizeof(ScreenElement));
  element->type = InType;
  if ( InName ) {
    element->name = StringCopy(InName);
  }
  memset(element, 0x00, sizeof(ScreenElement));
  return element;
}

/*****************************************************************************!
 * Function : ScreenElementDestroy
 *****************************************************************************/
void
ScreenElementDestroy
(ScreenElement* InElement)
{
  int                                   i;
  if ( NULL == InElement ) {
    return;
  }

  for (i = 0; i < InElement->valuesCount; i++) {
    ScreenElementValueDestroy(InElement->values[i]);
  }
  switch (InElement->type) {
    case ScreenElementTypeNone : {
      
      break;
    }
    case ScreenElementTypeBox : {
      
      break;
    }
    case ScreenElementTypeText : {
      FreeMemory(InElement->valueText);
      break;
    }
    case ScreenElementTypeInput : {
      
      break;
    }
  }
  if ( InElement->name ) {
    FreeMemory(InElement->name);
  }
  if ( InElement->values ) {
    FreeMemory(InElement->values);
  }
  FreeMemory(InElement);
}

/*****************************************************************************!
 * Function : ScreenElementAddValue
 *****************************************************************************/
void
ScreenElementAddValue
(ScreenElement* InElement, ScreenElementValue* InValue)
{
  int                                   i;
  ScreenElementValue**                  values;
  int                                   n;
  if ( NULL == InElement || NULL == InValue ) {
    return;
  }

  n = InElement->valuesCount + 1;

  values = (ScreenElementValue**)GetMemory(sizeof(ScreenElementValue*) * n);
  for (i = 0; i < InElement->valuesCount; i++) {
    values[i] = InElement->values[i];
  }
  if ( InElement->values ) {
    FreeMemory(InElement->values);
  }
  values[n] = InValue;
  InElement->values = values;
  InElement->valuesCount = n;
}

/*****************************************************************************!
 * Function : ScreenElementCreateBox
 *****************************************************************************/
ScreenElement*
ScreenElementCreateBox
(string InName)
{
  return ScreenElementCreate(ScreenElementTypeBox, InName);
}

/*****************************************************************************!
 * Function : ScreenElementCreateText
 *****************************************************************************/
ScreenElement*
ScreenElementCreateText
(string InName, string InText)
{
  ScreenElement*                        element;

  if ( NULL == InText ) {
    return NULL;
  }
  
  element = ScreenElementCreate(ScreenElementTypeText, InName);
  element->valueText = StringCopy(InText);
  return element;
}

/*****************************************************************************!
 * Function : ScreenElementCreateInput
 *****************************************************************************/
ScreenElement*
ScreenElementCreateInput
(string InName)
{
  return ScreenElementCreate(ScreenElementTypeInput, InName);
}

/*****************************************************************************!
 * Function : ScreenElementToJSON
 *****************************************************************************/
JSONOut*
ScreenElementToJSON
(ScreenElement* InElement)
{
  JSONOut*                              jsonout;

  jsonout = NULL;

  return jsonout;
}
