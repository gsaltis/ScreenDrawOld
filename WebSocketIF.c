/*****************************************************************************
 * FILE NAME    : WebSocketIF.c
 * DATE         : March 27 2020
 * PROJECT      : Bay Simulator
 * COPYRIGHT    : Copyright (C) 2020 by Vertiv Company
 *****************************************************************************/

/*****************************************************************************!
 * Global Headers
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "WebSocketIF.h"
#include "main.h"
#include "json.h"
#include "String.h"
#include "MemoryManager.h"
#include "JSONIF.h"
#include "Bay.h"
#include "FileUtils.h"
#include "Devices.h"
#include "ThreadSafePrint.h"

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
int
WebSocketIFPacketID = 0;

pthread_t
WebSocketServerThreadID;

string
s_websocket_port = NULL;

string
WebSocketIFAddress = NULL;

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
void
HandleWebSocketRequest
(struct mg_connection* nc, struct mg_str InString);

void
WebSocketEventHandler
(struct mg_connection* nc, int ev, void* p);

void*
WebSocketServerThread
(void* InArgs);

void
HandleGetDeviceDefsRequest
(struct mg_connection* nc, json_value* InPacket);

void
HandleGetBaysValuesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleRemoveFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleRemovePanelRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleRemovePanelConnectRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetDeviceRegsRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleAddPanelRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleAddPanelConnectionRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleAddBayRequest
(struct mg_connection* InConnection, json_value* InPacket);

int
WebSocketIFPacketIDGet
();

void
HandleWriteFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleWritePanelRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleAddFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetPanelRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleCloseRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleOpenRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetFuseBreakerStatesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetFuseBreakerTypesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetPanelTypesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetBayTypesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleWriteBayRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleRemoveBayRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleWriteBaysRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetBayRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetBaysRequest
(struct mg_connection* InConnection, json_value* InValue);

void
HandlePacketRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
WebSocketFrameSend
(struct mg_connection* InConnection, string InBuffer, uint16_t InBufferLen);

void
WebSocketFrameResponseSendError
(struct mg_connection* InConnection, string InResponseType, string InBody,
 int InPacketID, uint32_t InTime,
 string InResponseCode, string InResponseMessage);

void
WebSocketFrameResponseSend
(struct mg_connection* InConnection, string InResponseType, string InBody,
 int InPacketID, uint32_t InTime, string InResponseMessage);

void
WebSocketIFPacketIDSet
(int InPacketID);

void
WebSocketIFBroadcastPush
(string InString);

void
HandleGetBayRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleGetPanelRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleSetPanelRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket);

void
HandleSetBayRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket);

/*****************************************************************************!
 * Function : WebSocketServerInit
 *****************************************************************************/
void
WebSocketServerInit()
{
  pthread_create(&WebSocketServerThreadID, NULL, WebSocketServerThread, NULL);
}

/*****************************************************************************!
 * Function : WebSocketServerThread
 *****************************************************************************/
void*
WebSocketServerThread
(void* InArgs)
{
  struct mg_connection *              nc;
  struct mg_mgr                       mgr;
  if ( MainVerbose ) {
    printf("Starting websocket server thread on %s:%s\n", WebSocketIFAddress, s_websocket_port);
  }
  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, s_websocket_port, WebSocketEventHandler);
  if (nc == NULL) {
    printf("Failed to create webssocket listener\n");
    return (void*)1;
  }
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = MainDefaultWebDirectory;
  s_http_server_opts.enable_directory_listing = "yes";

  for (;;) {
    mg_mgr_poll(&mgr, 20);            
  }
}

/*****************************************************************************!
 * Function : WebSocketEventHandler
 *****************************************************************************/
void
WebSocketEventHandler
(struct mg_connection* nc, int ev, void* p)
{
  WebConnection*			connection;

  if ( ev == 0 ) {
    return;
  }
  switch (ev) {
    case MG_EV_CLOSE : {
      connection = WebConnectionListFind(mainWebConnections, nc);
      if ( connection ) {
        WebConnectionListRemove(mainWebConnections, connection);
 	WebConnectionDestroy(connection);
      }
      break;
    }

    case MG_EV_WEBSOCKET_FRAME: {
      connection = WebConnectionListFind(mainWebConnections, nc);
      if ( NULL == connection ) {
	// This is the first time we've seen this connection, save it
        connection = WebConnectionCreate(nc);
 	WebConnectionListAppend(mainWebConnections, connection);
      }
      WebConnectionTimeUpdate(connection, time(NULL));
      struct websocket_message *wm = (struct websocket_message *) p;
      struct mg_str d = {(char *) wm->data, wm->size};
      HandleWebSocketRequest(nc, d);
      break;
    }
  }
}

/*****************************************************************************!
 * Function : HandleWebSocketRequest
 *****************************************************************************/
void
HandleWebSocketRequest
(struct mg_connection* nc, struct mg_str InMessage)
{
  string                                packettype;
  json_value*                           jsonDoc;

  jsonDoc = json_parse((const json_char*)InMessage.p, (size_t)InMessage.len);

  packettype = JSONIFGetString(jsonDoc, "packettype");

  if ( NULL == packettype ) {
    json_value_free(jsonDoc);
    return;
  }

  if ( StringEqual("request", packettype) ) {
    HandlePacketRequest(nc, jsonDoc);
  } else if ( StringEqual("response", packettype) ) {
    
  } else if ( StringEqual("push", packettype) ) {
    
  }
  FreeMemory(packettype);
  json_value_free(jsonDoc);
}

/*****************************************************************************!
 * Function : HandlePacketRequest
 *****************************************************************************/
