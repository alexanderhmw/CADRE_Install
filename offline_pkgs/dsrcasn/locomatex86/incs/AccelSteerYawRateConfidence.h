/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_AccelSteerYawRateConfidence_H_
#define	_AccelSteerYawRateConfidence_H_


#include <asn_application.h>

/* Including external dependencies */
#include "YawRateConfidence.h"
#include "AccelerationConfidence.h"
#include "SteeringWheelAngleConfidence.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AccelSteerYawRateConfidence */
typedef struct AccelSteerYawRateConfidence {
	YawRateConfidence_t	 yawRate;
	AccelerationConfidence_t	 acceleration;
	SteeringWheelAngleConfidence_t	 steeringWheelAngle;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AccelSteerYawRateConfidence_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AccelSteerYawRateConfidence;

#ifdef __cplusplus
}
#endif

#endif	/* _AccelSteerYawRateConfidence_H_ */
