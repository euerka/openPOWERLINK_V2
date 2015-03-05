/**
********************************************************************************
\file   main.c

\brief  Main file of PCIe MN test application

This file contains the main file of the openPOWERLINK PCIe MN test
application.

\ingroup module_pcie_test_app
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2015, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2015, Kalycito Infotech Private Ltd.All rights reserved.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <stdio.h>

#include <oplk/oplk.h>

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tOplkError stubEventCb(tOplkApiEventType EventType_p,
                              tOplkApiEventArg* pEventArg_p,
                              void* pUserArg_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  main function

This is the main function of the openPOWERLINK MN PCIe test application.

\param  argc                    Number of arguments
\param  argv                    Pointer to argument strings

\return Returns an exit code

\ingroup module_pcie_test_app
*/
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    tOplkError          ret;
    tOplkApiInitParam   initParam;
    char                devName[128];
    UINT8               aMacAddr[6];

    memset(&initParam, 0, sizeof(initParam));
    initParam.sizeOfInitParam = sizeof(initParam);

    // Set dummy parameters to start stack without errors
    initParam.nodeId = 0xF0;
    initParam.cycleLen = 10000;
    initParam.hwParam.pDevName = devName;
    initParam.pfnCbEvent = stubEventCb;

    // initialize POWERLINK stack
    printf("Initialize openPOWERLINK stack...\n");
    ret = oplk_init(&initParam);
    if (ret != kErrorOk)
    {
        fprintf(stderr, "oplk_init() failed with 0x%X\n", ret);
        goto Exit;
    }

    // Get MAC address (which comes from IF card!)
    printf("Get MAC address from IF card...\n");
    ret = oplk_getEthMacAddr((UINT8*) aMacAddr);
    if (ret == kErrorOk)
    {
        printf("\tMAC Address:\t\t%02X:%02X:%02X:%02X:%02X:%02X\n",
               aMacAddr[0], aMacAddr[1], aMacAddr[2],
               aMacAddr[3], aMacAddr[4], aMacAddr[5]);
    }
    else
    {
        fprintf(stderr, "ERROR: Failed to read MAC address\n");
    }

    usleep(1000);
    oplk_shutdown();

Exit:
    usleep(1000);
    return 0;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  Stub for receiving events from openPOWERLINK stack

This is a stub function for receiving events from openPOWERLINK stack.

\param  EventType_p Type of event
\param  pEventArg_p     Pointer to union which describes the event in detail
\param  pUserArg_p      User specific argument

\return The functions returns a tOplkError code.

*/
//------------------------------------------------------------------------------
static tOplkError stubEventCb(tOplkApiEventType EventType_p,
                              tOplkApiEventArg* pEventArg_p,
                              void* pUserArg_p)
{
    UNUSED_PARAMETER(EventType_p);
    UNUSED_PARAMETER(pEventArg_p);
    UNUSED_PARAMETER(pUserArg_p);

    return kErrorOk;
}

/// \}