void
HandlePacketRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string                                requesttype;
  if ( NULL == InPacket ) {
    return;
  }
 
  requesttype = JSONIFGetString(InPacket, "type");
  if ( NULL == requesttype ) {
    return;
  }

  if ( MainMonitorWebRequest ) {
    printf_safe("%s\n", requesttype);
  }

  if ( StringEqual("getbays", requesttype) ){
    HandleGetBaysRequest(InConnection, InPacket);
  } else if ( StringEqual("setbayregvalues", requesttype) ) {
    HandleSetBayRegValuesRequest(InConnection, InPacket);
  } else if ( StringEqual("setpanelregvalues", requesttype) ) {
    HandleSetPanelRegValuesRequest(InConnection, InPacket);
  } else if ( StringEqual("getdevicedefs", requesttype) ) {
    HandleGetDeviceDefsRequest(InConnection, InPacket);
  } else if ( StringEqual("getdeviceregs", requesttype) ) {
    HandleGetDeviceRegsRequest(InConnection, InPacket);
  } else if ( StringEqual("getbaysvalues", requesttype) ) {
    HandleGetBaysValuesRequest(InConnection, InPacket);
  } else if ( StringEqual("addbay", requesttype) ) {
    HandleAddBayRequest(InConnection, InPacket);
  } else if ( StringEqual("getbay", requesttype) ) {
    HandleGetBayRequest(InConnection, InPacket);
  } else if ( StringEqual("writebays", requesttype) ) {
    HandleWriteBaysRequest(InConnection, InPacket);
  } else if ( StringEqual("writebay", requesttype) ) {
    HandleWriteBayRequest(InConnection, InPacket);
  } else if ( StringEqual("removepanel", requesttype) ) {
    HandleRemovePanelRequest(InConnection, InPacket);
  } else if ( StringEqual("removebay", requesttype) ) {
    HandleRemoveBayRequest(InConnection, InPacket);
  } else if ( StringEqual("getbaytypes", requesttype) ) {
    HandleGetBayTypesRequest(InConnection, InPacket);
  } else if ( StringEqual("getpaneltypes", requesttype) ) {
    HandleGetPanelTypesRequest(InConnection, InPacket);
  } else if ( StringEqual("addpanel", requesttype) ) {
    HandleAddPanelRequest(InConnection, InPacket);
  } else if ( StringEqual("open", requesttype) ) {
    HandleOpenRequest(InConnection, InPacket);
  } else if ( StringEqual("close", requesttype) ) {
    HandleCloseRequest(InConnection, InPacket);
  } else if ( StringEqual("getpanel", requesttype) ) {
    HandleGetPanelRequest(InConnection, InPacket);
  } else if ( StringEqual("writepanel", requesttype) ) {
    HandleWritePanelRequest(InConnection, InPacket);
  } else if ( StringEqual("getbayregvalues", requesttype) ) {
    HandleGetBayRegValuesRequest(InConnection, InPacket);
  } else if ( StringEqual("getpanelregvalues", requesttype) ) {
    HandleGetPanelRegValuesRequest(InConnection, InPacket);
  }
 
  FreeMemory(requesttype);  
}

/*****************************************************************************!
 * Function : HandleSetBayRegValuesRequest
 *****************************************************************************/
void
HandleSetBayRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  json_value*				value;
  json_value*				regs;
  int					bayindex;
  Bay*					bay;
  CanReg*				canreg;
  int					i;
  int					valuetype;
  string				valuestring;
 
  value = JSONIFGetObject(InPacket, "body");

  bayindex = JSONIFGetInt(value, "bayindex");
  regs = JSONIFGetArray(value, "registers");

  bay = FindBayByIndex(bayindex);
  if ( NULL == bay ) {
    return;
  }

  for ( i = 0 ; i < regs->u.array.length; i++ ) {
    valuetype = JSONIFGetInt(regs->u.array.values[i], "valuetype");
    valuestring = JSONIFGetString(regs->u.array.values[i], "value");
   
    canreg = BayFindCANRegister(bay, valuetype);
    if ( canreg ) {
      CanRegSetFromString(canreg, valuestring);
    } 
    FreeMemory(valuestring);
  }
  BaysSaveValues(NULL);
}


/*****************************************************************************!
 * Function : HandleSetPanelRegValuesRequest
 *****************************************************************************/
void
HandleSetPanelRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  json_value*				value;
  int					bayindex, panelindex;
  Bay*					bay;
  Panel*				panel;
 
  value = JSONIFGetObject(InPacket, "body");

  bayindex = JSONIFGetInt(value, "bayindex");
  panelindex = JSONIFGetInt(value, "panelindex");

  bay = FindBayByIndex(bayindex);
  if ( NULL == bay ) {
    return;
  }

  panel = BayFindPanelByIndex(bay, panelindex);
  if ( NULL == panel ) {
    return;
  }

  if ( !panel->panelType->usesSMDUH2 ) {
    return;
  }

  PanelValuesFromJSONString(panel, value);
  BaysSaveValues(NULL);
}

/*****************************************************************************!
 * Function : HandleGetPanelRegValuesRequeset
 *****************************************************************************/
void
HandleGetPanelRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string				bayString;
  int					packetid;
  int					bayid, panelid;
  Bay*					bay;
  Panel*				panel;
  string				s;

  packetid = JSONIFGetInt(InPacket, "packetid");
  bayid = JSONIFGetInt(InPacket, "bayindex");
  panelid = JSONIFGetInt(InPacket, "panelindex");

  bay = FindBayByIndex(bayid);
  if ( NULL == bay ) {
    return;
  }
  panel = BayFindPanelByIndex(bay, panelid);
  if ( NULL == panel ) {
    return;
  }
  bayString = StringCopy("{  \"panelregvalues\" : ");
  s = PanelRegValuesToJSONString(panel);
  bayString = StringConcatTo(bayString, s);
  bayString = StringConcatTo(bayString, "\n}");
  WebSocketFrameResponseSend(InConnection, "respanelregvalues", bayString, packetid, 0, "");

  FreeMemory(s);
  FreeMemory(bayString);
}


/*****************************************************************************!
 * Function : HandleGetBayRegValuesRequeset
 *****************************************************************************/
void
HandleGetBayRegValuesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string				bayString;
  int					packetid;
  int					bayid;
  Bay*					bay;
  string				s;

  packetid = JSONIFGetInt(InPacket, "packetid");
  bayid = JSONIFGetInt(InPacket, "bayindex");
  bay = FindBayByIndex(bayid);
  if ( NULL == bay ) {
    return;
  }
  bayString = StringCopy("{  \"bayregvalues\" : ");
  s = BayRegValuesToJSONString(bay, false);
  bayString = StringConcatTo(bayString, s);
  bayString = StringConcatTo(bayString, "\n}");
  WebSocketFrameResponseSend(InConnection, "resbayregvalues", bayString, packetid, 0, "");

  FreeMemory(s);
  FreeMemory(bayString);
}

/*****************************************************************************!
 * Function : HandleGetBaysValuesRequest
 *****************************************************************************/
void
HandleGetBaysValuesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string                                s;
  int                                   packetid;
  string                                baysString;
  baysString = BaysValueToJSONString();
  packetid = JSONIFGetInt(InPacket, "packetid");
  WebSocketIFPacketIDSet(packetid);

  s = StringCopy("{ \"bays\" : ");
  s = StringConcatTo(s, baysString);

  s = StringConcatTo(s, "\n}");
  WebSocketFrameResponseSend(InConnection, "resbaysvalues", s, packetid, 0, "");
  FreeMemory(baysString);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : HandleGetBaysRequest
 *****************************************************************************/
void
HandleGetBaysRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string                                s;
  int                                   packetid;
  string                                baysString;
  baysString = BaysToJSONString();
  packetid = JSONIFGetInt(InPacket, "packetid");
  WebSocketIFPacketIDSet(packetid);

  s = StringCopy("{ \"bays\" : ");
  s = StringConcatTo(s, baysString);

  s = StringConcatTo(s, "\n}");
  WebSocketFrameResponseSend(InConnection, "resbays", s, packetid, 0, "");
  FreeMemory(baysString);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : HandleGetPanelRequest
 *****************************************************************************/
