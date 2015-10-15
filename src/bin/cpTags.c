/* 
Copyright (c) 2015,
All rights reserved.

Redistribution and use in source and binary forms,
modification,

* Redistributions of source code must retain the above copyright notice,
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT,
DAMAGES (INCLUDING,
SERVICES; LOSS OF USE,
CAUSED AND ON ANY THEORY OF LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE,

*/
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "cfConfig.h"


int main(int argc, char**argv)
{

	enum chatPacketTags tag = cptag_NOOP;

	for ( tag=cptag_NOOP; tag<cptag_ENDTAG; tag++) {
		printf ( " %02x %s\n", tag, tagLookup (tag) );
	}



}