/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_RTCM_Corrections_H_
#define	_RTCM_Corrections_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DSRCmsgID.h"
#include "MsgCount.h"
#include "RTCM-Revision.h"
#include "RTCMHeader.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct FullPositionVector;
struct RTCMmsg;

/* RTCM-Corrections */
typedef struct RTCM_Corrections {
	DSRCmsgID_t	 msgID;
	MsgCount_t	 msgCnt;
	RTCM_Revision_t	 rev;
	struct FullPositionVector	*anchorPoint	/* OPTIONAL */;
	RTCMHeader_t	 rtcmHeader;
	struct RTCM_Corrections__rtcmSets {
		A_SEQUENCE_OF(struct RTCMmsg) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} rtcmSets;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RTCM_Corrections_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RTCM_Corrections;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "FullPositionVector.h"
#include "RTCMmsg.h"

#endif	/* _RTCM_Corrections_H_ */