void
HandleGetPanelRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  Panel*                                panel;
  int                                   panelid;
  string                                bodyString;
  char                                  s2[8];
  string                                s;
  int                                   packetid;
  string                                bayString;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayid;

  body = JSONIFGetObject(InPacket, "body");
  bayid = JSONIFGetInt(body, "bayindex");
  panelid = JSONIFGetInt(body, "panelindex");
  packetid = JSONIFGetInt(InPacket, "packetid");
  bay = FindBayByIndex(bayid);
  if ( bay ) {
    panel = BayFindPanelByIndex(bay, panelid);
  }
  if ( NULL == bay || NULL == panel ) {
    bodyString = StringCopy("\"bayindex\" : ");
    sprintf(s2, "%d, ", bayid);
    bodyString = StringCopy("\"panelindex\" : ");
    sprintf(s2, "%d", panelid);
    bodyString = StringConcatTo(bodyString, s2);
    WebSocketFrameResponseSendError(InConnection, "respanel", bodyString, packetid, 0, "PanelNotFound", "Panel not found");
    FreeMemory(bodyString);
    return;
  }
  WebSocketIFPacketIDSet(packetid + 1);
  bayString = PanelToJSONString(panel);
  
  s = StringCopy("{ ");
  s = StringConcatTo(s, "  \"bayindex\" : ");
  sprintf(s2, "%d", bayid);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");

  s = StringConcatTo(s, "  \"panelindex\" : ");
  sprintf(s2, "%d", panelid);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");
  
  s = StringConcatTo(s, "  \"panel\" : ");
  s = StringConcatTo(s, bayString);
  s = StringConcatTo(s, "\n}");
  WebSocketFrameResponseSend(InConnection, "respanel", s, packetid, 0, "");
  FreeMemory(bayString);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : HandleGetBayRequest
 *****************************************************************************/
void
HandleGetBayRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string                                bodyString;
  char                                  s2[8];
  string                                s;
  int                                   packetid;
  string                                bayString;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayid;

  body = JSONIFGetObject(InPacket, "body");
  bayid = JSONIFGetInt(body, "index");
  packetid = JSONIFGetInt(InPacket, "packetid");
  bay = FindBayByIndex(bayid);
  if ( NULL == bay ) {
    bodyString = StringCopy("\"index\" : ");
    sprintf(s2, "%d", bayid);
    bodyString = StringConcatTo(bodyString, s2);
    WebSocketFrameResponseSendError(InConnection, "resbay", bodyString, packetid, 0, "BayNotFound", "Bay not found");
    FreeMemory(bodyString);
    return;
  }
  WebSocketIFPacketIDSet(packetid + 1);
  bayString = BayToJSONString(bay);
  
  s = StringCopy("{ ");
  s = StringConcatTo(s, "  \"index\" : ");
  sprintf(s2, "%d", bayid);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");
  s = StringConcatTo(s, "  \"bay\" : ");
  s = StringConcatTo(s, bayString);
  s = StringConcatTo(s, "\n}");
  WebSocketFrameResponseSend(InConnection, "resbay", s, packetid, 0, "");
  FreeMemory(bayString);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : HandleWriteBaysRequest
 *****************************************************************************/
void
HandleWriteBaysRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
}

/*****************************************************************************!
 * Function : HandleWriteBayRequest
 *****************************************************************************/
void
HandleWriteBayRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  char                                  bodyString[32];
  char                                  message[128];
  json_value*                           bayinfo;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayindex;
  
  body = JSONIFGetObject(InPacket, "body");
  packetid = JSONIFGetInt(InPacket, "packetid");
  bayindex = JSONIFGetInt(body, "bayindex");
  bayinfo = JSONIFGetObject(body, "bay");

  bay = FindBayByIndex(bayindex);
  sprintf(bodyString, " { \"bayindex\" : %d }", bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "reswritebay", bodyString, packetid, 0, "BayNotFound", message);
    return;
  }

  if ( BayUpdateFromJSONString(bay, bayinfo) ) {
    BaysSave(NULL);
  }
  sprintf(message, "Bay %d written", bayindex);
  WebSocketFrameResponseSend(InConnection, "reswritebay", bodyString, packetid, 0, message);
}

/*****************************************************************************!
 * Function : HandleRemoveBayRequest
 *****************************************************************************/
void
HandleRemoveBayRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayindex;
  int                                   packetid;
  char                                  bodyString[64];
  char                                  message[128];

  body = JSONIFGetObject(InPacket, "body");
  packetid = JSONIFGetInt(InPacket, "packetid");

  bayindex = JSONIFGetInt(body, "bayindex");
  
  bay = FindBayByIndex(bayindex);
  sprintf(bodyString, " { \"bayindex\" : %d }", bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "reswritebay", bodyString, packetid, 0, "BayNotFound", message);
    return;
  }

  if ( !BaysRemoveBayByIndex(bayindex) ) {
    sprintf(message, "Bay %d not removed", bayindex);
    WebSocketFrameResponseSendError(InConnection, "reswritebay", bodyString, packetid, 0, "BayNotFound", message);
    return;
  }
  BaysSave(NULL);
  WebSocketFrameResponseSend(InConnection, "resbays", bodyString, packetid, 0, message);
}

/*****************************************************************************!
 * Function : HandleGetPanelTypesRequest
 *****************************************************************************/
