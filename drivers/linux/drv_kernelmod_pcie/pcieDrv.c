/**
********************************************************************************
\file   pcieDrv.c

\brief  Linux PCIe driver interface for openPOWERLINK kernel daemon

This module handles all the application request forwarded to the daemon
in Linux kernel. It uses dualprocshm and circbuf libraries to manage PDO
memory, error objects shared memory, event and DLL queues.

\ingroup module_driver_linux_kernel_pcie
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2015, Kalycito Infotech Private Limited
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
#include <linux/semaphore.h>
#endif

#include "drvintf.h"
#include "pcieDrv.h"

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
#error "Linux Kernel versions older 2.6.19 are not supported by this driver!"
#endif

#define OPLK_MAX_BAR_COUNT      6
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

#define DRV_NAME                "plk"                   // driver name to be used by Linux

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

/**
\brief PCIe BAR information

The structure holds the information of the PCIe BAR mapped.

*/
typedef struct
{
    ULONG       phyAddr;                                ///< Physical address of the BAR. XXX ignored for now
    ULONG       virtualAddr;                            ///< Virtual address of the BAR in kernel memory.
    ULONG       length;                                 ///< Length of the BAR.
} tBarInfo;

typedef struct
{
    struct pci_dev*     pPciDev;                        // pointer to PCI device structure
    tBarInfo            barInfo[OPLK_MAX_BAR_COUNT];    // Bar instances of the PCIe interface
} tPcieDrvInstance;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static INT          initOnePciDev(struct pci_dev* pPciDev_p,
                                  const struct pci_device_id* pId_p);
static void         removeOnePciDev(struct pci_dev* pPciDev_p);
static irqreturn_t  pcieDrvIrqHandler(INT irqNum_p, void* ppDevInstData_p);

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

static struct pci_device_id     aDriverPciTbl_l[] =
{
    {0x1677, 0xe53f, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},  // APC2100
    {0x1677, 0xe809, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},  // APC2100
    {0x1172, 0xe001, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},  // DE2i150
    {0, }
};

MODULE_DEVICE_TABLE(pci, aDriverPciTbl_l);

static struct pci_driver        pcieDrvDriver_l =
{
    .name         = DRV_NAME,
    .id_table     = aDriverPciTbl_l,
    .probe        = initOnePciDev,
    .remove       = removeOnePciDev,
};

static tPcieDrvInstance         pcieDrvInstance_l;

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//
//------------------------------------------------------------------------------
/**
\brief  PCIe driver initialization

This function initializes the PCIe driver.

\return The function returns a tOplkError error code.

\ingroup module_driver_linux_kernel_pcie
*/
//------------------------------------------------------------------------------
tOplkError pcieDrv_init(void)
{
    tOplkError      ret;
    INT             result;
    UINT            index;

    ret = kErrorOk;

    // clear instance structure
    OPLK_MEMSET(&pcieDrvInstance_l, 0, sizeof(pcieDrvInstance_l));

    // save the init data

    // clear driver structure
    OPLK_MEMSET(&pcieDrvDriver_l, 0, sizeof(pcieDrvDriver_l));
    pcieDrvDriver_l.name         = DRV_NAME,
    pcieDrvDriver_l.id_table     = aDriverPciTbl_l,
    pcieDrvDriver_l.probe        = initOnePciDev,
    pcieDrvDriver_l.remove       = removeOnePciDev,

    // register PCI driver
    result = pci_register_driver(&pcieDrvDriver_l);
    if (result != 0)
    {
        printk("%s pci_register_driver failed with %d\n", __FUNCTION__, result);
        ret = kErrorNoResource;
        goto Exit;
    }

    if (pcieDrvInstance_l.pPciDev == NULL)
    {
        printk("%s pPciDev=NULL\n", __FUNCTION__);
        ret = pcieDrv_shutdown();
        ret = kErrorNoResource;
        goto Exit;
    }

Exit:
    return ret;
}

