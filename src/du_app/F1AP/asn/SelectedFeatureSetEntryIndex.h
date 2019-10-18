/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "F1AP-IEs"
 * 	found in "F1.asn1"
 * 	`asn1c -D ./out -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_SelectedFeatureSetEntryIndex_H_
#define	_SelectedFeatureSetEntryIndex_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SelectedFeatureSetEntryIndex */
typedef OCTET_STRING_t	 SelectedFeatureSetEntryIndex_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SelectedFeatureSetEntryIndex;
asn_struct_free_f SelectedFeatureSetEntryIndex_free;
asn_struct_print_f SelectedFeatureSetEntryIndex_print;
asn_constr_check_f SelectedFeatureSetEntryIndex_constraint;
ber_type_decoder_f SelectedFeatureSetEntryIndex_decode_ber;
der_type_encoder_f SelectedFeatureSetEntryIndex_encode_der;
xer_type_decoder_f SelectedFeatureSetEntryIndex_decode_xer;
xer_type_encoder_f SelectedFeatureSetEntryIndex_encode_xer;
oer_type_decoder_f SelectedFeatureSetEntryIndex_decode_oer;
oer_type_encoder_f SelectedFeatureSetEntryIndex_encode_oer;
per_type_decoder_f SelectedFeatureSetEntryIndex_decode_uper;
per_type_encoder_f SelectedFeatureSetEntryIndex_encode_uper;
per_type_decoder_f SelectedFeatureSetEntryIndex_decode_aper;
per_type_encoder_f SelectedFeatureSetEntryIndex_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _SelectedFeatureSetEntryIndex_H_ */
#include <asn_internal.h>