void
HandleGetPanelTypesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  string                                body;
  string                                s;

  body = StringCopy("{ \"paneltypes\" : ");
  s = PanelTypesToJSONString(MainPanelTypes);
  body = StringConcatTo(body, s);
  FreeMemory(s);
  body = StringConcatTo(body, "}");
  packetid = JSONIFGetInt(InPacket, "packet");
  WebSocketFrameResponseSend(InConnection, "respaneltypes", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : HandleGetBayTypesRequest
 *****************************************************************************/
void
HandleGetBayTypesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  string                                body;
  string                                s;
 
  body = StringCopy("{ \"baytypes\" : ");
  s = BayTypesToJSONString(MainBayTypes);
  body = StringConcatTo(body, s);
  FreeMemory(s);
  body = StringConcatTo(body, "}");
  packetid = JSONIFGetInt(InPacket, "packet");
  WebSocketFrameResponseSend(InConnection, "resbaytypes", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : HandleGetFuseBreakerTypesRequest
 *****************************************************************************/
void
HandleGetFuseBreakerTypesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  string                                body;
  int                                   fuseBreakerSize;
  string                                fuseBreakers;
  body = StringCopy("{ \"fusebreakertypes\" : ");

  GetFileBuffer("FuseBreakerTypes.json", &fuseBreakers, &fuseBreakerSize);

  body = StringConcatTo(body, fuseBreakers);
  FreeMemory(fuseBreakers);
  body = StringConcatTo(body, "}");
  packetid = JSONIFGetInt(InPacket, "packet");
  WebSocketFrameResponseSend(InConnection, "resfusebreakertypes", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : HandleGetFuseBreakerStatesRequest
 *****************************************************************************/
void
HandleGetFuseBreakerStatesRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  string                                body;
  int                                   fuseBreakerSize;
  string                                fuseBreakers;
  body = StringCopy("{ \"fusebreakerstates\" : ");

  GetFileBuffer("FuseBreakerStates.json", &fuseBreakers, &fuseBreakerSize);

  body = StringConcatTo(body, fuseBreakers);
  free(fuseBreakers);
  body = StringConcatTo(body, "}");
  packetid = JSONIFGetInt(InPacket, "packet");
  WebSocketFrameResponseSend(InConnection, "resfusebreakerstates", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : HandleOpenRequest
 *****************************************************************************/
void
HandleOpenRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  
}

/*****************************************************************************!
 * Function : HandleCloseRequest
 *****************************************************************************/
void
HandleCloseRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  
}

/*****************************************************************************!
 * Function : HandleGetFuseBreakerRequest
 *****************************************************************************/
void
HandleGetFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   fusebreakerindex;
  FuseBreaker*                          fusebreaker;
  Panel*                                panel;
  int                                   panelid;
  string                                bodyString;
  char                                  s2[8];
  string                                s;
  int                                   packetid;
  string                                bayString;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayid;

  packetid = JSONIFGetInt(InPacket, "packetid");
  body = JSONIFGetObject(InPacket, "body");
  bayid = JSONIFGetInt(body, "bayindex");
  panelid = JSONIFGetInt(body, "panelindex");
  fusebreakerindex = JSONIFGetInt(body, "fusebreakerindex");
  bay = FindBayByIndex(bayid);
  if ( bay ) {
    panel = BayFindPanelByIndex(bay, panelid);
    if ( panel ) {
      fusebreaker = PanelFindFuseBreaker(panel, fusebreakerindex);
    }
  }
  if ( NULL == bay || NULL == panel || NULL == fusebreaker ) {
    bodyString = StringCopy("\"bayindex\" : ");
    sprintf(s2, "%d, ", bayid);
    bodyString = StringConcatTo(bodyString, s2);
    
    bodyString = StringConcatTo(bodyString, "\"panelindex\" : ");
    sprintf(s2, "%d, ", panelid);
    bodyString = StringConcatTo(bodyString, s2);
    
    bodyString = StringConcatTo(bodyString, "\"fusebreakerindex\" : ");
    sprintf(s2, "%d", fusebreakerindex);
    bodyString = StringConcatTo(bodyString, s2);

    WebSocketFrameResponseSendError(InConnection, "resfusebreaker", bodyString, packetid, 0, "FuseBreakerNotFound", "Fuse breaker not found");
    FreeMemory(bodyString);
    return;
  }
  WebSocketIFPacketIDSet(packetid + 1);
  bayString = FuseBreakerToJSONString(fusebreaker);
  
  s = StringCopy("{ ");
  s = StringConcatTo(s, "  \"bayindex\" : ");
  sprintf(s2, "%d", bayid);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");

  s = StringConcatTo(s, "  \"panelindex\" : ");
  sprintf(s2, "%d", panelid);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");
  
  s = StringConcatTo(s, "  \"fuseBreakerindex\" : ");
  sprintf(s2, "%d", fusebreakerindex);
  s = StringConcatTo(s, s2);
  s = StringConcatTo(s, ",\n");
  
  s = StringConcatTo(s, "  \"fusebreaker\" : ");
  s = StringConcatTo(s, bayString);
  s = StringConcatTo(s, "\n}");
  WebSocketFrameResponseSend(InConnection, "resfusebreaker", s, packetid, 0, "");
  FreeMemory(bayString);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : HandleWritePanelRequest
 *****************************************************************************/
void
HandleWritePanelRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  
}

/*****************************************************************************!
 * Function : HandleWriteFuseBreakerRequest
 *****************************************************************************/
void
HandleWriteFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  Panel*                                panel;
  Bay*                                  bay;
  FuseBreaker*                          fusebreaker;
  json_value*                           body;
  json_value*                           fusebreakerinfo;
  int                                   fusebreakerindex;
  int                                   panelindex;
  int                                   bayindex;
  char                                  message[128];
  char                                  bodyString[64];
  int                                   packetid;

  packetid = JSONIFGetInt(InPacket, "packetp");
  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  panelindex = JSONIFGetInt(body, "panelindex");
  fusebreakerindex = JSONIFGetInt(body, "fusebreakerindex");
  fusebreakerinfo = JSONIFGetObject(body, "fusebreaker");

  bay = FindBayByIndex(bayindex);
  sprintf(bodyString, " { \"bayindex\" : %d }", bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "reswritefusebreaker", bodyString, packetid, 0, "BatNotFound", message);
    return;
  }

  panel = BayFindPanelByIndex(bay, panelindex);
  if ( NULL == panel ) {
    sprintf(message, "Bay %d Panel %d not found", bayindex, panelindex);
    sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d }", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "reswritefusebreaker", bodyString, packetid, 0, "PanelNotFound", message);
    return;
  }

  fusebreaker = PanelFindFuseBreaker(panel, fusebreakerindex);
  sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d, \"fusebreakerindex\" : %d }", bayindex, panelindex, fusebreakerindex);
  if ( NULL == fusebreaker ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d not found", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "reswritefusebreaker", bodyString, packetid, 0, "FuseBreakerNotFound", message);
    return;
  }
  
  if ( !FuseBreakerUpdateFromJSONString(fusebreaker, fusebreakerinfo) || !FuseBreakerValueFromJSONString(fusebreaker, fusebreakerinfo) ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d not saved", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "reswritefusebreaker", bodyString, packetid, 0, "FuseBreakerNotSaved", message);
  } else {
    WebSocketFrameResponseSend(InConnection, "reswritefusebreaker", bodyString, packetid, 0, "");
    BaysSave(NULL);
    BaysSaveValues(NULL);
  }
}

/*****************************************************************************!
 * Function : WebSocketFrameSend
 *****************************************************************************/
void
WebSocketFrameSend
(struct mg_connection* InConnection, string InBuffer, uint16_t InBufferLen)
{
  mg_send_websocket_frame(InConnection, WEBSOCKET_OP_TEXT,
                          InBuffer, InBufferLen);
}

/*****************************************************************************!
 * Function : WebSocketFrameResponseSendError
 *****************************************************************************/
