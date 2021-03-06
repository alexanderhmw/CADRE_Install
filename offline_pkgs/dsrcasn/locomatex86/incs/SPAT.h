/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_SPAT_H_
#define	_SPAT_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DSRCmsgID.h"
#include "DescriptiveName.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct IntersectionState;

/* SPAT */
typedef struct SPAT {
	DSRCmsgID_t	 msgID;
	DescriptiveName_t	*name	/* OPTIONAL */;
	struct SPAT__intersections {
		A_SEQUENCE_OF(struct IntersectionState) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} intersections;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SPAT_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SPAT;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "IntersectionState.h"

#endif	/* _SPAT_H_ */
