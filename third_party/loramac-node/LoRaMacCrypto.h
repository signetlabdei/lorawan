/*!
 * \file      LoRaMacCrypto.h
 *
 * \brief     LoRa MAC layer cryptographic functionality implementation
 *
 * \copyright Revised BSD License, see LICENSE file in this directory.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 *
 * addtogroup LORAMAC
 * \{
 *
 */
#ifndef __LORAMAC_CRYPTO_H__
#define __LORAMAC_CRYPTO_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Frame direction definition for uplink communications
 */
#define UPLINK 0

/*
 * Frame direction definition for downlink communications
 */
#define DOWNLINK 1

/*!
 * Start value for multicast keys enumeration
 */
#define LORAMAC_CRYPTO_MULTICAST_KEYS 127

/*!
 * Secure-element keys size in bytes
 */
#define SE_KEY_SIZE 16

/*!
 * Secure-element EUI size in bytes
 */
#define SE_EUI_SIZE 8

/*!
 * Secure-element pin size in bytes
 */
#define SE_PIN_SIZE 4

/*!
 * Number of supported crypto keys for the soft-se
 */
#define NUM_OF_KEYS 23

/*!
 * LoRaMac Key identifier
 */
typedef enum eKeyIdentifier {
  /*!
     * Application root key
     */
  APP_KEY = 0,
  /*!
     * Network root key
     */
  NWK_KEY,
  /*!
     * Join session integrity key
     */
  J_S_INT_KEY,
  /*!
     * Join session encryption key
     */
  J_S_ENC_KEY,
  /*!
     * Forwarding Network session integrity key
     */
  F_NWK_S_INT_KEY,
  /*!
     * Serving Network session integrity key
     */
  S_NWK_S_INT_KEY,
  /*!
     * Network session encryption key
     */
  NWK_S_ENC_KEY,
  /*!
     * Application session key
     */
  APP_S_KEY,
  /*!
     * Multicast root key
     */
  MC_ROOT_KEY,
  /*!
     * Multicast key encryption key
     */
  MC_KE_KEY = LORAMAC_CRYPTO_MULTICAST_KEYS,
  /*!
     * Multicast root key index 0
     */
  MC_KEY_0,
  /*!
     * Multicast Application session key index 0
     */
  MC_APP_S_KEY_0,
  /*!
     * Multicast Network session key index 0
     */
  MC_NWK_S_KEY_0,
  /*!
     * Multicast root key index 1
     */
  MC_KEY_1,
  /*!
     * Multicast Application session key index 1
     */
  MC_APP_S_KEY_1,
  /*!
     * Multicast Network session key index 1
     */
  MC_NWK_S_KEY_1,
  /*!
     * Multicast root key index 2
     */
  MC_KEY_2,
  /*!
     * Multicast Application session key index 2
     */
  MC_APP_S_KEY_2,
  /*!
     * Multicast Network session key index 2
     */
  MC_NWK_S_KEY_2,
  /*!
     * Multicast root key index 3
     */
  MC_KEY_3,
  /*!
     * Multicast Application session key index 3
     */
  MC_APP_S_KEY_3,
  /*!
     * Multicast Network session key index 3
     */
  MC_NWK_S_KEY_3,
  /*!
     * Zero key for slot randomization in class B
     */
  SLOT_RAND_ZERO_KEY,
  /*!
     * No Key
     */
  NO_KEY,
} KeyIdentifier_t;

/*!
 * Key structure definition for the soft-se
 */
typedef struct sKey
{
  /*!
     * Key identifier
     */
  KeyIdentifier_t KeyID;
  /*!
     * Key value
     */
  uint8_t KeyValue[SE_KEY_SIZE];
} Key_t;

typedef struct sSecureElementNvCtx
{
  /*!
     * DevEUI storage
     */
  uint8_t DevEui[SE_EUI_SIZE];
  /*!
     * Join EUI storage
     */
  uint8_t JoinEui[SE_EUI_SIZE];
  /*!
     * Pin storage
     */
  uint8_t Pin[SE_PIN_SIZE];
  /*!
     * The key list is required for the soft-se only. All other secure-elements
     * handle the storage on their own.
     */
  Key_t KeyList[NUM_OF_KEYS];
  /*!
     * CRC32 value of the SecureElement data structure.
     */
  uint32_t Crc32;
} SecureElementNvmData_t;