void
WebSocketFrameResponseSendError
(struct mg_connection* InConnection, string InResponseType,
 string InBody, int InPacketID, uint32_t InTime,
 string InResponseCode, string InResponseMessage)
{
  string                                responseString;
  char                                  s[16];
  
  responseString = StringCopy("{\n");

  responseString = StringConcatTo(responseString, "\"type\" : \"");
  responseString = StringConcatTo(responseString, InResponseType);
  responseString = StringConcatTo(responseString, "\",\n");

  responseString = StringConcatTo(responseString, "\"body\" : ");
  responseString = StringConcatTo(responseString, InBody);
  responseString = StringConcatTo(responseString, ",\n");

  responseString = StringConcatTo(responseString, "\"packettype\" : \"response\",\n");

  sprintf(s, "%d", InPacketID);
  responseString = StringConcatTo(responseString, "\"packetid\" : ");
  responseString = StringConcatTo(responseString, s);
  responseString = StringConcatTo(responseString, ",\n");

  sprintf(s, "%d", InTime);
  responseString = StringConcatTo(responseString, "\"time\" : ");
  responseString = StringConcatTo(responseString, s);
  responseString = StringConcatTo(responseString, ",\n");

  responseString = StringConcatTo(responseString, "\"responseid\" : \"");
  responseString = StringConcatTo(responseString, InResponseCode);
  responseString = StringConcatTo(responseString, "\",\n");

  responseString = StringConcatTo(responseString, "\"responsemessage\" : \"");
  responseString = StringConcatTo(responseString, InResponseMessage);
  responseString = StringConcatTo(responseString, "\"\n");
  
  responseString = StringConcatTo(responseString, "}\n");

  WebSocketFrameSend(InConnection, responseString, strlen(responseString));
  FreeMemory(responseString);
}

/*****************************************************************************!
 * Function : WebSocketFrameResponseSend
 *****************************************************************************/
void
WebSocketFrameResponseSend
(struct mg_connection* InConnection, string InResponseType,
 string InBody, int InPacketID, uint32_t InTime,
 string InResponseMessage)
{
  string                                responseString;
  char                                  s[16];

  responseString = StringCopy("{\n");

  responseString = StringConcatTo(responseString, "\"type\" : \"");
  responseString = StringConcatTo(responseString, InResponseType);
  responseString = StringConcatTo(responseString, "\",\n");

  responseString = StringConcatTo(responseString, "\"body\" : ");
  responseString = StringConcatTo(responseString, InBody);
  responseString = StringConcatTo(responseString, ",\n");

  responseString = StringConcatTo(responseString, "\"packettype\" : \"response\",\n");

  sprintf(s, "%d", InPacketID);
  responseString = StringConcatTo(responseString, "\"packetid\" : ");
  responseString = StringConcatTo(responseString, s);
  responseString = StringConcatTo(responseString, ",\n");

  sprintf(s, "%d", InTime);
  responseString = StringConcatTo(responseString, "\"time\" : ");
  responseString = StringConcatTo(responseString, s);
  responseString = StringConcatTo(responseString, ",\n");

  responseString = StringConcatTo(responseString, "\"responseid\" : \"OK\",\n");
  responseString = StringConcatTo(responseString, "\"responsemessage\" : \"");
  responseString = StringConcatTo(responseString, InResponseMessage);
  responseString = StringConcatTo(responseString, "\"\n");
  
  responseString = StringConcatTo(responseString, "}\n");

  WebSocketFrameSend(InConnection, responseString, strlen(responseString));
  FreeMemory(responseString);
}

/*****************************************************************************!
 * Function : WebSocketIFPacketIDSet
 *****************************************************************************/
void
WebSocketIFPacketIDSet
(int InPacketID)
{
  WebSocketIFPacketID = InPacketID;
}

/*****************************************************************************!
 * Function : WebSocketIFPacketIDGet
 *****************************************************************************/
int
WebSocketIFPacketIDGet
()
{
  return WebSocketIFPacketID++;
}


/*****************************************************************************!
 * Function : HandleAddBayRequest
 *****************************************************************************/
void
HandleAddBayRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  json_value*                           bayinfo;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayindex;
  char                                  bodyString[64];
  char                                  message[128];

  body = JSONIFGetObject(InPacket, "body");
  packetid = JSONIFGetInt(InPacket, "packetid");
  bayindex = JSONIFGetInt(body, "bayindex");
  bayinfo = JSONIFGetObject(body, "bayinfo");

  sprintf(bodyString, " { \"bayindex\" : %d }", bayindex);
  bay = FindBayByIndex(bayindex);
  
  if ( NULL != bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "resaddbay", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }

  bay = BayFromJSONString(bayinfo);
  if ( NULL == bay ) {
    return;
  }
  sprintf(message, "Bay %d added", bayindex);
  WebSocketFrameResponseSend(InConnection, "resaddbay", bodyString, packetid, 0, message);
  BaysSave(NULL);
}

/*****************************************************************************!
 * Function : HandleAddPanelConnectionRequest
 *****************************************************************************/
void
HandleAddPanelConnectionRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int					bayIndex, toIndex, fromIndex;
  json_value*				body;
  Panel*				panel;
  Bay*					bay;
  PanelConnection*			panelConnection;

  body = JSONIFGetObject(InPacket, "body");
  if ( NULL == body ) {
    return;
  }
  bayIndex = JSONIFGetInt(body, "bayindex");
  toIndex  = JSONIFGetInt(body, "topanel");
  fromIndex = JSONIFGetInt(body, "frompanel");

  bay = FindBayByIndex(bayIndex);
  if ( NULL == bay ) {
    return;
  }
  panel = BayFindPanelByIndex(bay, toIndex);
  if ( NULL == panel ) {
    return;
  }
  panel = BayFindPanelByIndex(bay, fromIndex);
  if ( NULL == panel ) {
    return;
  }

  panelConnection = PanelConnectionFromJSONString(body);
  BayAppendPanelConnection(bay, panelConnection);  
  BaysSave(NULL);
}

/*****************************************************************************!
 * Function : HandleAddPanelRequest
 *****************************************************************************/
