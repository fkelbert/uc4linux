/* parts of the AES encryption code is
 * from http://www.koders.com/c/fid77206A4408995566267D354F3F5A9B77DCD5E957.aspx
 */
#include <stdlib.h>

#include <gcrypt.h>	/* used for encryption / decryption */
#include "./encryption.h"

gcry_cipher_hd_t cipher1;
unsigned char *elog_crypt_key = NULL;
unsigned char iv[16] = { 'a', 'g', 'z', 'e', 'Q', '5', 'E', '7', 'c', '+', '*', 'G', '1', 'D', 'u', '=' };
unsigned char ctr[16] = { 'd', 'g', '4', 'e', 'J', '5', '3', 'l', 'c', '-', '!', 'G', 'z', 'A', 'z', '=' };

/*
 * This method encrypts 'plaintext' of length 'len' and
 * stores the cipher in 'ciphertext'
 *
 * @PARAM:	plaintext	the text to encrypt
 * @PARAM:	ciphertext	where the cipher is stored
 * @PARAM:	len		how many bytes should be encrypted
 */
void encryptText(unsigned char *plaintext, unsigned char *ciphertext, size_t len)
{
	gcry_error_t error1 = gcry_cipher_open(&cipher1, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);
	error1 = gcry_cipher_setiv(cipher1, &iv, sizeof( iv ));
	error1 = gcry_cipher_setctr(cipher1, &ctr, sizeof( ctr ));
	error1 = gcry_cipher_setkey(cipher1, viewKey(), 32);	/* AES 256/8 = 32 */
	gcry_cipher_encrypt(cipher1, &ciphertext[0], len, plaintext, len );
	gcry_cipher_close(cipher1);
}


/*
 * This method decrypts a ciphertext 'ciphertext' of
 * length 'len' and stores the result in 'ciphertext'
 *
 * @PARAM:	ciphertext	the ciphertext to decrypt
 * @PARAM:	len		how many bytes should be decrypted
 */
void decryptText(unsigned char *ciphertext, size_t len)
{
	gcry_error_t error1 = gcry_cipher_open(&cipher1, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);
	error1 = gcry_cipher_setiv(cipher1, &iv, sizeof( iv ));
	error1 = gcry_cipher_setctr(cipher1, &ctr, sizeof( ctr ));
	error1 = gcry_cipher_setkey(cipher1, elog_crypt_key, 32);	/* AES 256/8 = 32 */
	gcry_cipher_decrypt(cipher1, ciphertext, len, NULL, 0);
	gcry_cipher_close(cipher1);
}


/*
 * This method creates a key of length 'keyLen'
 * from a password 'password'
 *
 * @PARAM:	password	the password used to create the key
 * @PARAM:	keyLen		how many bytes the key should be
 *
 * @RETURN:	the key
 */
unsigned char *createKey(char *password, int keyLen)
{
	if (elog_crypt_key != NULL)
		free(elog_crypt_key);
	unsigned char *key = elog_crypt_key = (unsigned char *) malloc(sizeof(unsigned char) * (keyLen / 8));



	int c;

	for (c = 0; c < keyLen / 8; ++c)
		key[c] = 0;


	int powersof2[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	int len_str = strlen(password);
	key[(keyLen / 8) - 1] = len_str;
	int size_str = len_str * (sizeof *password) * 8;
	int skip = (keyLen - 5) / size_str;
	//^^num of bits..
	int current = 0;
	for (c = 0; c < len_str; ++c) {
		int j;
		int remainder = password[c];
		for (j = 0; j < 8; ++j) {
			int mod = remainder % powersof2[7 - j];

			if (mod != remainder)	//if it divided for 1
			{
				remainder = mod;
				int i = current / 8;
				int pow = current % 8;
				key[i] += powersof2[7 - pow];
			}
			current += skip;
		}

	}
	return key;
}


/*
 * This method returns the key
 *
 * @RETURN:	the key
 */
unsigned char *viewKey()
{
	return elog_crypt_key;
}


/*
 * This method sets the key
 *
 * @PARAM:	key	the key to set
 */
void setKey( unsigned char *key )
{
	elog_crypt_key = key;
}
