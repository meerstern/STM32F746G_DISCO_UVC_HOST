/**
  ******************************************************************************
  * @file    usbh_mtp.c
  * @author  MCD Application Team
  * @brief   This file is the MTP Layer Handlers for USB Host MTP class.
  *
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_uvc.h"
#include "stm32f7xx_ll_usb.h"
#include "usbh_conf.h"
#include "usb_host.h"
/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_TEMPLATE_CLASS
* @{
*/

/** @defgroup USBH_TEMPLATE_CORE
* @brief    This file includes TEMPLATE Layer Handlers for USB Host TEMPLATE class.
* @{
*/

/** @defgroup USBH_TEMPLATE_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_TEMPLATE_CORE_Private_Defines
* @{
*/
/**
* @}
*/


/** @defgroup USBH_TEMPLATE_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBH_TEMPLATE_CORE_Private_Variables
* @{
*/
/**
* @}
*/
extern HCD_HandleTypeDef hhcd_USB_OTG_FS;

/** @defgroup USBH_TEMPLATE_CORE_Private_FunctionPrototypes
* @{
*/

static USBH_StatusTypeDef USBH_UVC_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_SOFProcess(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_FindVideoControl(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_FindVideoStreamingIN(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_ParseCSDescriptors(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef ParseCSDescriptors(VIDEO_ClassSpecificDescTypedef *class_desc, uint8_t subclass, uint8_t *pdesc);
static USBH_StatusTypeDef USBH_UVC_FindFormat(VIDEO_ClassSpecificDescTypedef *class_desc);
static USBH_StatusTypeDef USBH_UVC_FindFrame(VIDEO_ClassSpecificDescTypedef *class_desc);
USBH_StatusTypeDef USBH_UVC_SetCur(USBH_HandleTypeDef *phost, uint8_t subtype);
USBH_StatusTypeDef USBH_UVC_GetCur(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_UVC_GetMax(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_UVC_GetMin(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  UVC_Class =
{
  "VIDEO",
  USB_UVC_CLASS,
  USBH_UVC_InterfaceInit,
  USBH_UVC_InterfaceDeInit,
  USBH_UVC_ClassRequest,
  USBH_UVC_Process,
  NULL
  //USBH_UVC_SOFProcess
};

VIDEO_PROBE_COMMIT_CONTROLS_HandleTypeDef params;
volatile uint8_t packet_framebuffer[UVC_RX_FIFO_SIZE];
/**
* @}
*/


/** @defgroup USBH_TEMPLATE_CORE_Private_Functions
* @{
*/

/**
  * @brief  USBH_TEMPLATE_InterfaceInit
  *         The function init the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_InterfaceInit(USBH_HandleTypeDef *phost)
{
	VIDEO_HandleTypeDef *VIDEO_Handle;
	USBH_StatusTypeDef status=USBH_OK;
	uint8_t interface;
	uint16_t ep_size = 0U;
	uint8_t index;


	interface=USBH_FindInterface(phost, USB_UVC_CLASS, USB_SUBCLASS_VIDEOCONTROL, 0x00U);
	if (interface == 0xFFU) /* Not Valid Interface */
	{
		USBH_DbgLog("Cannot Find the interface1 for %s class.", phost->pActiveClass->Name);
		return USBH_FAIL;
	}

	interface=USBH_UVC_FindVideoControl(phost);
	if (interface == 0xFFU) /* Not Valid Interface */
	{
		USBH_DbgLog("Cannot Find the interface2 for %s class.", phost->pActiveClass->Name);
		return USBH_FAIL;
	}


    phost->pActiveClass->pData = (VIDEO_HandleTypeDef *)USBH_malloc (sizeof(VIDEO_HandleTypeDef));
    VIDEO_Handle =  (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

    if (VIDEO_Handle == NULL)
    {
		USBH_DbgLog("Cannot allocate memory for AUDIO Handle");
		return USBH_FAIL;
    }

    /* Initialize video handler */
    USBH_memset(VIDEO_Handle, 0, sizeof(VIDEO_HandleTypeDef));


    /* 1st Step:  Find UVC Interfaces */
    status = USBH_UVC_FindVideoStreamingIN(phost);

    if(status == USBH_FAIL)
    {
		USBH_UsrLog("%s class configuration not supported.", phost->pActiveClass->Name);
		status = USBH_FAIL;
		return USBH_FAIL;
    }

    /* 2nd Step:  Select Video Interfaces with largest endpoint size : default behavior */
    //for (index = 0U; index < USB_MAX_VIDEO_STD_INTERFACE; index ++)
    	for (index = 0U; index < 1; index ++)
    {
      if (VIDEO_Handle->stream_in[index].valid == 1U)
      {
        if (ep_size < VIDEO_Handle->stream_in[index].EpSize && ep_size < UVC_FIFO_MAX_SIZE)
        {
			ep_size = VIDEO_Handle->stream_in[index].EpSize;
			VIDEO_Handle->video.interface = VIDEO_Handle->stream_in[index].interface;
			VIDEO_Handle->video.AltSettings = VIDEO_Handle->stream_in[index].AltSettings;
			VIDEO_Handle->video.Ep = VIDEO_Handle->stream_in[index].Ep;
			VIDEO_Handle->video.EpSize = VIDEO_Handle->stream_in[index].EpSize;
			VIDEO_Handle->video.Poll = (uint8_t)VIDEO_Handle->stream_in[index].Poll;
			VIDEO_Handle->video.supported = 1U;

			printf("index:%d, IF:%d, As:%d, Ep:%d, EpS:0x%X, Poll:%d\n\r",
					index,
					VIDEO_Handle->video.interface,
					VIDEO_Handle->video.AltSettings,
					VIDEO_Handle->video.Ep,
					VIDEO_Handle->video.EpSize,
					VIDEO_Handle->video.Poll);

        }
      }
    }

    printf("Selected EP size: %d bytes, index:%d\n\r", ep_size,index);

    /* 3rd Step:  Find and Parse Video interfaces */
    USBH_UVC_ParseCSDescriptors(phost);

    status=USBH_UVC_FindFormat(&VIDEO_Handle->class_desc);
    if(status == USBH_FAIL)
    {
    	printf("USBH_UVC_FindFormat ERR\n\r");
    	return status;
    }

    status=USBH_UVC_FindFrame(&VIDEO_Handle->class_desc);
    if(status == USBH_FAIL)
    {
    	printf("USBH_UVC_FindFrame ERR\n\r");
    	return status;
    }


    VIDEO_Handle->control.Pipe = USBH_AllocPipe( phost, VIDEO_Handle->control.Ep);
	status = USBH_OpenPipe(phost,
			VIDEO_Handle->control.Pipe,
			VIDEO_Handle->control.Ep ,
			phost->device.address,
			phost->device.speed,
			USB_EP_TYPE_INTR,
			VIDEO_Handle->control.EpSize);
    printf("Control OpenPipe:%d\n\r",status);
    if(status == USBH_FAIL)
    {
    	return status;
    }

   status = USBH_LL_SetToggle(phost, VIDEO_Handle->control.Pipe, 0);
   printf("USBH_LL_SetToggle:%d\n\r",status);
   printf("Control Pipe:0x%x Ep:%d\n\r",VIDEO_Handle->control.Pipe,VIDEO_Handle->control.Ep);
   if(status == USBH_FAIL)
   {
		return status;
   }

	VIDEO_Handle->class_req_state=CLASS_REQ_INIT;

	return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_UVC_InterfaceDeInit(USBH_HandleTypeDef *phost)
{

	VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
	if(VIDEO_Handle->video.Pipe != 0x00)
	{
		USBH_ClosePipe  (phost, VIDEO_Handle->video.Pipe);
		USBH_FreePipe  (phost, VIDEO_Handle->video.Pipe);
		VIDEO_Handle->video.Pipe = 0;
	}

	if(VIDEO_Handle->control.Pipe != 0x00)
	{
		USBH_ClosePipe  (phost, VIDEO_Handle->control.Pipe);
		USBH_FreePipe  (phost, VIDEO_Handle->control.Pipe);
		VIDEO_Handle->control.Pipe = 0;
	}

	if (phost->pActiveClass->pData)
	{
		USBH_free(phost->pActiveClass->pData);
		phost->pActiveClass->pData = 0U;
	}
	return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost)
{
	USBH_StatusTypeDef status=USBH_BUSY;
	VIDEO_HandleTypeDef *VIDEO_Handle;
	VIDEO_Handle =  (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;



	switch(VIDEO_Handle->class_req_state)
	{

		case CLASS_REQ_INIT:
			status = USBH_SetInterface(	phost, VIDEO_Handle->control.interface, 0U);
			if(status!=USBH_BUSY)
			{
				printf("USBH_SetInterface1:%d\n\r",status);
				VIDEO_Handle->class_req_state=CLASS_REQ_PROBE_GET0;
			}
			break;

		case CLASS_REQ_PROBE_GET0:
			status=USBH_UVC_GetCur(phost);
			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_GET_MAX;
				printf("USBH_UVC_GetCur:%d\n\r",status);
			}
			break;

		case CLASS_REQ_GET_MAX:
			status=USBH_UVC_GetMax(phost);
			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_GET_MIN;
				printf("USBH_UVC_GetMax:%d\n\r",status);
			}
			break;

		case CLASS_REQ_GET_MIN:
			status=USBH_UVC_GetMin(phost);
			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_PROBE_SET;
				printf("USBH_UVC_GetMin:%d\n\r",status);
			}
			break;

		case CLASS_REQ_PROBE_SET:
			status=USBH_UVC_SetCur(phost,UVC_PROBE_CONTROL_SET_CUR);
			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_PROBE_GET;
				printf("USBH_UVC_SetCur:%d\n\r",status);
			}
			break;

		case CLASS_REQ_PROBE_GET:
			status=USBH_UVC_GetCur(phost);

			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_COMMIT_SET;
				printf("USBH_UVC_GetCur:%d\n\r",status);
			}
			break;

		case CLASS_REQ_COMMIT_SET:
			status=USBH_UVC_SetCur(phost,UVC_COMMIT_CONTROL_SET_CUR);
			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_SET_INTERFACE;
				printf("USBH_UVC_SetCur2:%d\n\r",status);
			}
			break;

		case CLASS_REQ_SET_INTERFACE:

			//Alternate setting:  1:192(0xC0)bytes, 2:384(0x180)bytes, 3:512(0x200)bytes, 4:640(0x280)bytes, 5:800(0x320)bytes
			status = USBH_SetInterface(	phost,
										VIDEO_Handle->control.interface,
										VIDEO_Handle->control.AltSettings);
			printf("USBH_SetInterface2 IF:%d Als:%d RES:%d\n\r",VIDEO_Handle->control.interface,VIDEO_Handle->control.AltSettings,status);
			VIDEO_Handle->class_req_state=CLASS_REQ_OPEN_PIPE;

			break;

		case CLASS_REQ_OPEN_PIPE:
			VIDEO_Handle->video.Pipe  = USBH_AllocPipe(phost, VIDEO_Handle->video.Ep);
		    status=USBH_OpenPipe(	phost,
									VIDEO_Handle->video.Pipe,
									VIDEO_Handle->video.Ep,
									phost->device.address,
									phost->device.speed,
									USB_EP_TYPE_ISOC,
									VIDEO_Handle->video.EpSize);

		    printf("Pipe:%d, Ep:%d, Addr:%d\n\r",VIDEO_Handle->video.Pipe, VIDEO_Handle->video.Ep,phost->device.address);
		    if(status!=USBH_BUSY)
		    {
		    	status=USBH_LL_SetToggle(phost,  VIDEO_Handle->video.Pipe, 0U);

			    if(status!=USBH_BUSY)
			    {
			    	printf("USBH_LL_SetToggle:%d\n\r",status);
			    	VIDEO_Handle->class_req_state=CLASS_REQ_SET_INTERFACE2;
			    }
		    }
			break;

		case CLASS_REQ_SET_INTERFACE2:
			status = USBH_SetInterface(	phost,
										VIDEO_Handle->video.interface,
										VIDEO_Handle->video.AltSettings);
			printf("USBH_SetInterface3 IF:%d Als:%d RES:%d\n\r",VIDEO_Handle->video.interface,VIDEO_Handle->video.AltSettings,status);
			VIDEO_Handle->class_req_state=CLASS_REQ_START_ISOCHRONOUS;
			break;

		case CLASS_REQ_START_ISOCHRONOUS:
			status=USBH_IsocReceiveData(phost,
			                            (uint8_t*)packet_framebuffer,
			                            VIDEO_Handle->video.EpSize,
			                            VIDEO_Handle->video.Pipe);

			if(status!=USBH_BUSY)
			{
				VIDEO_Handle->class_req_state=CLASS_REQ_IDLE;
				printf("Start ISOCHRONOUS Size:%d Pipe:%d\n\r", VIDEO_Handle->video.EpSize,VIDEO_Handle->video.Pipe);
			}
			break;
		case CLASS_REQ_IDLE:
				return USBH_OK;
			break;

		case CLASS_REQ_NONE:
			break;


	}
	return USBH_BUSY;
}


/**
  * @brief  USBH_TEMPLATE_Process
  *         The function is for managing state machine for TEMPLATE data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_Process(USBH_HandleTypeDef *phost)
{
	USBH_URBStateTypeDef status;
	VIDEO_HandleTypeDef *VIDEO_Handle= (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

	status = USBH_LL_GetURBState(phost, VIDEO_Handle->video.Pipe);
	uint32_t rxlen = USBH_LL_GetLastXferSize(phost, VIDEO_Handle->video.Pipe);

	if(	(status == USBH_URB_DONE)
			&& ((phost->Timer - VIDEO_Handle->video.timer) >= VIDEO_Handle->video.Poll))
	{


		VIDEO_Handle->video.timer = phost->Timer;

        printf("Size:%d \n\r",(int)rxlen);
        USBH_IsocReceiveData(phost,
                            (uint8_t*)packet_framebuffer,
                            VIDEO_Handle->video.EpSize,
                            VIDEO_Handle->video.Pipe);
	}

	return USBH_BUSY;
}


/**
  * @brief  USBH_TEMPLATE_Init
  *         The function Initialize the TEMPLATE function
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_UVC_Init(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef Status = USBH_BUSY;

#if (USBH_USE_OS == 1U)
  osEvent event;

  event = osMessageGet(phost->class_ready_event, osWaitForever);

  if (event.status == osEventMessage)
  {
    if (event.value.v == USBH_CLASS_EVENT)
    {
#else
  while ((Status == USBH_BUSY) || (Status == USBH_FAIL))
  {
    /* Host background process */
    USBH_Process(phost);

    if (phost->gState == HOST_CLASS)
    {
#endif
      Status = USBH_OK;
    }
  }
  return Status;
}

/**
  * @brief  USBH_TEMPLATE_IOProcess
  *         TEMPLATE TEMPLATE process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_UVC_IOProcess(USBH_HandleTypeDef *phost)
{
  if (phost->device.is_connected == 1U)
  {
    if (phost->gState == HOST_CLASS)
    {
      USBH_UVC_Process(phost);
    }
  }

  return USBH_OK;
}

/**
  * @brief  USBH_UVC_SOFProcess
  *         The function is for managing the SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_SOFProcess (USBH_HandleTypeDef *phost)
{
  VIDEO_HandleTypeDef           *VIDEO_Handle;
  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  VIDEO_Handle->sof_received = 1;
  return USBH_OK;
}
/**
  * @brief  Find Video Control interfaces
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_FindVideoControl(USBH_HandleTypeDef *phost)
{
  uint8_t interface;
  USBH_StatusTypeDef status = USBH_FAIL ;
  VIDEO_HandleTypeDef *VIDEO_Handle;

  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  /* Look For VIDEO Control interface */
  for (interface = 0U; interface < USBH_MAX_NUM_INTERFACES; interface++)
  {
    if ((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == USB_UVC_CLASS) &&
        (phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass == USB_SUBCLASS_VIDEOCONTROL))
    {
      if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) &&
          (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0U))
      {
    	  VIDEO_Handle->control.Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    	  VIDEO_Handle->control.EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    	  VIDEO_Handle->control.interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
    	  VIDEO_Handle->control.AltSettings = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
    	  VIDEO_Handle->control.Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;

    	  VIDEO_Handle->control.supported = 1U;
    	  status = USBH_OK;


    	    printf("Control Interface Ep:0x%x, EpSize:0x%x, interface:0x%x, Alt:0x%x, Poll:0x%x,\n\r",
    	    		VIDEO_Handle->control.Ep,
					VIDEO_Handle->control.EpSize,
					VIDEO_Handle->control.interface,
					VIDEO_Handle->control.AltSettings,
					 VIDEO_Handle->control.Poll);

      }
    }
  }

  return status;
}
/**
  * @brief  Find IN Video Streaming interfaces
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_FindVideoStreamingIN(USBH_HandleTypeDef *phost)
{
  uint8_t interface, alt_settings;
  USBH_StatusTypeDef status = USBH_FAIL ;
  VIDEO_HandleTypeDef *VIDEO_Handle;

  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  /* Look For VideoStreaming IN interface */
  alt_settings = 0U;
  for (interface = 0U; interface < USBH_MAX_NUM_INTERFACES; interface++)
  {
    if ((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == USB_UVC_CLASS) &&
        (phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass == USB_SUBCLASS_VIDEOSTREAMING))
    {
      if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) &&
          (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0U))
      {
    	  VIDEO_Handle->stream_in[alt_settings].Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    	  VIDEO_Handle->stream_in[alt_settings].EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    	  VIDEO_Handle->stream_in[alt_settings].interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
    	  VIDEO_Handle->stream_in[alt_settings].AltSettings = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
    	  VIDEO_Handle->stream_in[alt_settings].Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
    	  VIDEO_Handle->stream_in[alt_settings].valid = 1U;
    	  alt_settings++;
      }
    }
  }

  if (alt_settings > 0U)
  {
    status = USBH_OK;
  }

  return status;
}

