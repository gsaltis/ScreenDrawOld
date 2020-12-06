/*****************************************************************************
 * FILE NAME    : WebSocketServer.c
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
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

/*****************************************************************************!
 * Local Headers
 *****************************************************************************/
#include "WebSocketServer.h"
#include "GeneralUtilities/String.h"
#include "GeneralUtilities/ANSIColors.h"
#include "GeneralUtilities/MemoryManager.h"
#include "RPiBaseModules/json.h"
#include "JSONIF.h"
#include "JSONOut.h"
#include "HTTPServer.h"
#include "WebConnection.h"

/*****************************************************************************!
 * Local Macros
 *****************************************************************************/

/*****************************************************************************!
 * Local Functions
 *****************************************************************************/
void
WebSocketHandlePacket
(struct mg_connection* InConnection, string InData, int InDataSize);

void
WebSocketHandleRequest
(struct mg_connection* InConnection, json_value* InValue);

void
WebSocketHandleInit
(struct mg_connection* InConnection, json_value* InValue);

void
WebSocketFrameSend
(struct mg_connection* InConnection, string InBuffer, uint16_t InBufferLen);

/*****************************************************************************!
 * Local Data
 *****************************************************************************/
static WebConnectionList*
WebSocketConnections;

static int
WebSocketServerPollPeriod = 20;

static struct mg_serve_http_opts
WebSocketServerOptions;

static struct mg_connection*
WebSocketConnection;

static struct mg_mgr
WebSocketManager;

static string
WebSocketWWWDirectoryDefault = "www";

static string
WebSocketWWWDirectory = NULL;

static string
WebSocketPortAddressDefault = "8002";

static string
WebSocketPortAddress = NULL;

static pthread_t
WebSocketServerThreadID;

/*****************************************************************************!
 * Function : WebSocketServerInitialize
 *****************************************************************************/
void
WebSocketServerInitialize
()
{
  WebSocketConnections = WebConnectionListCreate();
  WebSocketPortAddress = WebSocketPortAddressDefault;
  WebSocketWWWDirectory = WebSocketWWWDirectoryDefault;
}

/*****************************************************************************!
 * Function : WebSocketServerStart
 *****************************************************************************/
void
WebSocketServerStart
()
{
  if ( pthread_create(&WebSocketServerThreadID, NULL, WebSocketServerThread, NULL) ) {
    fprintf(stderr, "Could not create WebSocket Thread\n");
  }
}

/*****************************************************************************!
 * Function : WebSocketServerThread
 *****************************************************************************/
void*
WebSocketServerThread
(void* InParameters)
{
  mg_mgr_init(&WebSocketManager, NULL);
  WebSocketConnection = mg_bind(&WebSocketManager, WebSocketPortAddress, WebSocketServerEventHandler);
  if ( NULL == WebSocketConnection ) {
    fprintf(stderr, "%sFailed to create WebSocket server%s\n", ColorBrightRed, ColorReset);
    exit(EXIT_FAILURE);
  }
  mg_set_protocol_http_websocket(WebSocketConnection);
  WebSocketServerOptions.document_root = WebSocketWWWDirectory;
  WebSocketServerOptions.enable_directory_listing = "yes";
  
  printf("%sWebSocket Server started%s\n", ColorBrightGreen, ColorReset);
  HTTPServerStart();
  while ( true ) {
    mg_mgr_poll(&WebSocketManager, WebSocketServerPollPeriod);
  }
}

/*****************************************************************************!
 * Function : WebSocketServerGetThreadID
 *****************************************************************************/
pthread_t
WebSocketServerGetThreadID
()
{
  return WebSocketServerThreadID;
}

/*****************************************************************************!
 * Function : WebSocketServerEventHandler
 *****************************************************************************/
void
WebSocketServerEventHandler
(struct mg_connection* InConnection, int InEvent, void* InParameter)
{
  WebConnection*                        con;
  struct websocket_message*             message;
  
  if ( InEvent == 0 ) {
    return;
  }

  switch (InEvent) {
    case MG_EV_CLOSE : {
      con = WebConnectionListFind(WebSocketConnections, InConnection);
      if ( con ) {
        WebConnectionListRemove(WebSocketConnections, con);
      }
      break;
    }

    case MG_EV_WEBSOCKET_FRAME : {
      if ( NULL == WebConnectionListFind(WebSocketConnections, InConnection) ) {
        WebConnectionListAppend(WebSocketConnections, WebConnectionCreate(InConnection));
      }
      message = (struct websocket_message*)InParameter;
      WebSocketHandlePacket(InConnection, (string)message->data, message->size);
      break;
    }
  }
}

/*****************************************************************************!
 * Function : WebSocketServerCreateInfoScript
 *****************************************************************************/
