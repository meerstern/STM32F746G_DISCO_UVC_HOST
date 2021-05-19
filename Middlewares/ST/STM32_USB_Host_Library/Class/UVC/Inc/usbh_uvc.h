/**
  ******************************************************************************
  * @file    usbh_template.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_template.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_TEMPLATE_H
#define __USBH_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"


/** @addtogroup USBH_LIB
* @{
*/
#define	UVC_USE_FORMAT				UVC_MJPEG//UVC_MJPEG//UVC_MJPEG//UVC_YUV
#define UVC_USE_WIDTH				320//320//160
#define UVC_USE_HEIGHT				240//240//120
#define UVC_USE_FPS					UVC_05FPS//UVC_30FPS//UVC_05FPS

#define UVC_RX_FIFO_SIZE			(UVC_USE_WIDTH*UVC_USE_HEIGHT*2)

#define UVC_FIFO_MAX_SIZE			1024//1024//512

/** @addtogroup USBH_CLASS
* @{
*/
#define USB_UVC_CLASS					0x0EU
#define USB_SUBCLASS_VIDEOCONTROL		0x01U
#define USB_SUBCLASS_VIDEOSTREAMING		0x02U

#define USB_MAX_VIDEO_STD_INTERFACE		0x05U
#define USB_DESC_TYPE_CS_INTERFACE      0x24U

#define VIDEO_MAX_INTERFACE_NBR			0x05U
#define VIDEO_MAX_INPUT_TERMINAL_NBR	0x05U

#define VIDEO_MAX_NUM_IN_TERMINAL       10
#define VIDEO_MAX_NUM_OUT_TERMINAL      4
#define VIDEO_MAX_NUM_FEATURE_UNIT      2
#define VIDEO_MAX_NUM_PROCESSING_UNIT   2
#define VIDEO_MAX_NUM_SELECTOR_UNIT     2
#define VIDEO_MAX_NUM_EXTENSION_UNIT    2
#define VIDEO_MAX_NUM_IN_HEADER         3
#define VIDEO_MAX_MJPEG_FORMAT          3
#define VIDEO_MAX_MJPEG_FRAME           10
#define VIDEO_MAX_COLOR_MATCHING        3
#define VIDEO_MAX_UNCOMP_FORMAT         3
#define VIDEO_MAX_STILL_FORMAT          3
#define VIDEO_MAX_UNCOMP_FRAME          10
#define VIDEO_MAX_SAMFREQ_NBR           5
#define VIDEO_MAX_CONTROLS_NBR          5
//#define VS_PROBE_CONTROL                1
//#define VS_COMMIT_CONTROL               2

#define UVC_SET_CUR                    0x01
#define UVC_GET_CUR                    0x81
#define UVC_GET_MAX					   0x83
#define UVC_GET_MIN					   0x82

#define VS_PROBE_CONTROL  0x0111
#define VS_COMMIT_CONTROL 0x0222


/** @addtogroup USBH_TEMPLATE_CLASS
* @{
*/

/** @defgroup USBH_TEMPLATE_CLASS
* @brief This file is the Header file for usbh_template.c
* @{
*/


/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Types
* @{
*/

typedef struct
{
    uint16_t bmHint;
    uint8_t  bFormatIndex;
    uint8_t  bFrameIndex;
    uint32_t dwFrameInterval;
    uint16_t wKeyFrameRate;
    uint16_t wPFrameRate;
    uint16_t wCompQuality;
    uint16_t wCompWindowSize;
    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
    uint32_t dwClockFrequency;
    uint8_t  bmFramingInfo;
    uint8_t  bPreferedVersion;
    uint8_t  bMinVersion;
    uint8_t  bMaxVersion;
}
__packed VIDEO_PROBE_COMMIT_CONTROLS_HandleTypeDef;


typedef struct
{
  uint8_t              Ep;
  uint16_t             EpSize;
  uint8_t              AltSettings;
  uint8_t              interface;
  uint8_t              valid;
  uint16_t             Poll;
}
VIDEO_STREAMING_IN_HandleTypeDef;


typedef struct
{
  uint8_t              Ep;
  uint16_t             EpSize;
  uint8_t              interface;
  uint8_t              AltSettings;
  uint8_t              supported;
  uint8_t              Pipe;
  uint8_t              Poll;
  uint32_t             timer;
}
VIDEO_InterfaceStreamPropTypeDef;

typedef struct
{
  uint8_t              Ep;
  uint16_t             EpSize;
  uint8_t              interface;
  uint8_t              AltSettings;
  uint8_t              supported;
  uint8_t              Pipe;
  uint8_t              Poll;
  uint32_t             timer;
}
VIDEO_InterfaceControlPropTypeDef;

/* Class-specific VC Interface Header Descriptor */
typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bcdUVC[2];
  uint8_t  wTotalLength[2];
  uint8_t  dwClockFrequency[4];
  uint8_t  bInCollection;
  uint8_t  baInterfaceNr[VIDEO_MAX_INTERFACE_NBR];
}
VIDEO_HeaderDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bNumFormats;
  uint8_t  wTotalLength[2];
  uint8_t  bEndPointAddress;
  uint8_t  bmInfo;
  uint8_t  bTerminalLink;
  uint8_t  bStillCaptureMethod;
  uint8_t  bTriggerSupport;
  uint8_t  bTriggerUsage;
  uint8_t  bControlSize;
  uint8_t  bmaControls[VIDEO_MAX_INPUT_TERMINAL_NBR];
}
VIDEO_InputHeaderDescTypeDef;

typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bFormatIndex;
  uint8_t bNumFrameDescriptors;
  uint8_t bmFlags;
  uint8_t bDefaultFrameIndex;
  uint8_t bAspectRatioX;
  uint8_t bAspectRatioY;
  uint8_t bmInterlaceFlags;
  uint8_t bCopyProtect;
} VIDEO_MJPEGFormatDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bFrameIndex;
  uint8_t  bmCapabilities;
  uint8_t  wWidth[2];
  uint8_t  wHeight[2];
  uint8_t  dwMinBitRate[4];
  uint8_t  dwMaxBitRate[4];
  uint8_t  dwDefaultFrameInterval[4];
  uint8_t  bFrameIntervalType;
}
VIDEO_MJPEGFrameDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bFormatIndex;
  uint8_t  bNumFrameDescriptors;
  uint8_t  guidFormat[16];
  uint8_t  bBitsPerPixel;
  uint8_t  bDefaultFrameIndex;
  uint8_t  bAspectRatioX;
  uint8_t  bAspectRatioY;
  uint8_t  bmInterfaceFlags;
  uint8_t  bCopyProtect;
}
VIDEO_UncompFormatDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bFrameIndex;
  uint8_t  bmCapabilities;
  uint8_t  wWidth[2];
  uint8_t  wHeight[2];
  uint8_t  dwMinBitRate[4];
  uint8_t  dwMaxBitRate[4];
  uint8_t  dwDefaultFrameInterval[4];
  uint8_t  bFrameIntervalType;
}
VIDEO_UncompFrameDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bTerminalID;
  uint8_t  wTerminalType[2];
  uint8_t  bAssocTerminal;
  uint8_t  iTerminal;
  uint8_t  wObjectiveFocalLengthMin[2];
  uint8_t  wObjectiveFocalLengthMax[2];
  uint8_t  wOcularFocalLength[2];
  uint8_t  bControlSize;
  uint8_t  bmControls[3];
}
VIDEO_ITDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bTerminalID;
  uint8_t  wTerminalType[2];
  uint8_t  bAssocTerminal;
  uint8_t  bSourceID;
  uint8_t  iTerminal;
}
VIDEO_OTDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bUnitID;
  uint8_t  bSourceID;
  uint8_t  bControlSize;
  uint8_t  bmaControls[VIDEO_MAX_CONTROLS_NBR][2];
}
VIDEO_FeatureDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bUnitID;
  uint8_t  bNrInPins;
  uint8_t  bSourceID0;
  uint8_t  iSelector;
}
VIDEO_SelectorDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bUnitID;
  uint8_t  bSourceID0;
  uint8_t  wMaxMultiplier[2];
  uint8_t  bControlSize;
  uint8_t  bmControls[2];
  uint8_t  iProcessing;
}
VIDEO_PUDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bUnitID;
  uint8_t  guidExtensionCode[16];
  uint8_t  bNumControls;
  uint8_t  bNrInPins;
  uint8_t  baSourceID;
  uint8_t  bControlSize;
  uint8_t  bmControls[2];
  uint8_t  iExtension;
}
VIDEO_EUDescTypeDef;

typedef struct
{
	VIDEO_HeaderDescTypeDef   *HeaderDesc;
	VIDEO_ITDescTypeDef       *InputTerminalDesc [VIDEO_MAX_NUM_IN_TERMINAL];
	VIDEO_OTDescTypeDef       *OutputTerminalDesc[VIDEO_MAX_NUM_OUT_TERMINAL];
	VIDEO_PUDescTypeDef		  *ProcessingUnitDesc[VIDEO_MAX_NUM_PROCESSING_UNIT];
	VIDEO_SelectorDescTypeDef *SelectorUnitDesc  [VIDEO_MAX_NUM_SELECTOR_UNIT];
	VIDEO_EUDescTypeDef 	  *ExtensionUnitDesc [VIDEO_MAX_NUM_EXTENSION_UNIT];

}
VIDEO_CDescTypeDef;

typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bEndpointAddress;
  uint8_t  bNumImageSizePatterns;
  uint8_t  wWidth[2];
  uint8_t  wHeight[2];
  uint8_t  bNumCompressionPattern;
  uint8_t  bCompression[10];
}
VIDEO_StllImgFrameDescTypeDef;



typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  ColorPrimaries;
  uint8_t  bTransferCharacteristics;
  uint8_t  bMatrixCoefficients;
}
VIDEO_ColorMatchDescTypeDef;