/*!
 * Return values.
 */
typedef enum eSecureElementStatus {
  /*!
     * No error occurred
     */
  SECURE_ELEMENT_SUCCESS = 0,
  /*!
     * CMAC does not match
     */
  SECURE_ELEMENT_FAIL_CMAC,
  /*!
     * Null pointer exception
     */
  SECURE_ELEMENT_ERROR_NPE,
  /*!
     * Invalid key identifier exception
     */
  SECURE_ELEMENT_ERROR_INVALID_KEY_ID,
  /*!
     * Invalid LoRaWAN specification version
     */
  SECURE_ELEMENT_ERROR_INVALID_LORAWAM_SPEC_VERSION,
  /*!
     * Incompatible buffer size
     */
  SECURE_ELEMENT_ERROR_BUF_SIZE,
  /*!
     * Undefined Error occurred
     */
  SECURE_ELEMENT_ERROR,
  /*!
     * Failed to encrypt
     */
  SECURE_ELEMENT_FAIL_ENCRYPT,
} SecureElementStatus_t;

/*!
 * LoRaMac Crypto Status
 */
typedef enum eLoRaMacCryptoStatus {
  /*!
     * No error occurred
     */
  LORAMAC_CRYPTO_SUCCESS = 0,
  /*!
     * MIC does not match
     */
  LORAMAC_CRYPTO_FAIL_MIC,
  /*!
     * Address does not match
     */
  LORAMAC_CRYPTO_FAIL_ADDRESS,
  /*!
     * JoinNonce was not greater than previous one.
     */
  LORAMAC_CRYPTO_FAIL_JOIN_NONCE,
  /*!
     * RJcount0 reached 2^16-1
     */
  LORAMAC_CRYPTO_FAIL_RJCOUNT0_OVERFLOW,
  /*!
     * FCNT_ID is not supported
     */
  LORAMAC_CRYPTO_FAIL_FCNT_ID,
  /*!
     * FCntUp/Down check failed (new FCnt is smaller than previous one)
     */
  LORAMAC_CRYPTO_FAIL_FCNT_SMALLER,
  /*!
     * FCntUp/Down check failed (duplicated)
     */
  LORAMAC_CRYPTO_FAIL_FCNT_DUPLICATED,
  /*!
     * Not allowed parameter value
     */
  LORAMAC_CRYPTO_FAIL_PARAM,
  /*!
     * Null pointer exception
     */
  LORAMAC_CRYPTO_ERROR_NPE,
  /*!
     * Invalid key identifier exception
     */
  LORAMAC_CRYPTO_ERROR_INVALID_KEY_ID,
  /*!
     * Invalid address identifier exception
     */
  LORAMAC_CRYPTO_ERROR_INVALID_ADDR_ID,
  /*!
     * Invalid LoRaWAN specification version
     */
  LORAMAC_CRYPTO_ERROR_INVALID_VERSION,
  /*!
     * Incompatible buffer size
     */
  LORAMAC_CRYPTO_ERROR_BUF_SIZE,
  /*!
     * The secure element reports an error
     */
  LORAMAC_CRYPTO_ERROR_SECURE_ELEMENT_FUNC,
  /*!
     * Error from parser reported
     */
  LORAMAC_CRYPTO_ERROR_PARSER,
  /*!
     * Error from serializer reported
     */
  LORAMAC_CRYPTO_ERROR_SERIALIZER,
  /*!
     * RJcount1 reached 2^16-1 which should never happen
     */
  LORAMAC_CRYPTO_ERROR_RJCOUNT1_OVERFLOW,
  /*!
     * Undefined Error occurred
     */
  LORAMAC_CRYPTO_ERROR,
} LoRaMacCryptoStatus_t;

class LoRaMacCrypto
{
public:
  LoRaMacCrypto ();
  ~LoRaMacCrypto ();

  /*
   * Prepares B0 block for cmac computation.
   *
   * \param[IN]  msgLen         - Length of message
   * \param[IN]  keyID          - Key identifier
   * \param[IN]  isAck          - True if it is a acknowledge frame ( Sets ConfFCnt in B0 block )
   * \param[IN]  devAddr        - Device address
   * \param[IN]  dir            - Frame direction ( Uplink:0, Downlink:1 )
   * \param[IN]  fCnt           - Frame counter
   * \param[IN/OUT]  b0         - B0 block
   * \retval                    - Status of the operation
   */
  LoRaMacCryptoStatus_t PayloadEncrypt (uint8_t *buffer, int16_t size, KeyIdentifier_t keyID,
                                        uint32_t address, uint8_t dir, uint32_t frameCounter);

