#ifndef __LORAWAN_MIC_TRAILER__
#define __LORAWAN_MIC_TRAILER__

#include "ns3/trailer.h"

namespace ns3 {
namespace lorawan {

/**
 *  Class used to hold and manage the Message Integrity Code (MIC) trailer
 *  of a LoRaWAN packet
 */
    
class LorawanMICTrailer : public Trailer
{
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
    
    /**
     *  Performs AES-128 to encrypt data
     *
     *  \param  K   the 128-bit key to be used for encryption
     *  \param  M   the 128-bit message to be encrypted
     *  \param  O   the 128-bit container passed to hold the encrypted output
     */
    void aes128 (uint8_t K[16], uint8_t M[16], uint8_t O[16]);
    
    /**
     * Gets the currently stored MIC
     * 
     * \return  the 4-byte MIC currently stored in the trailer
     */
    uint32_t GetMIC (void) const;
    
    /**
     *  Sets the MIC being used in the trailer
     * 
     *  \param  newMIC  the new MIC value to be used by the trailer
     */
    void SetMIC (uint32_t newMIC);
    
    /**
     *  Calculates a MIC for DL and v1.0 LoRaWAN network UL packets
     * 
     *  \param  msgLen      the length (in bytes) of the "msg" parameter
     *  \param  msg         the message being sent in the packet for which the MIC is to be attached to
     *  \param  B0          the B0 (128-bit) MIC computation block (See Figures 18 (DL) and 19 (UL) 
     *                      in LoRaWAN specification)
     *  \param  xNwkSIntKey either the SNwkSIntKey (DL) or FNwkSIntKey (UL)  128-bit keys
     *                      generated for this device
     */
    uint32_t CalcMIC (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t xNwkSIntKey[16]);
    
    /**
     *  Calculates a MIC for v1.1 LoRaWAN networks UL packets
     * 
     *  \param  msgLen      the length in bytes of the "msg" parameter  
     *  \param  msg         the message being sent in the packet for which the MIC is to be attached to
     *  \param  B0          the UL B0 (128-bit) MIC computation block (See Figure 19 in LoRaWAN specification)
     *  \param  B1          the UL B1 (128-bit) MIC computation block (See Figure 20 in LoRaWAN specification)
     *  \param  SNwkSIntKey Serving Network session integrity 128-bit key
     *  \param  FNwkSIntKey Forwarding Network session integrity 128-bit key
     * 
     *  \return the 4-byte MIC used for the packet trailer
     */
    uint32_t CalcMIC_1_1_UL(uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t B1[16], 
                            uint8_t SNwkSIntKey[16], uint8_t FNwkSIntKey[16]);
    
    /**
     *  Used to verify the MIC currently stored in the trailer against that of a newly calculated one
     *  based on the parameters provided (for DL and v1.0 UL)
     * 
     *  \param  msgLen      the length (in bytes) of the "msg" parameter
     *  \param  msg         the message being sent in the packet for which the MIC is to be attached to
     *  \param  B0          the B0 (128-bit) MIC computation block (See Figures 18 (DL) and 19 (UL) 
     *                      in LoRaWAN specification)
     *  \param  xNwkSIntKey either the SNwkSIntKey (DL) or FNwkSIntKey (UL)  128-bit keys
     *                      generated for this device
     * 
     *  \return whether the MIC calculated matches the one stored in the trailer or not
     */
    bool VerifyMIC (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t xNwkSIntKey[16]);
    
    /**
     *  Used to verify the MIC currently stored in the trailer against that of a newly calculated one
     *  based on the parameters provided (for v1.1 UL)
     * 
     *  \param  msgLen      the length in bytes of the "msg" parameter  
     *  \param  msg         the message being sent in the packet for which the MIC is to be attached to
     *  \param  B0          the UL B0 (128-bit) MIC computation block (See Figure 19 in LoRaWAN specification)
     *  \param  B1          the UL B1 (128-bit) MIC computation block (See Figure 20 in LoRaWAN specification)
     *  \param  SNwkSIntKey Serving Network session integrity 128-bit key
     *  \param  FNwkSIntKey Forwarding Network session integrity 128-bit key 
     *  
     *  \return whether the MIC calculated matches the one stored in the trailer or not
     */
    bool VerifyMIC_1_1_UL (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t B1[16],
                           uint8_t SNwkSIntKey[16], uint8_t FNwkSIntKey[16]);
    
