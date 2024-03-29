/*****************************************************************************
 * FILE NAME    : HTTPServer.h
 * DATE         : December 01 2020
 * COPYRIGHT    : Copyright (C) 2020 by Gregory R Saltis
 *****************************************************************************/
#ifndef _httpserver_h_
#define _httpserver_h_

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

/*****************************************************************************!
 * Exported Macros
 *****************************************************************************/

/*****************************************************************************!
 * Exported Data
 *****************************************************************************/

/*****************************************************************************!
 * Exported Functions
 *****************************************************************************/
pthread_t
HTTPServerGetThreadID
();

void*
HTTPServerThread
(void* InParameter);

void
HTTPServerInitialize
();

void
HTTPServerStart
();

#endif /* _httpserver_h_*/
