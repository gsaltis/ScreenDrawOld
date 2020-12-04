/*****************************************************************************
 * FILE NAME    : WebSocketServer.h
 * DATE         : December 01 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _websocketserver_h_
#define _websocketserver_h_

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
#include "RPiBaseModules/mongoose.h"
#include "JSONOut.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/
#define WEBSOCKET_SERVER_MAX_ADDRESS_TRIES      30
#define WEBSOCKET_SERVER_ADDRESS_WAIT_PERIOD    2

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
void
WebSocketJSONSendAll
(JSONOut* InJSON);

void
WebSocketServerEventHandler
(struct mg_connection* InConnection, int InEvent, void* InParameter);

pthread_t
WebSocketServerGetThreadID
();

void*
WebSocketServerThread
(void* InParameters);

void
WebSocketServerInitialize
();

void
WebSocketServerStart
();

void
WebSocketServerCreateInfoScript
();

#endif /* _websocketserver_h_*/
