/*****************************************************************************
 * FILE NAME    : UserInputThread.c
 * DATE         : December 01 2020
 * PROJECT      : 
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
/*****************************************************************************!
 * Global Headers
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "UserInputThread.h"
#include "RPiBaseModules/linenoise.h"
#include "GeneralUtilities/String.h"
#include "GeneralUtilities/ANSIColors.h"
#include "ScreenElement.h"
#include "GeneralUtilities/MemoryManager.h"
#include "WebSocketServer.h"
#include "main.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
void
UserInputProcessCommandShowSets
(StringList* InCommands);

void
UserInputDisplayErrorMessage
(string InMessage);

void
UserInputDisplayMessage
(string InMessage);

void
UserInputProcessCommandSetValue
(StringList* InCommands);

void
UserInputProcessCommandSet
(StringList* InCommands);

void
UserInputProcessCommandShowScreen
(StringList* InCommands);

void
UserInputProcessCommandShow
(StringList* InCommands);

void
UserInputProcessCommandConnections
(StringList* InCommands);

void
UserInputProcessCommandCreateElement
(StringList* InCommands);

void
UserInputProcessCommandCreateBox
(StringList* InCommands);

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
static string
UserInputPrompt = "Enter Command : ";

static pthread_t
UserInputThreadID;

/*****************************************************************************!
 * Function : UserInputInitialize
 *****************************************************************************/
void
UserInputInitialize
()
{
}

/*****************************************************************************!
 * Function : UserInputServerStart
 *****************************************************************************/
void
UserInputServerStart
()
{
  if ( pthread_create(&UserInputThreadID, NULL, UserInputThread, NULL) ) {
    fprintf(stderr, "Could not start User Input Thread\n");
    exit(EXIT_FAILURE);
  }
}

/*****************************************************************************!
 * Function : UserInputThread
 *****************************************************************************/
void*
UserInputThread
(void* InParameter)
{
  StringList*                           commandArgs;
  string                                inputString;
  while ( true ) {
    inputString = linenoise(UserInputPrompt);
    commandArgs = UserInputParseCommandLine(inputString);
    if ( commandArgs ) {
      UserInputProcessCommand(commandArgs);
      StringListDestroy(commandArgs);
    }
    linenoiseHistoryAdd(inputString);
    free(inputString);
  }
}

/*****************************************************************************!
 * Function : UserInputGetThreadID
 *****************************************************************************/
pthread_t
UserInputGetThreadID
()
{
  return UserInputThreadID;
}


/*****************************************************************************!
 * Function : UserInputParseCommandLine
 *****************************************************************************/
