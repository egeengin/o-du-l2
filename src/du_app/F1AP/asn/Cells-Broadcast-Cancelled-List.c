/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "F1AP-PDU-Contents"
 * 	found in "F1.asn1"
 * 	`asn1c -D ./out -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "Cells-Broadcast-Cancelled-List.h"

#include "ProtocolIE-SingleContainer.h"
static asn_oer_constraints_t asn_OER_type_Cells_Broadcast_Cancelled_List_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..512)) */};
static asn_per_constraints_t asn_PER_type_Cells_Broadcast_Cancelled_List_constr_1 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 9,  9,  1,  512 }	/* (SIZE(1..512)) */,
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_Cells_Broadcast_Cancelled_List_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ProtocolIE_SingleContainer_4590P53,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_Cells_Broadcast_Cancelled_List_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_Cells_Broadcast_Cancelled_List_specs_1 = {
	sizeof(struct Cells_Broadcast_Cancelled_List),
	offsetof(struct Cells_Broadcast_Cancelled_List, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_Cells_Broadcast_Cancelled_List = {
	"Cells-Broadcast-Cancelled-List",
	"Cells-Broadcast-Cancelled-List",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_Cells_Broadcast_Cancelled_List_tags_1,
	sizeof(asn_DEF_Cells_Broadcast_Cancelled_List_tags_1)
		/sizeof(asn_DEF_Cells_Broadcast_Cancelled_List_tags_1[0]), /* 1 */
	asn_DEF_Cells_Broadcast_Cancelled_List_tags_1,	/* Same as above */
	sizeof(asn_DEF_Cells_Broadcast_Cancelled_List_tags_1)
		/sizeof(asn_DEF_Cells_Broadcast_Cancelled_List_tags_1[0]), /* 1 */
	{ &asn_OER_type_Cells_Broadcast_Cancelled_List_constr_1, &asn_PER_type_Cells_Broadcast_Cancelled_List_constr_1, SEQUENCE_OF_constraint },
	asn_MBR_Cells_Broadcast_Cancelled_List_1,
	1,	/* Single element */
	&asn_SPC_Cells_Broadcast_Cancelled_List_specs_1	/* Additional specs */
};

