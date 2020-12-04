/*****************************************************************************
 * FILE NAME    : WebSocketIF.h
 * DATE         : March 27 2020
 * PROJECT      : Bay Simulator
 * COPYRIGHT    : Copyright (C) 2020 by Vertiv Company
 *****************************************************************************/
#ifndef _websocketif_h_
#define _websocketif_h_

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
#include "mongoose.h"
#include "FuseBreaker.h"
#include "Bay.h"
#include "Panel.h"
#include "PanelConnection.h"
#include "String.h"

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/
#define WEBSOCKET_IF_MAX_ADDRESS_TRIES		30
#define WEBSOCKET_IF_ADDRESS_WAIT_PERIOD	2

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/
extern pthread_t
WebSocketServerThreadID;

extern string
s_websocket_port;

extern string
WebSocketIFAddress;

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
void
WebSocketIFCreateInfoScript
();

void
WebSocketServerInit();

void
WebSocketIFSendFuseBreaker
(FuseBreaker* InFuseBreaker);

void
WebSocketIFSendBay
(Bay* InBay);

void
WebSocketIFSendPanel
(Panel* InPanel);

void
WebSocketIFRemoveBay
(int InBayIndex);

void
WebSocketIFRemovePanel
(int InBayIndex, int InPanelIndex);

void
WebSocketIFRemoveFuseBreaker
(int InBayIndex, int InPanelIndex, int InFuseBreakerIndex);

void
WebSocketIFAddPanelConnection
(int InBayIndex, int InFromIndex, int InToIndex);

void
WebSocketIFSendRemovePanelConnect
(PanelConnection* InConnection);

void
WebSocketIFSendPanelConnect
(PanelConnection* InConnection);

void
WebSocketIFSetPort
(string InPortNumber);

#endif /* _websocketif_h_*/
