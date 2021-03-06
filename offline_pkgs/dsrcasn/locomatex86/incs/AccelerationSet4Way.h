/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_AccelerationSet4Way_H_
#define	_AccelerationSet4Way_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AccelerationSet4Way */
typedef OCTET_STRING_t	 AccelerationSet4Way_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AccelerationSet4Way;
asn_struct_free_f AccelerationSet4Way_free;
asn_struct_print_f AccelerationSet4Way_print;
asn_constr_check_f AccelerationSet4Way_constraint;
ber_type_decoder_f AccelerationSet4Way_decode_ber;
der_type_encoder_f AccelerationSet4Way_encode_der;
xer_type_decoder_f AccelerationSet4Way_decode_xer;
xer_type_encoder_f AccelerationSet4Way_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _AccelerationSet4Way_H_ */
