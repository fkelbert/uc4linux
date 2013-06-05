#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

/*
 * This method encrypts 'plaintext' of length 'len' and
 * stores the cipher in 'ciphertext'
 *
 * @PARAM:	plaintext	the text to encrypt
 * @PARAM:	ciphertext	where the cipher is stored
 * @PARAM:	len		how many bytes should be encrypted
 */
void encryptText(unsigned char *plaintext, unsigned char *ciphertext, size_t len);


/*
 * This method decrypts a ciphertext 'ciphertext' of
 * length 'len' and stores the result in 'ciphertext'
 *
 * @PARAM:	ciphertext	the ciphertext to decrypt
 * @PARAM:	len		how many bytes should be decrypted
 */
void decryptText(unsigned char *text, size_t len);


/*
 * This method returns the key
 *
 * @RETURN:	the key
 */
unsigned char *viewKey();

/*
 * This method creates a key
 * @PARAM:	password	the password which the key should be generated from
 * @PARAM:	keyLen		the length of the generated key
 *
 * @RETURN:	the generated key
 */
unsigned char *createKey(char *password, int keyLen);

/*
 * This method sets the key
 * @PARAM:	key	the generated key
 */
void setKey(unsigned char *key);

#endif // ENCRYPTION_H_
