/**
 * @file  tssUtils.h
 * @brief Auxiliary utilties for working with TPM
 *
 * Contains all functions concerning the trusted software stack
 *
 * @author Dominik Holling
 */

#ifndef _TSS_UTILS_H
#define	_TSS_UTILS_H

#include <trousers/tss.h>
#include <trousers/trousers.h>

#ifdef	__cplusplus
extern "C" {
#endif

/** The minimal PCR number this program uses to do extend operations.
 * Needs to be unmodified by the BIOS, Bootloader, etc.
 * @see attestation_extendPCR
 */
#define PCR 12

/**
 * Bootstrapping to use the TPM
 *
 * @param  srk_password The password of the storage root key
 * @return EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_connect(char* srk_password);

/**
 * Final action to use when exiting
 *
 * @return EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_disconnect();

/**
 * Do a complete self test of the TPM
 *
 * @return EXIT_SUCCESS if self test was successful, EXIT_FAILURE otherwise
 */
int tpm_selfTest();

/**
 * Load a signing key by UUID
 *
 */
int tpm_loadKey();

/**
 * Get the value of a PCR
 *
 * @param   ulPcrIndex Index of the PCR to get
 * @param   vLen Length of the value of the PCR
 * @param   value The value of the PCR (out)
 * @return  EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_getPCR(UINT32 ulPcrIndex, UINT32* vLen, BYTE** value);

/**
 * Extend the value of a PCR with a hash
 *
 * @param   ulPcrIndex Index of the PCR to extend
 * @param   hLen Length of the hash
 * @param   hash The hash for extension
 * @param   vLen Length of the value of the PCR ?
 * @param   vValue The value of the PCR (out)    ?
 * @return  EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_extendPCR(UINT32 ulPcrIndex, UINT32 hLen, BYTE * hash, UINT32* vLen, BYTE** vValue);

/**
 * Extend the value of a PCR with a hash
 *
 * @param   dLen
 * @param   Data
 * @param   vLen
 * @param   ValidationData
 * @return  EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_getQuote(UINT32 *dLen, BYTE ** Data, UINT32* vLen, BYTE** ValidationData);

/**
 * Get the signature of a hash
 *
 * @param ulHashValueLength Lenght of the hash
 * @param rgbHashValue The hash to be signed
 * @param sLen The length of the resulting signature
 * @param signature The signature of the hash (out)
 * @return EXIT_SUCCESS if no error occured, EXIT_FAILURE otherwise
 */
int tpm_getSignedHash(UINT32 ulHashValueLength, BYTE* rgbHashValue, UINT32* sLen, BYTE** signature);

#ifdef	__cplusplus
}
#endif

#endif	/* _TSS_UTILS_H */