void
HandleAddPanelRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  Panel*                                panel;
  Bay*                                  bay;
  json_value*                           body;
  int                                   panelindex;
  int                                   bayindex;
  char                                  bodyString[64];
  char                                  message[128];


  packetid = JSONIFGetInt(InPacket, "packetid");
  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  panelindex = JSONIFGetInt(body, "panelindex");

  sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d }", bayindex, panelindex);
  
  bay = FindBayByIndex(bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "resaddbay", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }

  if ( BayPanelSlotOccupied(bay, panelindex) ) {
    sprintf(message, "Bay %d Panel %d not found", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resaddbay", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }
  
  panel = PanelFromJSONString(body);
  if ( NULL == panel ) {
    sprintf(message, "Bay %d Panel %d not created", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resaddbay", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }
  BayAddPanel(bay, panel);
  sprintf(message, "Bay %d Panel %d created", bayindex, panelindex);
  WebSocketFrameResponseSend(InConnection, "resaddpanel", bodyString, packetid, 0, message);
  BaysSave(NULL);
  BaysSaveValues(NULL);
}

/*****************************************************************************!
 * Function : HandleAddFuseBreakerRequest
 *****************************************************************************/
void
HandleAddFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  Panel*                                panel;
  Bay*                                  bay;
  FuseBreaker*                          fusebreaker;
  json_value*                           body;
  json_value*                           fusebreakerinfo;
  int                                   fusebreakerindex;
  int                                   panelindex;
  int                                   bayindex;
  char                                  bodyString[64];
  char                                  message[128];

  packetid = JSONIFGetInt(InPacket, "packetid");
  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  panelindex = JSONIFGetInt(body, "panelindex");
  fusebreakerindex = JSONIFGetInt(body, "fusebreakerindex");
  fusebreakerinfo = JSONIFGetObject(body, "fusebreaker");
  sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d }", bayindex, panelindex);

  bay = FindBayByIndex(bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "resaddfusebreaker", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }

  panel = BayFindPanelByIndex(bay, panelindex);
  if ( NULL == panel ) {
    sprintf(message, "Bay %d Panel %d not found", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resaddfusebreaker", bodyString, packetid, 0, "PanelNotFound", message); 
    return;
  }

  if ( PanelFuseBreakerSlotOccupied(panel, fusebreakerindex) ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d already occupied", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "resaddfusebreaker", bodyString, packetid, 0, "FuseBreakerExists", message); 
    return;
  }

  fusebreaker = FuseBreakerFromJSONString(fusebreakerinfo);
  if ( NULL == fusebreaker ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d not added", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "resaddfusebreaker", bodyString, packetid, 0, "FuseBreakerNotAdded", message); 
    return;
  }
  PanelAddFuseBreaker(panel, fusebreaker);
  sprintf(message, "Bay %d Panel %d Fuse/Breaker %d added", bayindex, panelindex, fusebreakerindex);
  WebSocketFrameResponseSend(InConnection, "resaddfusebreaker", bodyString, packetid, 0, message); 
  BaysSave(NULL);
}

/*****************************************************************************!
 * Function : HandleRemovePanelConnectRequest
 *****************************************************************************/
void
HandleRemovePanelConnectRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   bayindex, fromindex, toindex;
  int                                   packetid;
  json_value*                           body;
  Bay*                                  bay;
  PanelConnection*                      panelconnection;
  char					message[128];
  char					bodystring[128];

  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  fromindex = JSONIFGetInt(body, "frompanel");
  toindex   = JSONIFGetInt(body, "topanel");
  packetid = JSONIFGetInt(InPacket, "packetid");
  sprintf(bodystring, " { \"bayindex\" : %d, \"toindex\" : %d, \"fromindex\" : %d }", bayindex, toindex, fromindex);
 
  bay = FindBayByIndex(bayindex);
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSend(InConnection, "resaddfusebreaker", bodystring, packetid, 0, message); 
    return;
  }

  panelconnection = BayFindPanelConnectionByIndices(bay, fromindex, toindex);
  if ( NULL == panelconnection ) {
    sprintf(message, "A panel connection from panel %d to panel %d in bay %d not found", toindex, fromindex, bayindex);
    WebSocketFrameResponseSend(InConnection, "resaddfusebreaker", bodystring, packetid, 0, message); 
    return;
  }
  sprintf(message, "Bay %d panel connection from panel %d to panel %d added", bayindex, fromindex, toindex);
  BayRemovePanelConnection(bay, panelconnection); 
  WebSocketFrameResponseSend(InConnection, "resremovepanelconnect", bodystring, packetid, 0, message); 
  BaysSave(NULL);
  BaysSaveValues(NULL);
}

/*****************************************************************************!
 * Function : HandleRemovePanelRequest
 *****************************************************************************/
void
HandleRemovePanelRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  Panel*                                panel;
  int                                   panelindex;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayindex;
  char                                  bodyString[64];
  char                                  message[128];
  PanelConnection*			connection;

  packetid = JSONIFGetInt(InPacket, "packetid");
  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  panelindex = JSONIFGetInt(body, "panelindex");

  sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d }", bayindex, panelindex);
  
  bay = FindBayByIndex(bayindex);
  
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "resremovepanel", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }

  panel = BayFindPanelByIndex(bay, panelindex);
  if ( NULL == panel ) {
    sprintf(message, "Bay %d Panel %d not found", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resremovepanel", bodyString, packetid, 0, "PanelNotFound", message); 
    return;
  }

  if ( ! BayRemovePanelByIndex(bay, panelindex) ) {
    sprintf(message, "Bay %d Panel %d not removed", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resremovepanel", bodyString, packetid, 0, "PanelNotRemoved", message); 
  }
  sprintf(message, "Bay %d Panel %d removed", bayindex, panelindex);
  WebSocketFrameResponseSend(InConnection, "resremovepanel", bodyString, packetid, 0, message); 
  for ( connection = bay->panelConnections; connection; connection = connection->next ) {
    if ( connection->toPanel == panelindex || connection->fromPanel == panelindex ) {
      WebSocketIFSendRemovePanelConnect(connection);
    }
  }
  BayRemovePanelConnectionsByIndex(bay, panelindex);
  BaysSave(NULL);
}

/*****************************************************************************!
 * Function : HandleRemoveFuseBreakerRequest
 *****************************************************************************/
void
HandleRemoveFuseBreakerRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  FuseBreaker*                          fusebreaker;
  int                                   fusebreakerindex;
  Panel*                                panel;
  int                                   panelindex;
  Bay*                                  bay;
  json_value*                           body;
  int                                   bayindex;
  char                                  bodyString[64];
  char                                  message[128];

  packetid = JSONIFGetInt(InPacket, "packetid");
  body = JSONIFGetObject(InPacket, "body");
  bayindex = JSONIFGetInt(body, "bayindex");
  panelindex = JSONIFGetInt(body, "panelindex");
  fusebreakerindex = JSONIFGetInt(body, "fusebreakerindex");

  sprintf(bodyString, " { \"bayindex\" : %d, \"panelindex\" : %d, \"fusebreakerindex\" : %d  }",
          bayindex, panelindex, fusebreakerindex);
  
  bay = FindBayByIndex(bayindex);
  
  if ( NULL == bay ) {
    sprintf(message, "Bay %d not found", bayindex);
    WebSocketFrameResponseSendError(InConnection, "resremovefusebreaker", bodyString, packetid, 0, "BayNotFound", message); 
    return;
  }

  panel = BayFindPanelByIndex(bay, panelindex);
  if ( NULL == panel ) {
    sprintf(message, "Bay %d Panel %d not found", bayindex, panelindex);
    WebSocketFrameResponseSendError(InConnection, "resremovefusebreaker", bodyString, packetid, 0, "PanelNotFound", message); 
    return;
  }

  fusebreaker = PanelFindFuseBreaker(panel, fusebreakerindex);
  if ( NULL == fusebreaker ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d not found", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "resremovefusebreaker", bodyString, packetid, 0, "FuseBreakerNotFound", message); 
    return;
  }
  
  if ( ! PanelRemoveFuseBreakerByIndex(panel, fusebreakerindex) ) {
    sprintf(message, "Bay %d Panel %d Fuse/Breaker %d not removed", bayindex, panelindex, fusebreakerindex);
    WebSocketFrameResponseSendError(InConnection, "resremovefusebreaker", bodyString, packetid, 0, "FuseBreakerNotFound", message);
    return;
  }
  
  sprintf(message, "Bay %d Panel %d Fuse/Breaker %d removed", bayindex, panelindex, fusebreakerindex);
  WebSocketFrameResponseSend(InConnection, "resremovefusebreaker", bodyString, packetid, 0, message);
  BaysSave(NULL);
}

