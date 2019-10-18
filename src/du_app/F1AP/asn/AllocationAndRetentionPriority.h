/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "F1AP-IEs"
 * 	found in "F1.asn1"
 * 	`asn1c -D ./out -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_AllocationAndRetentionPriority_H_
#define	_AllocationAndRetentionPriority_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PriorityLevel.h"
#include "Pre-emptionCapability.h"
#include "Pre-emptionVulnerability.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ProtocolExtensionContainer;

/* AllocationAndRetentionPriority */
typedef struct AllocationAndRetentionPriority {
	PriorityLevel_t	 priorityLevel;
	Pre_emptionCapability_t	 pre_emptionCapability;
	Pre_emptionVulnerability_t	 pre_emptionVulnerability;
	struct ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AllocationAndRetentionPriority_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AllocationAndRetentionPriority;
extern asn_SEQUENCE_specifics_t asn_SPC_AllocationAndRetentionPriority_specs_1;
extern asn_TYPE_member_t asn_MBR_AllocationAndRetentionPriority_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _AllocationAndRetentionPriority_H_ */
#include <asn_internal.h>