    /**
     *  Generates the DL B0 computation block (See Figure 18 in LoRaWAN specification) needed for 
     *  MIC calaculation
     * 
     *  \param  B0          the container for storing the 128-bit B0 block once calculated
     *  \param  ConfFCnt    If the device is connected to a LoRaWAN1.1 Network Server and the 
     *                      ACK bit of the downlink frame is set, meaning this frame is 
     *                      acknowledging an uplink “confirmed” frame, then ConfFCnt is the frame 
     *                      counter value modulo 2^16 of the “confirmed” uplink frame that is 
     *                      being acknowledged. In all other cases ConfFCnt = 0x0000
     *  \param  DevAddr     the end-device address
     *  \param  xFCntDwn    either AFCntDown or NFCntDown
     *  \param  msgLen      the length of the message being sent in the packet for which the 
     *                      MIC is to be attached to
     */
    void GenerateB0DL (uint8_t B0[16], uint16_t ConfFCnt, uint32_t DevAddr, uint32_t xFCntDwn, uint8_t msgLen);
    
    /**
     *  Generates the UL B0 computation block (See Figure 19 in LoRaWAN specification) needed for 
     *  MIC calaculation
     * 
     *  \param  B0      the container for storing the 128-bit B0 block once calculated
     *  \param  DevAddr the end-device address
     *  \param  FCntUp  current UL frame counter value
     *  \param  msgLen  the length of the message being sent in the packet for which the 
     *                      MIC is to be attached to
     */
    void GenerateB0UL (uint8_t B0[16], uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen);
    
    /**
     *  Generates the UL B1 computation block (See Figure 20 in LoRaWAN specification) needed for 
     *  MIC calaculation
     * 
     *  \param  B1          the container for storing the 128-bit B1 block once calculated
     *  \param  ConfFCnt    If the ACK bit of the uplink frame is set, meaning this frame is 
     *                      acknowledging a downlink “confirmed” frame, then ConfFCnt is 
     *                      the frame counter value modulo 2^16 of the “confirmed” downlink
     *                      frame that is being acknowledged. In all other cases ConfFCnt = 0x0000
     *  \param  TxDr        the data rate that will be used for the UL transmission
     *  \param  TxCh        the index of the channel being used
     *  \param  DevAddr     the end-device address
     *  \param  FCntUp      current UL frame coutner value
     *  \param  msgLen      the length of the message being sent in the packet for which the 
     *                      MIC is to be attached to
     * 
     */
    void GenerateB1UL (uint8_t B1[16], uint16_t ConfFCnt, uint8_t TxDr, uint8_t TxCh, 
                       uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen);
    
private:
    uint32_t m_mic; /*  the 4 byte mic  */
    
    /*  AES-CMAC funcs (taken from RFC4493)   */
    uint32_t aes128_cmac_4 (uint8_t xNwkSIntKey[16], uint8_t *Bx_msg, uint8_t msgLen);   /*  returns first 4 byte of cmac AKA the MIC  */
    void aes128_cmac_generate_subkeys (uint8_t K[16], uint8_t K0[16], uint8_t K1[16]);
    void leftshift_1bit (uint8_t in[16], uint8_t out[16]);
    void xor_128 (uint8_t in1[16], uint8_t in2[16], uint8_t out[16]);
    void padding_128 (uint8_t *in, uint8_t out[16], unsigned int length);
    
    /*  AES-128 funcs taken from NIST publication   */
    void aes128_subbytes (uint8_t state[4][4], const uint8_t sbox[16][16]);
    void aes128_shiftrows (uint8_t state[4][4]);
    void aes128_mixcolumns (uint8_t state[4][4]);
    void aes128_addroundkeys (uint8_t state[4][4], uint8_t w[44][4], unsigned int round);
    void aes128_keyexpansion(uint8_t K[16], uint8_t w[44][4], const uint8_t sbox[16][16]);
    uint8_t gfmul (uint8_t x, uint8_t y);   /*  GF(2^8) multiplication  */
    void rotword (uint8_t word[4]);
    void subword (uint8_t word[4], const uint8_t sbox[16][16]);

};

}
}

#endif /*   __LORAWAN_MIC_TRAILER__ */