/*****************************************************************************!
 * Function : WebSocketIFSendFuseBreaker
 *****************************************************************************/
void
WebSocketIFSendFuseBreaker
(FuseBreaker* InFuseBreaker)
{

  string				sendString;
  int					packetid;
  char					s2[16];
  string				fuseString; 
  
  if ( NULL == InFuseBreaker ) {
    return;
  }

  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"addfusebreaker\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d", InFuseBreaker->panel->bay->index);
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");

  sendString = StringConcatTo(sendString, "    \"panelindex\" : ");
  sprintf(s2, "%d", InFuseBreaker->panel->index);
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");

  sendString = StringConcatTo(sendString, "    \"fusebreakerindex\" : ");
  sprintf(s2, "%d", InFuseBreaker->index);
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n   \"fuse\" : ");
  
  fuseString = FuseBreakerToJSONString(InFuseBreaker);
  sendString = StringConcatTo(sendString, fuseString);
  sendString = StringConcatTo(sendString, "\n");
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);

  FreeMemory(sendString);
  FreeMemory(fuseString); 
}

/*****************************************************************************!
 * Function : WebSocketIFSendPanelConnect
 *****************************************************************************/
void
WebSocketIFSendPanelConnect
(PanelConnection* InConnection)
{
  string				sendString;
  int					packetid;
  char					s2[16];
 
  if ( NULL == InConnection ) {
    return;
  }

  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"addpanelconnection\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d", InConnection->bay->index);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, ",\n    \"frompanel\" : ");
  sprintf(s2, "%d", InConnection->fromPanel);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, ",\n    \"topanel\" : ");
  sprintf(s2, "%d", InConnection->toPanel);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);

  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : WebSocketIFSendPanel
 *****************************************************************************/
void
WebSocketIFSendPanel
(Panel* InPanel)
{

  string				sendString;
  int					packetid;
  char					s2[16];
  string				panelString; 
  
  if ( NULL == InPanel ) {
    return;
  }

  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"addpanel\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d", InPanel->bay->index);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, ",\n    \"panelindex\" : ");
  sprintf(s2, "%d", InPanel->index);
  sendString = StringConcatTo(sendString, s2);


  sendString = StringConcatTo(sendString, ",\n  \"panel\" : ");
  
  panelString = PanelToJSONString(InPanel);
  sendString = StringConcatTo(sendString, panelString);
  sendString = StringConcatTo(sendString, "\n");
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);

  FreeMemory(sendString);
  FreeMemory(panelString); 
}


/*****************************************************************************!
 * Function : WebSocketIFSendBay
 *****************************************************************************/
void
WebSocketIFSendBay
(Bay* InBay)
{
  string				sendString;
  int					packetid;
  char					s2[16];
  string				bayString; 
  
  if ( NULL == InBay ) {
    return;
  }

  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"addbay\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d", InBay->index);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, ",\n  \"bay\" : ");
  
  bayString = BayToJSONString(InBay);
  sendString = StringConcatTo(sendString, bayString);
  sendString = StringConcatTo(sendString, "\n");
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);

  FreeMemory(sendString);
  FreeMemory(bayString); 
}

/*****************************************************************************!
 * Function : WebSocketIFBroadcastPush
 *****************************************************************************/
void
WebSocketIFBroadcastPush
(string InString)
{
  int						i, n;
  WebConnection*				connection;

  n = WebConnectionListCount(mainWebConnections);
  for (i = 0; i < n; i++) {
    connection = WebConnectionListGetByIndex(mainWebConnections, i);
    WebSocketFrameSend(connection->connection, InString, strlen(InString));
  } 
}

/*****************************************************************************!
 * Function : WebSocketIFRemoveBay
 *****************************************************************************/
void
WebSocketIFRemoveBay
(int InBayIndex)
{
  string				sendString;
  int					packetid;
  char					s2[16];
 
  if ( InBayIndex < 1 || InBayIndex > BAY_MAX_INDEX ) {
    return;
  } 
  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"removebay\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d", InBayIndex);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "\n  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);
  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : WebSocketIFRemovePanel
 *****************************************************************************/
void
WebSocketIFRemovePanel
(int InBayIndex, int InPanelIndex)
{
  string				sendString;
  int					packetid;
  char					s2[16];
 
  if ( InBayIndex < 1 || InBayIndex > BAY_MAX_INDEX ) {
    return;
  } 
  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"removepanel\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d,\n", InBayIndex);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"panelindex\" : ");
  sprintf(s2, "%d\n", InPanelIndex);
  sendString = StringConcatTo(sendString, s2);
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);
  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : WebSocketIFRemoveFuseBreaker 
 *****************************************************************************/
void
WebSocketIFRemoveFuseBreaker
(int InBayIndex, int InPanelIndex, int InFuseBreakerIndex)
{
  string				sendString;
  int					packetid;
  char					s2[16];
 
  if ( InBayIndex < 1 || InBayIndex > BAY_MAX_INDEX ) {
    return;
  } 
  packetid = WebSocketIFPacketIDGet();

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"removefusebreaker\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d,\n", InBayIndex);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"panelindex\" : ");
  sprintf(s2, "%d,\n", InPanelIndex);
  sendString = StringConcatTo(sendString, s2);
 
  sendString = StringConcatTo(sendString, "    \"fusebreakerindex\" : ");
  sprintf(s2, "%d\n", InFuseBreakerIndex);
  sendString = StringConcatTo(sendString, s2);
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);
  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : WebSocketIFAddPanelConnection
 *****************************************************************************/