void
WebSocketServerCreateInfoScript
()
{
  struct ifaddrs*                       addresses;
  struct ifaddrs*                       tempAddress;
  
  string                                address;
  int                                   fd;
  struct                                ifreq ifr;
  FILE*                                 file;
  struct sockaddr_in*                   sa;
  char                                  b;
  bool                                  found;
  string                                interface;

  found = false;

  getifaddrs(&addresses);
  
  printf("%sWebSocket Script ", ColorBrightGreen);
  fflush(stdout);
  //! We only want to do this when we have a value 192. address
  //  So we loop until we do or we eventually give up
  for (int i = 0; i < WEBSOCKET_SERVER_MAX_ADDRESS_TRIES && !found; i++ ) { 
    for ( tempAddress = addresses ; tempAddress ; tempAddress = tempAddress->ifa_next ) {
      if ( ! (tempAddress->ifa_addr && tempAddress->ifa_addr->sa_family == AF_PACKET) ) {
        continue;
      }
      fd = socket(AF_INET, SOCK_DGRAM, 0);
      printf(".");
      fflush(stdout); 
      /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;

      interface = tempAddress->ifa_name;    
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
        fprintf(file, "var WebSocketIFPort = \"%s\";\n", WebSocketPortAddress);
        fclose(file);
        FreeMemory(address);
        found = true;
      }
      if ( !found ) {
        //! We don't so sleep and try again
        sleep(WEBSOCKET_SERVER_ADDRESS_WAIT_PERIOD);
      }
    }
  }
  freeifaddrs(addresses);
  printf(" %screated%s\n", found ? "" : "not ", ColorReset);
}

/*****************************************************************************!
 * Function : WebSocketHandlePacket
 *****************************************************************************/
void
WebSocketHandlePacket
(struct mg_connection* InConnection, string InData, int InDataSize)
{
  string                                packetType;
  string                                s;
  json_value*                           jsonDoc;

  
  s = StringNCopy(InData, InDataSize);
  jsonDoc = json_parse((const json_char*)InData, (size_t)InDataSize);

  packetType = JSONIFGetString(jsonDoc, "packettype");

  if ( StringEqual(packetType, "request") ) {
    WebSocketHandleRequest(InConnection, jsonDoc);
  }
  json_value_free(jsonDoc);
  FreeMemory(packetType);
  FreeMemory(s);
}

/*****************************************************************************!
 * Function : WebSocketHandleRequest
 *****************************************************************************/
void
WebSocketHandleRequest
(struct mg_connection* InConnection, json_value* InJSONDoc)
{
  string                                type;

  type = JSONIFGetString(InJSONDoc, "type");

  if ( StringEqual(type, "init") ) {
    WebSocketHandleInit(InConnection, InJSONDoc);
  }

  FreeMemory(type);
}

/*****************************************************************************!
 * Function : WebSocketHandleInit
 *****************************************************************************/
void
WebSocketHandleInit
(struct mg_connection* InConnection, json_value* InJSONDoc)
{
  JSONOut*                              body;
  string                                s;
  JSONOut*                              object;

  object = JSONOutCreateObject(NULL);

  body = JSONOutCreateObject("body");
  JSONOutObjectAddObject(body, JSONOutCreateString("name", "greg"));
  JSONOutObjectAddObjects(object,
                          JSONOutCreateString("packettype", "response"),
                          JSONOutCreateInt("packetid", JSONIFGetInt(InJSONDoc, "packetid")),
                          JSONOutCreateInt("time", (int)time(NULL)),
                          JSONOutCreateString("type", "init"),
                          JSONOutCreateString("status", "OK"),
                          body,
                          NULL);
  
  s = JSONOutToString(object, 0);
  WebSocketFrameSend(InConnection, s, strlen(s));
  FreeMemory(s);
  JSONOutDestroy(object);
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
 * Function : WebSocketJSONSendAll
 *****************************************************************************/
void
WebSocketJSONSendAll
(JSONOut* InJSON)
{
  
}

/*****************************************************************************!
 * Function : WebSocketSendCreate
 *****************************************************************************/
void
WebSocketSendCreate
(JSONOut* InJSON)
{
  string                                s;
  JSONOut*                              object;

  object = JSONOutCreateObject(NULL);

  JSONOutObjectAddObjects(object,
                          JSONOutCreateString("packettype", "request"),
                          JSONOutCreateInt("packetid", 1),
                          JSONOutCreateInt("time", (int)time(NULL)),
                          JSONOutCreateString("type", "create"),
                          JSONOutCreateString("status", "OK"),
                          InJSON,                          
                          NULL);
  
  s = JSONOutToString(object, 0);
  printf("%s %d : %s\n", __FILE__, __LINE__, s);
  // WebSocketFrameSend(, s, strlen(s));
  FreeMemory(s);
  JSONOutDestroy(object);  
}

/*****************************************************************************!
 * Function : WebSocketDisplayConnections
 *****************************************************************************/
void
WebSocketDisplayConnections
()
{
  WebConnection*                        connection;
  struct tm*                            ts;
  
  printf("%sACTIVE WEB CONNECTIONS%s\n", ColorBrightGreen, ColorReset);
  printf("%s                       ADDRESS                TIME%s\n", ColorBoldYellowReverse, ColorReset);
  for ( connection = WebSocketConnections->first ; connection ; connection = connection->next ) {
    ts = localtime(&connection->lastReceiveTime);

    printf("%s  %22s:%-5d%s %s%02d/%02d/%04d %02d:%02d:%02d%s\n", 
           ColorBoldBlue, 
           inet_ntoa(connection->connection->sa.sin.sin_addr), 
	   connection->connection->sa.sin.sin_port,
	   ColorReset,
           ColorBoldCyan, 
           ts->tm_mon + 1, ts->tm_mday, ts->tm_year + 1900,
           ts->tm_hour, ts->tm_min, ts->tm_sec, ColorReset);
  }
}