typedef struct
{
	VIDEO_InputHeaderDescTypeDef     *InputHeader[VIDEO_MAX_NUM_IN_HEADER];
	VIDEO_StllImgFrameDescTypeDef  	 *StillImageFormat[VIDEO_MAX_STILL_FORMAT];
	VIDEO_UncompFormatDescTypeDef 	 *UncompFormat[VIDEO_MAX_UNCOMP_FORMAT];
	VIDEO_UncompFrameDescTypeDef     *UncompFrame[VIDEO_MAX_UNCOMP_FRAME];
	VIDEO_MJPEGFormatDescTypeDef     *MJPEGFormat[VIDEO_MAX_MJPEG_FORMAT];
	VIDEO_MJPEGFrameDescTypeDef      *MJPEGFrame[VIDEO_MAX_MJPEG_FRAME];
	VIDEO_ColorMatchDescTypeDef      *ColorMatching[VIDEO_MAX_COLOR_MATCHING];
}
VIDEO_SDescTypeDef;

typedef struct
{
	VIDEO_CDescTypeDef   cs_desc;//Control
	VIDEO_SDescTypeDef   ss_desc;//Stream

	uint16_t ASNum;
	uint16_t InputTerminalNum;
	uint16_t OutputTerminalNum;
	uint16_t ProcessingUnitNum;
	uint16_t FeatureUnitNum;
	uint16_t SelectorUnitNum;
	uint16_t ExtensionUnitNum;

	uint16_t InputHeaderNum;
	uint16_t StillImageFrameNum;
	uint16_t UncompFormatNum;
	uint16_t UncompFrameNum;
	uint16_t MJPEGFormatNum;
	uint16_t MJPEGFrameNum;
	uint16_t ColorMatchingNum;

	int8_t	 bFormatIndex;
	int8_t   bFrameIndex;
}
VIDEO_ClassSpecificDescTypedef;

typedef struct _VIDEO_Process
{
	VIDEO_STREAMING_IN_HandleTypeDef   stream_in[USB_MAX_VIDEO_STD_INTERFACE];
	VIDEO_InterfaceStreamPropTypeDef   video;
	VIDEO_InterfaceControlPropTypeDef  control;
	VIDEO_ClassSpecificDescTypedef     class_desc;
	uint8_t							   class_req_state;
	uint8_t sof_received;
}
VIDEO_HandleTypeDef;



/* States for TEMPLATE State Machine */


/**
* @}
*/

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Defines
* @{
*/
enum
{
	VC_DESCRIPTOR_UNDEFINED,
	VC_HEADER,
	VC_INPUT_TERMINAL,
	VC_OUTPUT_TERMINAL,
	VC_SELECTOR_UNIT,
	VC_PROCESSING_UNIT,
	VC_EXTENSION_UNIT
};

enum
{
	SC_UNDEFINED,
	SC_VIDEOCONTROL,
	SC_VIDEOSTREAMING,
	SC_VIDEO_INTERFACE_COLLECTION
};

enum
{
	VS_UNDEFINED,
	VS_INPUT_HEADER,
	VS_OUTPUT_HEADER,
	VS_STILL_IMAGE_FRAME,
	VS_FORMAT_UNCOMPRESSED,
	VS_FRAME_UNCOMPRESSED,
	VS_FORMAT_MJPEG,
	VS_FRAME_MJPEG,
	VS_FORMAT_MPEG2TS=0x0A,
	VS_FORMAT_DV=0x0C,
	VS_COLORFORMAT,
	VS_FORMAT_FRAME_BASED=0x10,
	VS_FRAME_FRAME_BASED,
	VS_FORMAT_STREAM_BASED
};

enum
{
	//UVC_UNKNOWN,
	UVC_MJPEG,
	UVC_YUV
};

enum
{
	UVC_PROBE_CONTROL_SET_CUR,
	UVC_PROBE_CONTROL_GET_CUR,
	UVC_COMMIT_CONTROL_SET_CUR
};


enum
{
	UVC_30FPS=333333,
	UVC_25FPS=400000,
	UVC_20FPS=500000,
	UVC_15FPS=666666,
	UVC_10FPS=1000000,
	UVC_05FPS=2000000,
	//UVC_01FPS=10000000
};

enum
{
	CLASS_REQ_NONE,
	CLASS_REQ_INIT,
	CLASS_REQ_PROBE_GET0,
	CLASS_REQ_GET_MAX,
	CLASS_REQ_GET_MIN,
	CLASS_REQ_PROBE_SET,
	CLASS_REQ_PROBE_GET,
	CLASS_REQ_COMMIT_SET,
	CLASS_REQ_SET_INTERFACE,
	CLASS_REQ_OPEN_PIPE,
	CLASS_REQ_SET_INTERFACE2,
	CLASS_REQ_START_ISOCHRONOUS,
	CLASS_REQ_IDLE
};


/**
* @}
*/

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Variables
* @{
*/
extern USBH_ClassTypeDef  UVC_Class;
#define USBH_UVC_CLASS    &UVC_Class

/**
* @}
*/

/** @defgroup USBH_TEMPLATE_CLASS_Exported_FunctionsPrototype
* @{
*/

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