  /*
   * Computes cmac with adding B0 block in front.
   *
   *  cmac = aes128_cmac(keyID, B0 | msg)
   *
   * \param[IN]  msg            - Message to compute the integrity code
   * \param[IN]  len            - Length of message
   * \param[IN]  keyID          - Key identifier
   * \param[IN]  isAck          - True if it is a acknowledge frame ( Sets ConfFCnt in B0 block )
   * \param[IN]  devAddr        - Device address
   * \param[IN]  dir            - Frame direction ( Uplink:0, Downlink:1 )
   * \param[IN]  fCnt           - Frame counter
   * \param[OUT] cmac           - Computed cmac
   * \retval                    - Status of the operation
   */
  LoRaMacCryptoStatus_t ComputeCmacB0 (uint8_t *msg, uint16_t len, KeyIdentifier_t keyID,
                                       bool isAck, uint8_t dir, uint32_t devAddr, uint32_t fCnt,
                                       uint32_t *cmac);

private:
  /*!
   * Encrypt a buffer
   *
   * \param[IN]  buffer         - Data buffer
   * \param[IN]  size           - Data buffer size
   * \param[IN]  keyID          - Key identifier to determine the AES key to be used
   * \param[OUT] encBuffer      - Encrypted buffer
   * \retval                    - Status of the operation
   */
  SecureElementStatus_t SecureElementAesEncrypt (uint8_t *buffer, uint16_t size,
                                                 KeyIdentifier_t keyID, uint8_t *encBuffer);

  /*
   * Prepares B0 block for cmac computation.
   *
   * \param[IN]  msgLen         - Length of message
   * \param[IN]  keyID          - Key identifier
   * \param[IN]  isAck          - True if it is a acknowledge frame ( Sets ConfFCnt in B0 block )
   * \param[IN]  devAddr        - Device address
   * \param[IN]  dir            - Frame direction ( Uplink:0, Downlink:1 )
   * \param[IN]  fCnt           - Frame counter
   * \param[IN/OUT]  b0         - B0 block
   * \retval                    - Status of the operation
   */
  LoRaMacCryptoStatus_t PrepareB0 (uint16_t msgLen, KeyIdentifier_t keyID, bool isAck, uint8_t dir,
                                   uint32_t devAddr, uint32_t fCnt, uint8_t *b0);

  /*!
   * Computes a CMAC of a message using provided initial Bx block
   *
   * \param[IN]  micBxBuffer    - Buffer containing the initial Bx block
   * \param[IN]  buffer         - Data buffer
   * \param[IN]  size           - Data buffer size
   * \param[IN]  keyID          - Key identifier to determine the AES key to be used
   * \param[OUT] cmac           - Computed cmac
   * \retval                    - Status of the operation
   */
  SecureElementStatus_t SecureElementComputeAesCmac (uint8_t *micBxBuffer, uint8_t *buffer,
                                                     uint16_t size, KeyIdentifier_t keyID,
                                                     uint32_t *cmac);

  /*
   * Computes a CMAC of a message using provided initial Bx block
   *
   *  cmac = aes128_cmac(keyID, blocks[i].Buffer)
   *
   * \param[IN]  micBxBuffer    - Buffer containing the initial Bx block
   * \param[IN]  buffer         - Data buffer
   * \param[IN]  size           - Data buffer size
   * \param[IN]  keyID          - Key identifier to determine the AES key to be used
   * \param[OUT] cmac           - Computed cmac
   * \retval                    - Status of the operation
   */
  SecureElementStatus_t ComputeCmac (uint8_t *micBxBuffer, uint8_t *buffer, uint16_t size,
                                     KeyIdentifier_t keyID, uint32_t *cmac);

  /*
   * Gets key item from key list.
   *
   * \param[IN]  keyID          - Key identifier
   * \param[OUT] keyItem        - Key item reference
   * \retval                    - Status of the operation
   */
  SecureElementStatus_t GetKeyByID (KeyIdentifier_t keyID, Key_t **keyItem);

  SecureElementNvmData_t m_SeNvm;
};

#endif // __LORAMAC_CRYPTO_H__