void
WebSocketIFAddPanelConnection
(int InBayIndex, int InFromIndex, int InToIndex)
{
  Bay*					bay;
  int 					packetid;
  char					s2[16];
  string				sendString;

  packetid = WebSocketIFPacketIDGet();

  if ( InFromIndex == 0 || InToIndex == 0 ) {
    return;
  }
  if ( InBayIndex < 1 || InBayIndex > BAY_MAX_INDEX ) {
    return;
  }
  bay = FindBayByIndex(InBayIndex);
  if ( NULL == bay ) {
    return;
  }
  if ( InFromIndex < 1 || InFromIndex > bay->panelCount ) {
    return;
  }

  if ( InToIndex < 1 || InToIndex > bay->panelCount ) {
    return;
  }
 
  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"addpanelconnection\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d,\n", InBayIndex);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"frompanel\" : ");
  sprintf(s2, "%d,\n", InFromIndex);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"topanel\" : ");
  sprintf(s2, "%d\n", InToIndex);
  sendString = StringConcatTo(sendString, s2);
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);
  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : WebSocketIFSendRemovePanelConnect
 *****************************************************************************/
void
WebSocketIFSendRemovePanelConnect
(PanelConnection* InConnection)
{
  int 					packetid;
  char					s2[16];
  string				sendString;

  packetid = WebSocketIFPacketIDGet();

  if ( NULL == InConnection ) {
    return;
  }

  sprintf(s2, "%d", packetid);
  sendString = StringCopy("{ \n");
  sendString = StringConcatTo(sendString, "  \"packettype\" : \"push\",\n");
  sendString = StringConcatTo(sendString, "  \"packetid\" : ");
  sendString = StringConcatTo(sendString, s2);
  sendString = StringConcatTo(sendString, ",\n");
  sendString = StringConcatTo(sendString, "  \"time\" : 0,\n");
  sendString = StringConcatTo(sendString, "  \"type\" : \"removepanelconnection\",\n");
  sendString = StringConcatTo(sendString, "  \"body\" : {\n");

  sendString = StringConcatTo(sendString, "    \"bayindex\" : ");
  sprintf(s2, "%d,\n", InConnection->bay->index);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"frompanel\" : ");
  sprintf(s2, "%d,\n", InConnection->fromPanel);
  sendString = StringConcatTo(sendString, s2);

  sendString = StringConcatTo(sendString, "    \"topanel\" : ");
  sprintf(s2, "%d\n", InConnection->toPanel);
  sendString = StringConcatTo(sendString, s2);
 
  sendString = StringConcatTo(sendString, "  }\n");
  sendString = StringConcatTo(sendString, "}\n");
  WebSocketIFBroadcastPush(sendString);
  FreeMemory(sendString);
}

/*****************************************************************************!
 * Function : HandleGetDeviceDefsRequest
 *****************************************************************************/
void
HandleGetDeviceDefsRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  int                                   packetid;
  string                                body;
  string                                deviceDefs;

  deviceDefs = DeviceDefListToJSON(mainDeviceDefs);
  body = StringCopy("{ \"devicedefs\" : ");
  body = StringConcatTo(body, deviceDefs);
  FreeMemory(deviceDefs);
  body = StringConcatTo(body, "}");
  packetid = JSONIFGetInt(InPacket, "packet");
  WebSocketFrameResponseSend(InConnection, "resdevicedefs", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : HandleGetDeviceRegsRequest
 *****************************************************************************/
void
HandleGetDeviceRegsRequest
(struct mg_connection* InConnection, json_value* InPacket)
{
  string				deviceName;
  DeviceDef*				deviceDef;
  string				body;
  string				regDefs;
  int					packetid;

  packetid = JSONIFGetInt(InPacket, "packetid");
  deviceName = JSONIFGetString(InPacket, "devicename");
  deviceDef = FindDeviceDef(mainDeviceDefs, deviceName);
  if ( deviceDef == NULL ) {
    FreeMemory(deviceName);
    return;
  }
  body = StringCopy("{ \"devicename\" : \"");
  body = StringConcatTo(body, deviceName); 
  FreeMemory(deviceName);
  body = StringConcatTo(body, "\",\n");
  body = StringConcatTo(body, " \"registers\" : \n");
  regDefs = DeviceRegDefListToJSON(deviceDef->regDefs);
  body = StringConcatTo(body, regDefs);
  FreeMemory(regDefs);
  body = StringConcatTo(body, "}");
  WebSocketFrameResponseSend(InConnection, "resdeviceregs", body, packetid, 0, "");
  FreeMemory(body);
}

/*****************************************************************************!
 * Function : WebSocketIFSetPort
 *****************************************************************************/
void
WebSocketIFSetPort
(string InPortNumber)
{
  if ( InPortNumber == NULL ) {
    return;
  }

  if ( s_websocket_port ) {
    FreeMemory(s_websocket_port);
  }
  s_websocket_port = StringCopy(InPortNumber);
}

/*****************************************************************************!
 * Function : WebSocketIFCreateInfoScript
 *****************************************************************************/
void
WebSocketIFCreateInfoScript
()
{
  string                                address;
  int                                   fd;
  struct                                ifreq ifr;
  FILE*                                 file;
  struct sockaddr_in*                   sa;
  int                                   i, j;
  int					interfacesSize;
  char                                  b;
  char*					interfaces[] = { "eth0", "wlan0" };
  bool					found;
  string				interface;

  found = false;
  interfacesSize = sizeof(interfaces) / sizeof(char*);

  printf("Create HTML WebSocket Script ");
  fflush(stdout);
  //! We only want to do this when we have a value 192. address
  //  So we loop until we do or we eventually give up
  for (i = 0; i < WEBSOCKET_IF_MAX_ADDRESS_TRIES && !found; i++ ) { 
    for ( j = 0 ; j < interfacesSize && !found ; j++ ) {
      fd = socket(AF_INET, SOCK_DGRAM, 0);
      printf(".");
      fflush(stdout); 
      /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;

      interface = interfaces[j];    
      /* I want IP address attached to "eth0" */
      strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    
      ioctl(fd, SIOCGIFADDR, &ifr);
      close(fd);
      sa = (struct sockaddr_in*)&ifr.ifr_addr;
      b = (char)(sa->sin_addr.s_addr & 0xFF);
  
      //! Check that we have a at least a C Address
      if ( b & 0xC0 ) {
        address = StringCopy(inet_ntoa(sa->sin_addr));
        file = fopen("www/websocketinfo.js", "wb");
 	if ( NULL == file ) {
	  return;
 	}
        fprintf(file, "var WebSocketIFAddress = \"%s\";\n", address);
        fprintf(file, "var WebSocketIFPort = \"%s\";\n", s_websocket_port);
        fclose(file);
		WebSocketIFAddress = StringCopy(address);
        FreeMemory(address);
		found = true;
      }
      if ( !found ) {
        //! We don't so sleep and try again
        sleep(WEBSOCKET_IF_ADDRESS_WAIT_PERIOD);
      }
    }
  }
  printf(" %screated\n", found ? "" : "not ");
}

