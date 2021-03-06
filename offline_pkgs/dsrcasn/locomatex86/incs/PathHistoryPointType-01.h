/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PathHistoryPointType_01_H_
#define	_PathHistoryPointType_01_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "PositionalAccuracy.h"
#include "TransmissionAndSpeed.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PathHistoryPointType-01 */
typedef struct PathHistoryPointType_01 {
	long	 latOffset;
	long	 longOffset;
	long	*elevationOffset	/* OPTIONAL */;
	long	*timeOffset	/* OPTIONAL */;
	PositionalAccuracy_t	*posAccuracy	/* OPTIONAL */;
	long	*heading	/* OPTIONAL */;
	TransmissionAndSpeed_t	*speed	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PathHistoryPointType_01_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PathHistoryPointType_01;

#ifdef __cplusplus
}
#endif

#endif	/* _PathHistoryPointType_01_H_ */
