/*****************************************************************************
 * FILE NAME    : main.c
 * DATE         : December 01 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/

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
#include "main.h"
#include "UserInputThread.h"
#include "WebSocketServer.h"
#include "HTTPServer.h"
#include "Version.h"
#include "GeneralUtilities/ANSIColors.h"
#include "Screen.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
Screen*
mainScreen = NULL;

string
mainScreenName = NULL;

string
mainScreenNameDefault = "Screen";

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
void
MainCreateScreen
();

void
MainJoinThreads
();

void
MainCreateThreads
();

void
MainDisplayStartupInfo
();

void
MainInitialize
();

/*****************************************************************************!
 * Function : main
 *****************************************************************************/
int
main
(int argc, char** argv)
{
  MainInitialize();
  MainCreateScreen();
  MainDisplayStartupInfo();
  
  MainCreateThreads();

  WebSocketServerCreateInfoScript();

  // The threads are daisy chained and each is started after the other is started
  WebSocketServerStart();

  MainJoinThreads();
  return EXIT_SUCCESS;
}

/*****************************************************************************!
 * Function : MainDisplayStartupInfo
 *****************************************************************************/
void
MainDisplayStartupInfo
()
{
  printf("%s%s : ", ColorBrightGreen, PROGRAM_NAME);
  VersionDisplay();
  printf("%s", ColorReset);
}

/*****************************************************************************!
 * Function : MainCreateThreads
 *****************************************************************************/
void
MainCreateThreads
()
{
  UserInputInitialize();
  HTTPServerInitialize();
  WebSocketServerInitialize();  
}

/*****************************************************************************!
 * Function : MainJoinThreads
 *****************************************************************************/
void
MainJoinThreads
()
{
  pthread_join(UserInputGetThreadID(), NULL);
  pthread_join(HTTPServerGetThreadID(), NULL);
  pthread_join(WebSocketServerGetThreadID(), NULL);  
}

/*****************************************************************************!
 * Function : MainInitialize
 *****************************************************************************/
void
MainInitialize
()
{
  mainScreenName = StringCopy(mainScreenNameDefault);
}

/*****************************************************************************!
 * Function : MainCreateScreen
 *****************************************************************************/
void
MainCreateScreen
()
{
  mainScreen = ScreenCreate(mainScreenName);
}
