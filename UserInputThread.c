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

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "UserInputThread.h"
#include "RPiBaseModules/linenoise.h"
#include "GeneralUtilities/String.h"
#include "GeneralUtilities/ANSIColors.h"
#include "ScreenElement.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
void
UserInputProcessCommandCreateElement
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
  StringList*                           commands;

  if ( NULL == InCommandLine ) {
    return NULL;
  }
  commands = StringSplit(InCommandLine, " \t", true);
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
  printf("  %squit%s   : %sQuit this qpplication%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
  printf("  %screate%s : %sCreate a new object%s\n", ColorGreen, ColorReset, ColorYellow, ColorReset);
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
  JSONOut*                              jsonout;
  ScreenElement*                        element;
  if ( InCommands->stringCount < 3 ) {
    return;
  }
  if ( StringEqualNoCase(InCommands->strings[2], "box") ) {
    
  }

  element = ScreenElementCreate(ScreenElementTypeBox, "Box");
  jsonout = ScreenElementToJSON(element);

  JSONOutDestroy(jsonout);
}
