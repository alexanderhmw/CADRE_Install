/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PathHistoryPointType_02_H_
#define	_PathHistoryPointType_02_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PathHistoryPointType-02 */
typedef OCTET_STRING_t	 PathHistoryPointType_02_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PathHistoryPointType_02;
asn_struct_free_f PathHistoryPointType_02_free;
asn_struct_print_f PathHistoryPointType_02_print;
asn_constr_check_f PathHistoryPointType_02_constraint;
ber_type_decoder_f PathHistoryPointType_02_decode_ber;
der_type_encoder_f PathHistoryPointType_02_encode_der;
xer_type_decoder_f PathHistoryPointType_02_decode_xer;
xer_type_encoder_f PathHistoryPointType_02_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _PathHistoryPointType_02_H_ */
