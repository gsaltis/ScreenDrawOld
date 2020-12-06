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
JSONOut*
ScreenElementInputToJSON
(ScreenElement* InElement);

JSONOut*
ScreenElementTextToJSON
(ScreenElement* InElement);

JSONOut*
ScreenElementBoxToJSON
(ScreenElement* InElement);

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
static int
ScreenElementNameID = 1;

/*****************************************************************************!
 * Function : ScreenElementCreate
 *****************************************************************************/
ScreenElement*
ScreenElementCreate
(ScreenElementType InType, string InName )
{
  ScreenElement*                        element;

  element = (ScreenElement*)GetMemory(sizeof(ScreenElement));
  memset(element, 0x00, sizeof(ScreenElement));
  element->type = InType;
  if ( InName ) {
    element->name = StringCopy(InName);
  }
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
  values[InElement->valuesCount] = InValue;
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

  if ( NULL == InElement ) {
    return NULL;
  }

  jsonout = NULL;
  switch (InElement->type) {
    case ScreenElementTypeNone : {
      break;
    }
    case ScreenElementTypeBox : {
      jsonout = ScreenElementBoxToJSON(InElement);      
      break;
    }
    case ScreenElementTypeText : {
      jsonout = ScreenElementTextToJSON(InElement);      
      break;
    }
    case ScreenElementTypeInput : {
      jsonout = ScreenElementInputToJSON(InElement);      
      break;
    }
  }
  return jsonout;
}


/*****************************************************************************!
 * Function : ScreenElementBoxToJSON
 *****************************************************************************/
JSONOut*
ScreenElementBoxToJSON
(ScreenElement* InElement)
{
  int                                   i;
  JSONOut*                              jsonout;

  jsonout = JSONOutCreateObject(NULL);
  JSONOutObjectAddObject(jsonout, JSONOutCreateString("type", "box"));
  JSONOutObjectAddObject(jsonout, JSONOutCreateString("name", InElement->name));
  for (i = 0; i < InElement->valuesCount; i++) {
    JSONOutObjectAddObject(jsonout, ScreenElementValueToJSON(InElement->values[i]));
  }
  return jsonout;
}

/*****************************************************************************!
 * Function : ScreenElementTextToJSON
 *****************************************************************************/
JSONOut*
ScreenElementTextToJSON
(ScreenElement* InElement)
{
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementInputToJSON
 *****************************************************************************/
JSONOut*
ScreenElementInputToJSON
(ScreenElement* InElement)
{
  return NULL;
}

/*****************************************************************************!
 * Function : ScreenElementGenerateName
 *****************************************************************************/
string
ScreenElementGenerateName
()
{
  char                                  s[16];
  sprintf(s, "Element%04d", ScreenElementNameID);
  ScreenElementNameID++;
  return StringCopy(s);
}