StringList*
UserInputParseCommandLine
(string InCommandLine)
{
  string                                s;
  int                                   n;
  string                                start;
  string                                end;
  StringList*                           commands;
  enum
  {
   InStart = 1,
   InSpaces,
   InToken,
   InSingleQuotedString,
   InDoubleQuotedString,
   InDone
  } state = InStart;
  
  if ( NULL == InCommandLine ) {
    return NULL;
  }
  if ( StringContainsChar(InCommandLine, '"') ) {
    commands = StringListCreate();
    start = InCommandLine;
    end = start;
    while ( state != InDone ) {
      if ( *end == 0x00 ) {
        state = InDone;
        continue;
      }
      switch (state) {
        case InDone : {
          break;
        }
          
          //!
        case InStart : {
          if ( *start == 0x00 ) {
            state = InDone;
            break;
          }
          if (isspace(*start) ) {
            start++;
            state = InSpaces;
            break;
          }
          if ( *start == '"' ) {
            end = start;
            end++;
            state = InDoubleQuotedString;
            break;
          }
          if  ( *start == '\'' ) {
            end = start;
            end++;
            state = InSingleQuotedString;
            break;
          }
          end = start;
          end++;
          state = InToken;
          break;
        }
          //!
        case InSpaces : {
          if ( *start == 0x00 ) {
            state = InDone;
            break;
          }
          
          if ( isspace(*start) ) {
            start++;
            break;
          }
          if ( *start == '"' ) {
            end = start;
            end++;
            state = InDoubleQuotedString;
            break;
          }
          if  ( *start == '\'' ) {
            end = start;
            end++;
            state = InSingleQuotedString;
            break;
          }
          end = start;
          end++;
          state = InToken;
          break;
        }

          //!
        case InToken : {
          if ( *end == 0x00 ) {
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            state = InDone;
            break;
          }
          
          if ( isspace(*end) ) {
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            start = end;
            state = InSpaces;
            break;
          }
          end++;
          break;
        }

          //!
        case InSingleQuotedString : {
          if (*end == '\\' ) {
            end++;
            if ( *end ) {
              end++;
              break;
            }
            state = InDone;
            break;
          }
          if ( *end == 0x00 ) {
            start++;
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            state = InDone;
            break;
          }
          if ( *end == '\'' ) {
            start++;
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            end++;
            start = end;
            if ( *end == 0x00 ) {
              state = InDone;
              break;
            }
            if ( isspace(*start) ) {
              state = InSpaces;
              end++;
              break;
            }
            if ( *start == '"' ) {
              state = InDoubleQuotedString;
              end++;
              break;
            }
            if ( *start == '\'' ) {
              state = InSingleQuotedString;
              end++;
              break;
            }
            end++;
            state = InToken;
            break;
          }
          end++;
          break;
        }
          
          //!
        case InDoubleQuotedString : {
          if (*end == '\\' ) {
            end++;
            if ( *end ) {
              end++;
              break;
            }
            state = InDone;
            break;
          }
          if ( *end == 0x00 ) {
            start++;
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            state = InDone;
            break;
          }            
          if ( *end == '"' ) {
            start++;
            n = end - start;
            s = StringNCopy(start, n);
            StringListAppend(commands, s);
            end++;
            start = end;
            if ( *end == 0x00 ) {
              state = InDone;
              break;
            }
            if ( isspace(*start) ) {
              state = InSpaces;
              end++;
              break;
            }
            if ( *start == '"' ) {
              state = InDoubleQuotedString;
              end++;
              break;
            }
            if ( *start == '\'' ) {
              state = InSingleQuotedString;
              end++;
              break;
            }
            end++;
            state = InToken;
            break;
          }
          end++;
          break;
        }
      }
    }
  } else {
    commands = StringSplit(InCommandLine, " \t", true);
  }
  return commands;
}

/*****************************************************************************!
 * Function : UserInputProcessCommand
 *****************************************************************************/
