/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PathPrediction_H_
#define	_PathPrediction_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PathPrediction */
typedef struct PathPrediction {
	long	 radiusOfCurve;
	long	 confidence;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PathPrediction_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PathPrediction;

#ifdef __cplusplus
}
#endif

#endif	/* _PathPrediction_H_ */
