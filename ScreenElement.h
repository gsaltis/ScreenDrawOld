/*****************************************************************************
 * FILE NAME    : ScreenElement.h
 * DATE         : December 03 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _screenelement_h_
#define _screenelement_h_

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
#include "ScreenElementValue.h"
#include "JSONOut.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Type : ScreenElementType
 *****************************************************************************/
typedef enum ScreenElementType
{
 ScreenElementTypeNone                  = 0,
 ScreenElementTypeBox,
 ScreenElementTypeText,
 ScreenElementTypeInput,
} ScreenElementType;

/*****************************************************************************!
 * Exported Type : ScreenElement
 *****************************************************************************/
struct _ScreenElement
{
  ScreenElementType                     type;
  string                                name;
  union
  {
    string                              valueText;
  };
  ScreenElementValue**                  values;
  uint32_t                              valuesCount;
};
typedef struct _ScreenElement ScreenElement;

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
JSONOut*
ScreenElementToJSON
(ScreenElement* InElement);

ScreenElement*
ScreenElementCreateInput
(string InName);

ScreenElement*
ScreenElementCreateText
(string InName, string InText);

ScreenElement*
ScreenElementCreateBox
(string InName);

void
ScreenElementAddValue
(ScreenElement* InElement, ScreenElementValue* InValue);

void
ScreenElementDestroy
(ScreenElement* InElement);

ScreenElement*
ScreenElementCreate
(ScreenElementType InType, string InName);

#endif /* _screenelement_h_*/
