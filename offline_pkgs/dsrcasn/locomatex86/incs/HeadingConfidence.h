/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "../downloads/DSRC_R36_Source.ASN"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_HeadingConfidence_H_
#define	_HeadingConfidence_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ENUMERATED.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum HeadingConfidence {
	HeadingConfidence_unavailable	= 0,
	HeadingConfidence_prec45deg	= 1,
	HeadingConfidence_prec10deg	= 2,
	HeadingConfidence_prec05deg	= 3,
	HeadingConfidence_prec01deg	= 4,
	HeadingConfidence_prec0_1deg	= 5,
	HeadingConfidence_prec0_05deg	= 6,
	HeadingConfidence_prec0_01deg	= 7
} e_HeadingConfidence;

/* HeadingConfidence */
typedef ENUMERATED_t	 HeadingConfidence_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HeadingConfidence;
asn_struct_free_f HeadingConfidence_free;
asn_struct_print_f HeadingConfidence_print;
asn_constr_check_f HeadingConfidence_constraint;
ber_type_decoder_f HeadingConfidence_decode_ber;
der_type_encoder_f HeadingConfidence_encode_der;
xer_type_decoder_f HeadingConfidence_decode_xer;
xer_type_encoder_f HeadingConfidence_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _HeadingConfidence_H_ */
