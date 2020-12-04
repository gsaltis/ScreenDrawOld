/*****************************************************************************
 * FILE NAME    : UserInputThread.h
 * DATE         : December 01 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _userinputthread_h_
#define _userinputthread_h_

/*****************************************************************************!
 * Global Headers
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "GeneralUtilities/String.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
void
UserInputProcessCommandCreate
(StringList* InCommands);

void
UserInputProcessCommandHelp
(StringList* InCommands);

void
UserInputServerStart
();

void
UserInputProcessCommandQuit
(StringList* InCommands);

void
UserInputProcessCommand
(StringList* InCommands);

StringList*
UserInputParseCommandLine
(string InCommandLine);

pthread_t
UserInputGetThreadID
();

void*
UserInputThread
(void* InParameter);

void
UserInputInitialize
();

#endif /* _userinputthread_h_*/