/**
  * @brief  Parse UVC and interfaces Descriptors
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_ParseCSDescriptors(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_OK;
  USBH_DescHeader_t            *pdesc;
  uint16_t                      ptr;
  uint8_t                       itf_index = 0U;
  uint8_t                       itf_number = 0U;
  uint8_t                       alt_setting;
  VIDEO_HandleTypeDef           *VIDEO_Handle;

  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  pdesc   = (USBH_DescHeader_t *)(void *)(phost->device.CfgDesc_Raw);
  ptr = USB_LEN_CFG_DESC;

  VIDEO_Handle->class_desc.FeatureUnitNum = 0U;
  VIDEO_Handle->class_desc.InputTerminalNum = 0U;
  VIDEO_Handle->class_desc.OutputTerminalNum = 0U;
  VIDEO_Handle->class_desc.ASNum = 0U;


  while (ptr < phost->device.CfgDesc.wTotalLength)
  {
    pdesc = USBH_GetNextDesc((uint8_t *)(void *)pdesc, &ptr);

    switch (pdesc->bDescriptorType)
    {

      case USB_DESC_TYPE_INTERFACE:
        itf_number = *((uint8_t *)(void *)pdesc + 2U);
        alt_setting = *((uint8_t *)(void *)pdesc + 3U);
        itf_index = USBH_FindInterfaceIndex(phost, itf_number, alt_setting);

        //printf("itf:%d, alt:%d, index:%d\n\r",itf_number,alt_setting,itf_index);

        break;

      case USB_DESC_TYPE_CS_INTERFACE:

        if (itf_number <= phost->device.CfgDesc.bNumInterfaces)
        {
			if ((itf_index == 0xFFU) || (itf_index >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
			{
			  //!!!! Size of CfgDesc_Raw is USBH_MAX_SIZE_CONFIGURATION, Max 512.
			  //!!!! Many UVC CfgDesc_Raw are over 512, then STM32 has difficulty of converting all of them.
			  //!!!! However, most video interface are first interface, you don't have to handle everything.
			  //!!!! -> Ignore converting errors.
			  //USBH_DbgLog("Cannot Find the Video interface index for %s class.", phost->pActiveClass->Name);
			  //status = USBH_FAIL;
			}
			else
			{

				ParseCSDescriptors(	&VIDEO_Handle->class_desc,
									phost->device.CfgDesc.Itf_Desc[itf_index].bInterfaceSubClass,
									(uint8_t *)pdesc);
			}
        }
        break;

      default:
        break;
    }
  }

  return status;
}

/**
  * @brief  Parse UVC interfaces
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef ParseCSDescriptors(VIDEO_ClassSpecificDescTypedef *class_desc, uint8_t subclass, uint8_t *pdesc)
{

  uint8_t frameNum;
  uint16_t width, height;

  if (subclass == USB_SUBCLASS_VIDEOCONTROL)
  {
    switch (pdesc[2])
    {
		case VC_HEADER:
			class_desc->cs_desc.HeaderDesc = (VIDEO_HeaderDescTypeDef *)(void *)pdesc;
			break;

		case VC_INPUT_TERMINAL:
			class_desc->cs_desc.InputTerminalDesc[class_desc->InputTerminalNum++] = (VIDEO_ITDescTypeDef *)(void *)pdesc;
			break;

		case VC_OUTPUT_TERMINAL:
			class_desc->cs_desc.OutputTerminalDesc[class_desc->OutputTerminalNum++] = (VIDEO_OTDescTypeDef *)(void *)pdesc;
			break;

		case VC_SELECTOR_UNIT:
			class_desc->cs_desc.SelectorUnitDesc[class_desc->SelectorUnitNum++] = (VIDEO_SelectorDescTypeDef *)(void *)pdesc;
			break;

		case VC_PROCESSING_UNIT:
			class_desc->cs_desc.ProcessingUnitDesc[class_desc->ProcessingUnitNum++] = (VIDEO_PUDescTypeDef *)(void *)pdesc;
			break;

		case VC_EXTENSION_UNIT:
			class_desc->cs_desc.ExtensionUnitDesc[class_desc->ExtensionUnitNum++] = (VIDEO_EUDescTypeDef *)(void *)pdesc;
			break;

		default:
			break;
    }
  }
  else
  {
    if (subclass == USB_SUBCLASS_VIDEOSTREAMING)
    {
      switch (pdesc[2])
      {
        case VS_INPUT_HEADER:
            class_desc->ss_desc.InputHeader[class_desc->InputHeaderNum] = (VIDEO_InputHeaderDescTypeDef*) pdesc;
            if(class_desc->InputHeaderNum < VIDEO_MAX_NUM_IN_HEADER)
            {
            	class_desc->InputHeaderNum++;
            }
        	break;

        case VS_STILL_IMAGE_FRAME:
        	class_desc->ss_desc.StillImageFormat[class_desc->StillImageFrameNum] = (VIDEO_StllImgFrameDescTypeDef*) pdesc;
        	if(class_desc->StillImageFrameNum < VIDEO_MAX_STILL_FORMAT)
        	{
        		class_desc->StillImageFrameNum++;
        	}
        	break;

        case VS_FORMAT_UNCOMPRESSED:
        	class_desc->ss_desc.UncompFormat[class_desc->UncompFormatNum] = (VIDEO_UncompFormatDescTypeDef*) pdesc;
        	if(class_desc->UncompFormatNum < VIDEO_MAX_UNCOMP_FORMAT)
        	{
        		class_desc->UncompFormatNum++;
        	}
        	break;

        case VS_FRAME_UNCOMPRESSED:
        	frameNum = class_desc->UncompFrameNum;
        	class_desc->ss_desc.UncompFrame[frameNum] = (VIDEO_UncompFrameDescTypeDef*) pdesc;
            width = LE16(class_desc->ss_desc.UncompFrame[frameNum]->wWidth);
            height = LE16(class_desc->ss_desc.UncompFrame[frameNum]->wHeight);
            printf("Uncompressed Frame detected: %d x %d\n\r", width, height);

            if(class_desc->UncompFrameNum < VIDEO_MAX_UNCOMP_FRAME)
            {
            	class_desc->UncompFrameNum++;
            }
        	break;

        case VS_FORMAT_MJPEG:
        	frameNum = class_desc->MJPEGFormatNum;
        	class_desc->ss_desc.MJPEGFormat[frameNum] = (VIDEO_MJPEGFormatDescTypeDef*) pdesc;
        	if(class_desc->MJPEGFormatNum < VIDEO_MAX_MJPEG_FORMAT)
        	{
        		printf("Format:%d, Index:%d\n\r",
        				class_desc->ss_desc.MJPEGFormat[frameNum]->bDescriptorSubtype,
						class_desc->ss_desc.MJPEGFormat[frameNum]->bFormatIndex);
        		class_desc->MJPEGFormatNum++;
        	}
        	break;

        case VS_FRAME_MJPEG:
        	frameNum = class_desc->MJPEGFrameNum;
        	class_desc->ss_desc.MJPEGFrame[frameNum] = (VIDEO_MJPEGFrameDescTypeDef*) pdesc;
            width = LE16(class_desc->ss_desc.MJPEGFrame[frameNum]->wWidth);
            height = LE16(class_desc->ss_desc.MJPEGFrame[frameNum]->wHeight);
            printf("MJPEG Frame detected: %d x %d\n\r", width, height);
            if(class_desc->MJPEGFrameNum < VIDEO_MAX_MJPEG_FRAME)
            {
            	class_desc->MJPEGFrameNum++;
            }
        	break;

        case VS_COLORFORMAT:
        	class_desc->ss_desc.ColorMatching[class_desc->ColorMatchingNum] = (VIDEO_ColorMatchDescTypeDef*) pdesc;
        	if(class_desc->ColorMatchingNum < VIDEO_MAX_COLOR_MATCHING)
        	{
        		class_desc->ColorMatchingNum++;
        	}
        	break;

        default:
          break;
      }
    }
  }

  return USBH_OK;
}
/**
  * @brief  Find UVC Format
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_FindFormat(VIDEO_ClassSpecificDescTypedef *class_desc)
{
	class_desc->bFormatIndex=-1;

	if(UVC_USE_FORMAT== UVC_YUV)
	{
		if(class_desc->UncompFormatNum > 0 )
		{
			class_desc->bFormatIndex=class_desc->ss_desc.UncompFormat[0]->bFormatIndex;
		}
	}
	else if(UVC_USE_FORMAT== UVC_MJPEG)
	{
		if(class_desc->MJPEGFormatNum > 0)
		{
			class_desc->bFormatIndex=class_desc->ss_desc.MJPEGFormat[0]->bFormatIndex;
		}
	}
	printf("Format Index:%d\n\r",class_desc->bFormatIndex);

	return class_desc->bFormatIndex<0?USBH_FAIL:USBH_OK;
}
/**
  * @brief  Find UVC Frame
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_UVC_FindFrame(VIDEO_ClassSpecificDescTypedef *class_desc)
{
	class_desc->bFrameIndex=-1;

	if(UVC_USE_FORMAT== UVC_YUV)
	{
		uint8_t frameNum=class_desc->UncompFrameNum;
		for(uint8_t i=0; i<frameNum; i++)
		{
			VIDEO_UncompFrameDescTypeDef *UncompFrame=class_desc->ss_desc.UncompFrame[i];
			if( (LE16(UncompFrame->wWidth) == UVC_USE_WIDTH) && (LE16(UncompFrame->wHeight) == UVC_USE_HEIGHT) )
			{
				class_desc->bFrameIndex=UncompFrame->bFrameIndex;
			}
		}


	}
	else if(UVC_USE_FORMAT== UVC_MJPEG)
	{
		uint8_t frameNum=class_desc->MJPEGFrameNum;
		for(uint8_t i=0; i<frameNum; i++)
		{
			VIDEO_MJPEGFrameDescTypeDef *MjepgFrame=class_desc->ss_desc.MJPEGFrame[i];
			if( (LE16(MjepgFrame->wWidth) == UVC_USE_WIDTH) && (LE16(MjepgFrame->wHeight) == UVC_USE_HEIGHT) )
			{
				class_desc->bFrameIndex=MjepgFrame->bFrameIndex;
			}

		}

	}

	printf("Frame Index:%d\n\r",class_desc->bFrameIndex);

	return class_desc->bFrameIndex<0?USBH_FAIL:USBH_OK;
}


USBH_StatusTypeDef USBH_UVC_SetCur(USBH_HandleTypeDef *phost, uint8_t subtype)
{

  VIDEO_HandleTypeDef *VIDEO_Handle;
  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  USBH_StatusTypeDef ret = USBH_OK;
  uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;

  switch(subtype)
  {
  	  case UVC_PROBE_CONTROL_SET_CUR:
  		  memset(&params, 0, sizeof(params));
  		  params.bmHint=0x0000;
  		  params.bFormatIndex = VIDEO_Handle->class_desc.bFormatIndex;
  		  params.bFrameIndex =  VIDEO_Handle->class_desc.bFrameIndex;
  		  params.dwFrameInterval = UVC_USE_FPS;
  		  params.wKeyFrameRate = 0;
  		  params.wPFrameRate = 0;
  		  params.wCompQuality = 0;
  		  params.wCompWindowSize = 0;
  		  params.wDelay = 0;
  		  params.dwMaxVideoFrameSize = 0;
  		  params.dwMaxPayloadTransferSize = 0;
  		  wValue = VS_PROBE_CONTROL;//0x0100U;
  		  wIndex = VIDEO_Handle->video.interface;//0x0001U;
  		  wLength =26;//26;
  		  uint8_t da[26];
  		  memcpy(da,(uint8_t*)&params,26);
  		  printf("\n\rSET:");
  		  for(int i=0;i<26;i++)printf("%x,",da[i]);

  		 printf("Fmt:%d, Frm:%d\n\r",params.bFormatIndex,params.bFrameIndex);
  		  break;


  	  case UVC_COMMIT_CONTROL_SET_CUR:
  		  wValue = VS_COMMIT_CONTROL;
  		  wIndex = VIDEO_Handle->video.interface;//0x0001U;
  		  wLength =26;//26;
  		  printf(" GetCur:IF:%d, dwMaxVideoFrameSize:%d\n\r",VIDEO_Handle->video.interface,(int)params.dwMaxVideoFrameSize);
  		  uint8_t dat[26];
  		  memcpy(dat,(uint8_t*)&params,26);

  		  for(int i=0;i<26;i++)printf("%x,",dat[i]);

  		  break;

  	  default:
  		  ret = USBH_FAIL;
  		  break;

  }


  if (ret != USBH_OK)
  {
    return ret;
  }

  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
  phost->Control.setup.b.bRequest = UVC_SET_CUR;
  phost->Control.setup.b.wValue.w = wValue;
  phost->Control.setup.b.wIndex.w = wIndex;
  phost->Control.setup.b.wLength.w = wLength;


  USBH_StatusTypeDef status;
  do
  {
    status = USBH_CtlReq(phost, (uint8_t *)&params, wLength);

  } while (status == USBH_BUSY);

  printf("VS_SetCur HALOK\n\r");
  return USBH_OK;

}


/**
  * @brief  Handle Get Cur request
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_UVC_GetCur(USBH_HandleTypeDef *phost)
{
	VIDEO_HandleTypeDef *VIDEO_Handle;
	VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
	//memset(&params, 0, sizeof(params));
	uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;

	wValue=VS_PROBE_CONTROL;
	wIndex=VIDEO_Handle->video.interface;//0x0001;
	wLength=26;//26;

	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
	phost->Control.setup.b.bRequest = UVC_GET_CUR;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	USBH_StatusTypeDef status;
	do
	{
		status = USBH_CtlReq(phost, (uint8_t *)&params, wLength);
	}
	while (status == USBH_BUSY);

	if (status == USBH_OK)
	{
		if (params.dwMaxVideoFrameSize > 0)
		{
		  printf("GetCur:dwMaxVideoFrameSize OK:%d\n\r",(int)params.dwMaxVideoFrameSize);
		  return USBH_OK;
		}
		else
		{
			printf("dwMaxVideoFrameSize NG\n\r");
			return USBH_FAIL;
		}
	}

	return status;

}


USBH_StatusTypeDef USBH_UVC_GetMax(USBH_HandleTypeDef *phost)
{
	VIDEO_HandleTypeDef *VIDEO_Handle;
	VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

	uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;

	wValue=VS_PROBE_CONTROL;
	wIndex=VIDEO_Handle->video.interface;//0x0001;
	wLength=26;//26;

	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
	phost->Control.setup.b.bRequest = UVC_GET_MAX;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;


	  USBH_StatusTypeDef status;
	  do
	  {
		  status = USBH_CtlReq(phost, (uint8_t *)&params, wLength);
	  }
	  while (status == USBH_BUSY);

	  if (status == USBH_OK)
	  {
	      return USBH_OK;
	  }

	  return status;

}

USBH_StatusTypeDef USBH_UVC_GetMin(USBH_HandleTypeDef *phost)
{
	VIDEO_HandleTypeDef *VIDEO_Handle;
	VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
	//memset(&params, 0, sizeof(params));
	uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;

	wValue=VS_PROBE_CONTROL;
	wIndex=VIDEO_Handle->video.interface;//0x0001;
	wLength=26;//26;

	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
	phost->Control.setup.b.bRequest = UVC_GET_MIN;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	//return (USBH_CtlReq(phost, (uint8_t *)(&params), wLength));
	  USBH_StatusTypeDef status;
	  do
	  {
		  status = USBH_CtlReq(phost, (uint8_t *)&params, wLength);
	  }
	  while (status == USBH_BUSY);

	  if (status == USBH_OK)
	  {
		  return USBH_OK;
	  }

	  return status;

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
