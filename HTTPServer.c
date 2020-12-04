/*****************************************************************************
 * FILE NAME    : HTTPServer.c
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
#include <unistd.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "HTTPServer.h"
#include "RPiBaseModules/mongoose.h"
#include "GeneralUtilities/String.h"
#include "GeneralUtilities/ANSIColors.h"
#include "UserInputThread.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
static void
HTTPServerEventHandler
(struct mg_connection* InConnection, int InEvent, void* InParameter);

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
static int
HTTPServerPollPeriod = 20;

static struct mg_serve_http_opts
HTTPServerOptions;

static string
HTTPWWWDirectoryDefault = "www";

static string
HTTPWWWDirectory = NULL;

static string
HTTPPortAddress = NULL;

static string
HTTPPortAddressDefault = "8001";

static struct mg_connection*
HTTPConnection;

static struct mg_mgr
HTTPManager;

static pthread_t
HTTPServerThreadID;

/*****************************************************************************!
 * Function : HTTPServerInitialize
 *****************************************************************************/
void
HTTPServerInitialize
()
{
  HTTPPortAddress = StringCopy(HTTPPortAddressDefault);
  HTTPWWWDirectory = StringCopy(HTTPWWWDirectoryDefault);
}

/*****************************************************************************!
 * Function : HTTPServerStart
 *****************************************************************************/
void
HTTPServerStart
()
{
  if ( pthread_create(&HTTPServerThreadID, NULL, HTTPServerThread, NULL) ) {
    fprintf(stderr, "Could not create HTTP Server Thread\n");
  }
}

/*****************************************************************************!
 * Function : HTTPServerThread
 *****************************************************************************/
void*
HTTPServerThread
(void* InParameter)
{
  mg_mgr_init(&HTTPManager, NULL);
  HTTPConnection = mg_bind(&HTTPManager, HTTPPortAddress, HTTPServerEventHandler);

  if ( NULL == HTTPConnection ) {
    fprintf(stderr, "%sFailed to create HTTP server%s\n", ColorBrightRed, ColorReset);
    exit(EXIT_FAILURE);
  }

  mg_set_protocol_http_websocket(HTTPConnection);
  HTTPServerOptions.document_root = HTTPWWWDirectory;
  HTTPServerOptions.enable_directory_listing = "yes";
  printf("%sHTTP Server started%s\n", ColorBrightGreen, ColorReset);
  UserInputServerStart();
  while ( true ) {
    mg_mgr_poll(&HTTPManager, HTTPServerPollPeriod);
  }
}

/*****************************************************************************!
 * Function : HTTPServerGetThreadID
 *****************************************************************************/
pthread_t
HTTPServerGetThreadID
()
{
  return HTTPServerThreadID;
}

/*****************************************************************************!
 * Function : HTTPServerEventHandler
 *****************************************************************************/
void
HTTPServerEventHandler
(struct mg_connection* InConnection, int InEvent, void* InParameter)
{
  if ( InEvent == MG_EV_HTTP_REQUEST ) {
    mg_serve_http(InConnection, (struct http_message*)InParameter, HTTPServerOptions);
  }
}