//------------------------------------------------------------------------------
/**
\brief  PCIe driver shutdown

This function shuts down the PCIe driver.

\return The function returns a tOplkError error code.

\ingroup module_driver_linux_kernel_pcie
*/
//------------------------------------------------------------------------------
tOplkError pcieDrv_shutdown(void)
{
    // unregister PCI driver
    printk("%s calling pci_unregister_driver()\n", __FUNCTION__);
    pci_unregister_driver(&pcieDrvDriver_l);

    return kErrorOk;
}

//------------------------------------------------------------------------------
/**
\brief  Get PCIe BAR virtual address

This routine fetches the BAR address of the requested BAR.

\param  barCount_p     ID of the requested BAR.

\return Returns the address of requested PCIe BAR.

\ingroup module_driver_linux_kernel_pcie
*/
//------------------------------------------------------------------------------
ULONG pcieDrv_getBarAddr(UINT8 barCount_p)
{
    if (barCount_p >= OPLK_MAX_BAR_COUNT)
    {
        return 0;
    }

    return pcieDrvInstance_l.barInfo[barCount_p].virtualAddr;
}

//------------------------------------------------------------------------------
/**
\brief  Get BAR Length

\param  barCount_p     ID of the requested BAR.

\return Returns the length of requested PCIe BAR.

\ingroup module_driver_linux_kernel_pcie
*/
//------------------------------------------------------------------------------
ULONG pcieDrv_getBarLength(ULONG barCount_p)
{
    if (barCount_p >= OPLK_MAX_BAR_COUNT)
    {
        return 0;
    }

    return pcieDrvInstance_l.barInfo[barCount_p].length;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  PCIe driver interrupt handler

This function is the interrupt service routine for the PCIe driver.

\param  irqNum_p            IRQ number
\param  ppDevInstData_p     Pointer to private data provided by request_irq

\return The function returns an IRQ handled code.
*/
//------------------------------------------------------------------------------
static irqreturn_t pcieDrvIrqHandler(INT irqNum_p, void* ppDevInstData_p)
{
    UINT16      status;
    INT         handled = IRQ_HANDLED;

    UNUSED_PARAMETER(irqNum_p);
    UNUSED_PARAMETER(ppDevInstData_p);

Exit:
    return handled;
}

//------------------------------------------------------------------------------
/**
\brief  Initialize one PCI device

This function initializes one PCI device.

\param  pPciDev_p   Pointer to corresponding PCI device structure
\param  pId_p       PCI device ID

\return The function returns an integer error code.
\retval 0           Successful
\retval Otherwise   Error
*/
//------------------------------------------------------------------------------
static INT initOnePciDev(struct pci_dev* pPciDev_p,
                         const struct pci_device_id* pId_p)
{
    UINT        index;
    UINT32      temp;
    INT         result = 0;
    UINT16      regValue;
    UINT64      descAddr;
    UINT        order;
    UINT        rxBuffersInAllocation;
    UINT        rxBufferCount;
    UINT32      flags_le;
    UINT8       barCount = 0;
    tBarInfo*   pBarInfo = NULL;

    if (pcieDrvInstance_l.pPciDev != NULL)
    {
        // driver is already connected to a PCI device
        printk("%s device %s discarded\n", __FUNCTION__, pci_name(pPciDev_p));
        result = -ENODEV;
        goto Exit;
    }

    pcieDrvInstance_l.pPciDev = pPciDev_p;

    // enable device
    printk("%s enable device\n", __FUNCTION__);
    result = pci_enable_device(pPciDev_p);
    if (result != 0)
    {
        goto Exit;
    }

    printk("%s request regions\n", __FUNCTION__);
    result = pci_request_regions(pPciDev_p, DRV_NAME);
    if (result != 0)
    {
        goto ExitFail;
    }

    //XXX Ignoring whether or not any BAR is accessible
    for (barCount = 0; barCount < OPLK_MAX_BAR_COUNT; barCount++)
    {
        pBarInfo = &pcieDrvInstance_l.barInfo[barCount];

        if (pBarInfo->virtualAddr != NULL)
        {
            // The instance is already present
            result = -EIO;
            goto ExitFail;
        }

        // look for the MMIO BARs
        if ((pci_resource_flags(pPciDev_p, barCount) & IORESOURCE_MEM) == 0)
        {
            continue;
        }

        // get the size of this field
        pBarInfo->length = pci_resource_len(pPciDev_p, barCount);

        printk("%s() --> ioremap\n", __FUNCTION__);
        printk("\tbar#\t%u\n", barCount);
        printk("\tbarLen\t%lu\n", pBarInfo->length);
        pBarInfo->virtualAddr = ioremap(pci_resource_start(pPciDev_p, barCount),
                                        pBarInfo->length);

        printk("\tbarMap\t0x%X\n", pBarInfo->virtualAddr);
        if (pBarInfo->virtualAddr == NULL)
        {
            // remap of controller's register space failed
            result = -EIO;
            goto ExitFail;
        }
    }

    // Enable PCI busmaster
    printk("%s enable busmaster\n", __FUNCTION__);
    pci_set_master(pPciDev_p);

    for (barCount = 0; barCount < 15; barCount += 2)
    {
        printk("Bar Data: 0x%X --> 0x%X\n",
               pcieDrvInstance_l.barInfo[1].virtualAddr + barCount,
               readw(pcieDrvInstance_l.barInfo[1].virtualAddr + barCount));
    }

    // Enable msi
    printk("Enable MSI\n");
    result = pci_enable_msi(pPciDev_p);
    if (result != 0)
    {
        printk("%s Could not enable MSI\n", __FUNCTION__);
    }

    // install interrupt handler
    printk("%s install interrupt handler\n", __FUNCTION__);
    result = request_irq(pPciDev_p->irq,
                         pcieDrvIrqHandler,
                         IRQF_SHARED,
                         DRV_NAME, /* pPciDev_p->dev.name */
                         pPciDev_p);
    if (result != 0)
    {
        goto ExitFail;
    }

    goto Exit;

ExitFail:
    removeOnePciDev(pPciDev_p);

Exit:
    printk("%s finished with %d\n", __FUNCTION__, result);
    return result;
}

//------------------------------------------------------------------------------
/**
\brief  Remove one PCI device

This function removes one PCI device.

\param  pPciDev_p     Pointer to corresponding PCI device structure
*/
//------------------------------------------------------------------------------
static void removeOnePciDev(struct pci_dev* pPciDev_p)
{
    UINT32      temp;
    UINT        order;
    ULONG       bufferPointer;
    UINT        rxBufferCount;
    UINT8       barCount = 0;
    tBarInfo*   pBarInfo = NULL;

    if (pcieDrvInstance_l.pPciDev != pPciDev_p)
    {
        // trying to remove unknown device
        BUG_ON(pcieDrvInstance_l.pPciDev != pPciDev_p);
        goto Exit;
    }

    // remove interrupt handler
    free_irq(pPciDev_p->irq, pPciDev_p);

    // Disable Message Signaled Interrupt
    printk("%s Disable MSI\n", __FUNCTION__);
    pci_disable_msi(pPciDev_p);

    // unmap controller's register space
    for (barCount = 0; barCount < OPLK_MAX_BAR_COUNT; barCount++)
    {
        pBarInfo = &pcieDrvInstance_l.barInfo[barCount];

        if (pBarInfo->virtualAddr != NULL)
        {
            iounmap(pBarInfo->virtualAddr);
            pBarInfo->virtualAddr = NULL;
        }
    }

    // disable the PCI device
    pci_disable_device(pPciDev_p);

    // release memory regions
    pci_release_regions(pPciDev_p);

    pcieDrvInstance_l.pPciDev = NULL;

Exit:
    return;
}

///\}
