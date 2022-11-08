#ifndef __LORAWAN_MIC_TRAILER__
#define __LORAWAN_MIC_TRAILER__

#include "ns3/trailer.h"

namespace ns3 {
namespace lorawan {

class LorawanMICTrailer : public Trailer
{
private:
    uint32_t m_mic; /*  the 4 byte mic  */
    
    /*  AES-CMAC funcs (taken from RFC4493)   */
    uint32_t aes128_cmac_4 (uint8_t xNwkSIntKey[16], uint8_t *Bx_msg, uint8_t msgLen);   /*  returns first 4 byte of cmac AKA the MIC  */
    void aes128_cmac_generate_subkeys (uint8_t K[16], uint8_t K0[16], uint8_t K1[16]);
    void leftshift_1bit (uint8_t in[16], uint8_t out[16]);
    void xor_128 (uint8_t in1[16], uint8_t in2[16], uint8_t out[16]);
    void padding_128 (uint8_t *in, uint8_t out[16], unsigned int length);
    
    /*  AES-128 funcs taken from NIST publication   */
    void aes128 (uint8_t K[16], uint8_t M[16], uint8_t O[16]);
    void aes128_subbytes (uint8_t state[4][4], const uint8_t sbox[16][16]);
    void aes128_shiftrows (uint8_t state[4][4]);
    void aes128_mixcolumns (uint8_t state[4][4]);
    void aes128_addroundkeys (uint8_t state[4][4], uint8_t w[44][4], unsigned int round);
    void aes128_keyexpansion(uint8_t K[16], uint8_t w[44][4], const uint8_t sbox[16][16]);
    uint8_t gfmul (uint8_t x, uint8_t y);   /*  GF(2^8) multiplication  */
    void rotword (uint8_t word[4]);
    void subword (uint8_t word[4], const uint8_t sbox[16][16]);
    
public:
    
    LorawanMICTrailer ();
    ~LorawanMICTrailer ();
    
    /*  required funcs for trailer*/
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const ;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator end);
    virtual void Print (std::ostream &os) const;
    
    uint32_t GetMIC (void) const;
    void SetMIC (uint32_t newMIC);
    uint32_t CalcMIC (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t xNwkSIntKey[16]);    /*  Bx and xNwkSIntKey need to be 16 bytes (128 bits) each, uint128_t is not guranteed to be supported */
    uint32_t CalcMIC_1_1_UL(uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t B1[16], uint8_t SNwkSIntKey[16], uint8_t FNwkSIntKey[16]);    /*  special case for UL for 1.1 LoRaWAN network servers */
    bool VerifyMIC (uint8_t msgLen, uint8_t *msg,uint8_t B0[16], uint8_t xNwkSIntKey[16]);
    bool VerifyMIC_1_1_UL (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t B1[16], uint8_t SNwkSIntKey[16], uint8_t FNwkSIntKey[16]);
    
    /*  for making B0 and B1   */
    void GenerateB0DL (uint8_t B0[16], uint16_t ConfFCnt, uint32_t DevAddr, uint32_t xFCntDwn, uint8_t msgLen);
    void GenerateB0UL (uint8_t B0[16], uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen);
    void GenerateB1UL (uint8_t B1[16], uint16_t ConfFCnt, uint8_t TxDr, uint8_t TxCh, uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen);   /*  LoRaWAN 1.1 Network servers use B1 and B0 */
};

}
}

#endif /*   __LORAWAN_MIC_TRAILER__ */