void
UserInputProcessCommand
(StringList* InCommands)
{
  string                                command;
  if ( NULL == InCommands ) {
    return;
  }
  if ( InCommands->stringCount == 0 ) {
    return;
  }

  command = InCommands->strings[0];
  if ( StringEqualNoCase(command, "quit") ) {
    UserInputProcessCommandQuit(InCommands);
    return;
  }

  if ( StringEqualNoCase(command, "help") ) {
    UserInputProcessCommandHelp(InCommands);
    return;
  }

  if ( StringEqualNoCase(command, "create") ) {
    UserInputProcessCommandCreate(InCommands);
    return;
  }

  if ( StringEqualNoCase(command, "connections") ) {
    UserInputProcessCommandConnections(InCommands);
    return;
  }

  if ( StringEqualNoCase(command, "show") ) {
    UserInputProcessCommandShow(InCommands);
    return;
  }

  if ( StringEqualNoCase(command, "set") ) {
    UserInputProcessCommandSet(InCommands);
    return;
  }
  fprintf(stderr, "%s\"%s\"%s is not a valid command\n", ColorBrightRed, command, ColorReset);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandQuit
 *****************************************************************************/
void
UserInputProcessCommandQuit
(StringList* InCommands)
{
  exit(EXIT_SUCCESS);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandHelp
 *****************************************************************************/
void
UserInputProcessCommandHelp
(StringList* InCommands)
{
  printf("%sCOMMANDS%s\n\n", ColorGreen, ColorReset);
  printf("  %squit%s                 : %sQuit this qpplication%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
  printf("  %screate%s               : %sCreate a new object%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
  printf("  %sset value type value%s : %sSet a Type Value%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
  printf("  %shelp%s   : %sDisplay this message%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandCreate
 *****************************************************************************/
void
UserInputProcessCommandCreate
(StringList* InCommands)
{
  if ( NULL == InCommands ) {
    return;
  }
  if ( InCommands->stringCount < 2 ) {
    return;
  }

  if ( StringEqualNoCase(InCommands->strings[1], "element") ) {
    UserInputProcessCommandCreateElement(InCommands);
    return;
  }
  fprintf(stderr, "\"%s%s%s\" is an invalid subcommand to \"%s%s%s\"\n",
          ColorBrightRed, InCommands->strings[1], ColorReset,
          ColorYellow, InCommands->strings[0], ColorReset);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandCreateElement
 *****************************************************************************/
void
UserInputProcessCommandCreateElement
(StringList* InCommands)
{
  if ( InCommands->stringCount < 4 ) {
    return;
  }
  if ( StringEqualNoCase(InCommands->strings[2], "box") ) {
    UserInputProcessCommandCreateBox(InCommands);
  }
}

/*****************************************************************************!
 * Function : UserInputProcessCommandCreateBox
 *****************************************************************************/
void
UserInputProcessCommandCreateBox
(StringList* InCommands)
{
  string                                s;
  StringList*                           s2;
  int                                   n;
  string                                name;
  ScreenElementValue*                   value;
  JSONOut*                              jsonout;
  ScreenElement*                        element;

  n = 3;
  s = InCommands->strings[n];
  if ( StringContainsChar(s, '=') ) {
    name = ScreenElementGenerateName();
  } else {
    name = StringCopy(s);
    n++;
  }
  element = ScreenFindElementByName(mainScreen, name);
  if ( element != NULL ) {
    printf("%sAn element named \"%s\" exists%s\n", ColorRed, name, ColorReset);
    FreeMemory(name);
    return;
  }
  element = ScreenElementCreateBox(name);
  FreeMemory(name);

  for ( ; n < InCommands->stringCount; n++ ) {
    s = InCommands->strings[n];
    if ( StringContainsChar(s, '=') ) {
      s2 = StringSplit(s, "=", false);
      if ( s2 ) {
        if ( s2->stringCount == 2 ) {
          if ( StringEqualsOneOf(s2->strings[0], "left", "right", "top", "bottom", "width", "height", "font-size", NULL) ) {
            value = ScreenElementValueCreateDimension(s2->strings[0], s2->strings[1]);
          } else if ( StringEqualsOneOf(s2->strings[0], "background", "color", NULL) ) {
            value  = ScreenElementValueCreateColorString(s2->strings[0], s2->strings[1]);
          } else {
            value = ScreenElementValueCreateString(s2->strings[0], s2->strings[1]);
          }
          ScreenElementAddValue(element, value);
        }
        StringListDestroy(s2);
      }
    }
  }

  ScreenAddElement(mainScreen, element);
  jsonout = ScreenElementToJSON(element);
  JSONOutSetName(jsonout, "body");
  JSONOutObjectAddObject(jsonout, JSONOutCreateString("createtype", "element"));
  WebSocketSendCreate(jsonout);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandConnections
 *****************************************************************************/
void
UserInputProcessCommandConnections
(StringList* InCommands)
{
  WebSocketDisplayConnections();
}

/*****************************************************************************!
 * Function : UserInputProcessCommandShow
 *****************************************************************************/
void
UserInputProcessCommandShow
(StringList* InCommands)
{
  if ( NULL == InCommands ) {
    return;
  }
  if ( InCommands->stringCount < 2 ) {
    return;
  }

  if ( StringEqualNoCase(InCommands->strings[1], "screen") ) {
    UserInputProcessCommandShowScreen(InCommands);
    return;
  }
  if ( StringEqualNoCase(InCommands->strings[1], "sets") ) {
    UserInputProcessCommandShowSets(InCommands);
    return;
  }
}

/*****************************************************************************!
 * Function : UserInputProcessCommandShowScreen
 *****************************************************************************/
void
UserInputProcessCommandShowScreen
(StringList* InCommands)
{
  int                                   i;
  if ( NULL == InCommands ) {
    return;
  }

  printf("%sSCREENS%s\n", ColorBrightGreen, ColorReset);
  printf("%s%s%s\n", ColorGreen, mainScreen->name ? mainScreen->name : "No Name", ColorReset);
  printf("%s  VALUES%s\n", ColorBoldYellowReverse, ColorReset);
  for ( i = 0; i < mainScreen->elementsCount; i++ ) {
    ScreenElementDisplay(mainScreen->elements[i], 0);
  }
}

/*****************************************************************************!
 * Function : UserInputProcessCommandSet
 *****************************************************************************/
void
UserInputProcessCommandSet
(StringList* InCommands)
{
  string                                command;
  if ( NULL == InCommands ) {
    return;
  }
  if ( InCommands->stringCount < 2 ) {
    return;
  }
  command = InCommands->strings[1];
  if ( StringEqualNoCase(command, "value") ) {
    UserInputProcessCommandSetValue(InCommands);
    return;
  }
}

/*****************************************************************************!
 * Function : UserInputProcessCommandSetValue
 *****************************************************************************/
void
UserInputProcessCommandSetValue
(StringList* InCommands)
{
  StringList*                           s2;
  ScreenElementValue*                   value;
  string                                s;
  string                                valueString;
  ScreenElementValueSet*                set;
  string                                type;

  if ( NULL == InCommands ) {
    return;
  }
  if ( InCommands->stringCount < 4 ) {
    return;
  }
  type = InCommands->strings[2];

  set = ScreenElementValueSetListFindByName(mainScreenElementValueSets, type);
  if ( NULL == set ) {
    s = StringMultiConcat("\"", type, "\" is not a recognized type", NULL); 
    UserInputDisplayErrorMessage(s);
    FreeMemory(s);
    return;
  }
  valueString = InCommands->strings[3];
  s2 = StringSplit(valueString, "=", true);
  if ( NULL == s2 ) {
    return;
  }
  if ( s2->stringCount != 2 ) {
    s = StringMultiConcat("\"", valueString, "\" is not a valid value");
    UserInputDisplayErrorMessage(s);
    FreeMemory(s);
    return;
  }
  value = ScreenElementValueSetFindByName(set, s2->strings[0]);
  if ( value != NULL ) {
    s = StringMultiConcat("\"", s2->strings[0], "\" is not a recognized value in set \"", type, "\"", NULL);
    UserInputDisplayErrorMessage(s);
    StringListDestroy(s2);
    FreeMemory(s);
    return;
  }
  if ( ScreenElementValueSetValue(value, s2->strings[1]) ) {
    s = StringMultiConcat("\"", type, "\" value \"", s2->strings[0], "\" set to \"", s2->strings[1], "\"", NULL);
    UserInputDisplayMessage(s);
    StringListDestroy(s2);
    FreeMemory(s);
    return;
  }

  s = StringMultiConcat("\"", type, "\" value \"", s2->strings[0], "\" not set", NULL);
  UserInputDisplayErrorMessage(s);
  StringListDestroy(s2);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : UserInputDisplayErrorMessage
 *****************************************************************************/
void
UserInputDisplayErrorMessage
(string InMessage)
{
  printf("%s%s%s\n", ColorRed, InMessage, ColorReset);
}

/*****************************************************************************!
 * Function : UserInputDisplayMessage
 *****************************************************************************/
void
UserInputDisplayMessage
(string InMessage)
{
  printf("%s%s%s\n", ColorGreen, InMessage, ColorReset);
}

/*****************************************************************************!
 * Function : UserInputProcessCommandShowSets
 *****************************************************************************/
void
UserInputProcessCommandShowSets
(StringList* InCommands)
{
  int                                   i;
  printf("%sSETS%s\n", ColorGreen, ColorReset);
  printf("%s                               NAME  VALUES%s\n", ColorBoldYellowReverse, ColorReset);
  for (i = 0; i < mainScreenElementValueSets->setsCount; i++) {
    printf("%s%35s%s  ", ColorBoldBlue, mainScreenElementValueSets->sets[i]->name, ColorReset);
    printf("%s%6d%s\n", ColorBoldBlue, mainScreenElementValueSets->sets[i]->values->valuesCount, ColorReset);
  }
  printf("\n");
}

