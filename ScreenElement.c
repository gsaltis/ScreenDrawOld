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
  element->values = ScreenElementValueListCreate();
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
  if ( NULL == InElement ) {
    return;
  }

  ScreenElementValueListDestroy(InElement->values);
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
  if ( NULL == InElement || NULL == InValue ) {
    return;
  }

  ScreenElementValueListAppend(InElement->values, InValue);
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
  JSONOut*                              values;
  int                                   i;
  JSONOut*                              jsonout;

  jsonout = JSONOutCreateObject(NULL);
  JSONOutObjectAddObject(jsonout, JSONOutCreateString("type", "box"));
  JSONOutObjectAddObject(jsonout, JSONOutCreateString("name", InElement->name));
  values = JSONOutCreateObject("values");
  JSONOutObjectAddObject(jsonout, values);
  for (i = 0; i < InElement->values->valuesCount; i++) {
    JSONOutObjectAddObject(values, ScreenElementValueToJSON(InElement->values->values[i]));
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

/*****************************************************************************!
 * Function : ScreenElementDisplay
 *****************************************************************************/
void
ScreenElementDisplay
(ScreenElement* InElement, int InIndent)
{
  string                                s2;
  string                                s;
  if ( NULL == InElement ) {
    return;
  }
  if ( InIndent > 0 ) {
    s = StringFill(' ', InIndent);
  } else {
    s = (string)GetMemory(1);
    *s = 0x00;
  }
  printf("%s", s);
  s2 = ScreenElementTypeToString(InElement->type);
  printf("%10s %35s\n", s2, InElement->name);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : ScreenElementTypeToString
 *****************************************************************************/
string
ScreenElementTypeToString
(ScreenElementType InType)
{
  switch (InType) {
    case ScreenElementTypeNone : {
      return "None";
    }
    case ScreenElementTypeBox : {
      return "Box";
    }
    case ScreenElementTypeText : {
      return "Text";
    }
    case ScreenElementTypeInput : {
      return "Input";
    }
  }
  return "";
}
