/*****************************************************************************
 * FILE NAME    : scripts.js
 * DATE         : December 02 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/

/*****************************************************************************!
 * Local Variables
 *****************************************************************************/
var
SetMessageTimeoutID = 0;

var
WebSocketIFConnection = null;

var
WebSocketIFID = 0;

/*****************************************************************************!
 * Function : CBSystemInitialize
 *****************************************************************************/
function
CBSystemInitialize
()
{
  ClearMainInput();
  WebSocketIFInitialize();
  SetMessageError("Greetings Earthlings");
}

/*****************************************************************************!
 * Function : ClearMainInput
 *****************************************************************************/
function
ClearMainInput
()
{
  var                                   inputArea;

  inputArea = document.getElementById("MainInput");
  inputArea.value = "";
}

/*****************************************************************************!
 * Function : SetMessageNormal
 *****************************************************************************/
function
SetMessageNormal
(InMessage)
{
  SetMessage(InMessage, "#000080");
}

/*****************************************************************************!
 * Function : SetMessageError
 *****************************************************************************/
function
SetMessageError
(InMessage)
{
  SetMessage(InMessage, "#C00000");
}

/*****************************************************************************!
 * Function : SetMessage
 *****************************************************************************/
function
SetMessage
(InMessage, InColor)
{
  var                                   inputArea;

  inputArea = document.getElementById("MessageArea");
  inputArea.innerHTML = InMessage;
  inputArea.style.color = InColor;
  SetMessageTimeoutID = setInterval(SetMessageClear, 10000);
}

/*****************************************************************************!
 * Function : SetMessageClear
 *****************************************************************************/
function
SetMessageClear
()
{
  var                                   inputArea;

  inputArea = document.getElementById("MessageArea");
  inputArea.innerHTML = "";
  SetMessageTimeoutID = 0;
}

/*****************************************************************************!
 * Function : WebSocketIFInitialize
 *****************************************************************************/
function
WebSocketIFInitialize
()
{
  var                                   hostaddress;

  hostaddress = "ws://" + WebSocketIFAddress + ":" + WebSocketIFPort;

  WebSocketIFConnection = new WebSocket(hostaddress);
  WebSocketIFConnection.onopen = function() {
    SetMessage("Connected to " + WebSocketIFAddress + ":" + WebSocketIFPort);
    WebSocketIFSendSimpleRequest("init");
    HideBlocker();
  }

  WebSocketIFConnection.onerror = function() {

  }

  WebSocketIFConnection.onclose = function() {
    ShowBlocker();
    SetMessage("Disconnected from " + WebSocketIFAddress + ":" + WebSocketIFPort);
  }

  WebSocketIFConnection.onmessage = function(InEvent) {
    WebSocketIFHandlePacket(InEvent.data);
  }
}

/*****************************************************************************!
 * Function : WebSocketIFHandlePacket
 *****************************************************************************/
function
WebSocketIFHandlePacket
(InData)
{
  var                                   packet;
  var                                   packettype;
  
  packet = JSON.parse(InData);
  n = packet.packetid;
  if ( n > 0 ) {
    WebSocketIFID = n;
  }

  packettype = packet.packettype;
  if ( packettype == "request" ) {
    WebSocketIFHandleRequest(packet);
  }
}

/*****************************************************************************!
 * Function : WebSocketIFHandleRequest
 *****************************************************************************/
function
WebSocketIFHandleRequest
(InPacket)
{
  if ( InPacket.type == "create" ) {
    WebSocketIFHandleCreate(InPacket.body);
  }
}

/*****************************************************************************!
 * Function : WebSocketIFHandleCreate
 *****************************************************************************/
function
WebSocketIFHandleCreate
(InBody)
{

  console.log(InBody);
  if ( InBody.type == "box" ) {
    WebSocketCreateBox(InBody);
  }
}

/*****************************************************************************!
 * Function : WebSocketCreateBox
 *****************************************************************************/
function
WebSocketCreateBox
(InBody)
{
  var                                   value;
  var                                   values;
  var                                   mainarea;

  values = InBody.values;
  mainarea = document.getElementById("MainDrawArea");
  div = document.createElement("div");

  for ( value in values ) {
    div.style[value] = values[value];
  }
  mainarea.appendChild(div);
}

/*****************************************************************************!
 * Function : WebSocketIFSendSimpleRequest
 *****************************************************************************/
function
WebSocketIFSendSimpleRequest
(InRequest)
{
  var                                   d;
  var                                   request;

  request = {};

  d = new Date();
  
  request.packettype = "request";
  request.packetid = WebSocketIFGetNextID();
  request.time = d.getTime();
  request.type = InRequest;
  request.body = "";

  WebSocketIFSendGeneralRequest(request);
}

/*****************************************************************************!
 * Function : WebSocketIFSendGeneralRequest
 *****************************************************************************/
function
WebSocketIFSendGeneralRequest
(InRequest)
{
  if ( WebSocketIFConnection ) {
    WebSocketIFConnection.send(JSON.stringify(InRequest));
  }
}

/*****************************************************************************!
 * Function : HideBlocker
 *****************************************************************************/
function
HideBlocker
()
{
  var                                   blocker;

  blocker = document.getElementById("MainBlocker");
  blocker.style.visibility = "hidden";
}

/*****************************************************************************!
 * Function : ShowBlocker
 *****************************************************************************/
function
ShowBlocker
()
{
  var                                   blocker;

  blocker = document.getElementById("MainBlocker");
  blocker.style.visibility = "visible";
}

/*****************************************************************************!
 * Function : WebSocketIFGetNextID
 *****************************************************************************/
function
WebSocketIFGetNextID
()
{
  return ++WebSocketIFID;
}
