/* 
Copyright (c) 2015, David P. Discher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _UUID_WRAPPER_H_
#define _UUID_WRAPPER_H_

#ifdef FREEBSD
#include <uuid.h>
//typedef struct uuid_t uuid_cp;
#elif defined(ESP8266)
#include "uuid_local.h"
#elif defined(IOS_APP)
#include <stdlib.h>
#include <uuid/uuid.h>
#endif 

#ifndef uuid_cp
#ifdef IOS_APP
#define uuid_cp uuid_t
#else
typedef struct uuid uuid_cp;
#endif
#endif


void uuidToBytes(void *str, uuid_cp *uuid);
void uuidFromBytes(void *str, uuid_cp *uuid);
void uuidToStr(void *str, uuid_cp *uuid);
void uuidFromStr(void *str, uuid_cp *uuid);
void uuidCreate(uuid_cp *uuid);
void uuidCreateNil(uuid_cp *uuid);
void uuidCopy(uuid_cp *from, uuid_cp *to );

int uuidCompare(uuid_cp *uuid0, uuid_cp *uuid1);


#endif
