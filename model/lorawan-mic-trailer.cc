#include "lorawan-mic-trailer.h"
#include "ns3/log.h"

namespace ns3{
namespace lorawan{
 
NS_LOG_COMPONENT_DEFINE ("LorawanMICTrailer");
    
LorawanMICTrailer::LorawanMICTrailer ()
{
}

LorawanMICTrailer::~LorawanMICTrailer ()
{
}

TypeId
LorawanMICTrailer::GetTypeId (void)
{
    static TypeId tid = TypeId ("LorawanMICTrailer")
        .SetParent<Trailer> ()
        .AddConstructor<LorawanMICTrailer> ()
    ;
    
    return tid;
}

TypeId
LorawanMICTrailer::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t
LorawanMICTrailer::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION_NOARGS ();
    
    return 4;
}

void
LorawanMICTrailer::Serialize (Buffer::Iterator start) const
{
    NS_LOG_FUNCTION_NOARGS ();
    
    /*  move iterator to start of trailer before writing    */
    start.Prev(GetSerializedSize ());
    
    start.WriteU32 (m_mic);
    
    return;
}

uint32_t
LorawanMICTrailer::Deserialize (Buffer::Iterator end)
{
    uint32_t trailer_data;
    
    NS_LOG_FUNCTION_NOARGS ();
    
    /*  move iterator to start of trailer before reading    */
    end.Prev(GetSerializedSize ());
    
    trailer_data = end.ReadU32 ();
    
    m_mic = trailer_data;
    
    return GetSerializedSize ();
}

void
LorawanMICTrailer::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION_NOARGS ();
    
    os << "MIC=" << m_mic << std::endl;
    
    return;
}

void
LorawanMICTrailer::SetMIC (uint32_t newMIC)
{
    NS_LOG_FUNCTION_NOARGS ();
    
    m_mic = newMIC;

    return;
}

uint32_t
LorawanMICTrailer::GetMIC (void) const
{
    NS_LOG_FUNCTION_NOARGS ();
    
    return m_mic;
}

void
LorawanMICTrailer::leftshift_1bit (uint8_t in[16], uint8_t out[16])
{
    int i;
    uint8_t overflow; /*  from byte to the right  */
    
    for (i = 15, overflow = 0;i >= 0;i--)
    {
        out[i] = in[i] << 1;
        out[i] |= overflow; 
        overflow = (in[i] & 0x80)?1:0;
    }
    
    return;
}

void
LorawanMICTrailer::xor_128 (uint8_t in1[16], uint8_t in2[16], uint8_t out[16])
{
    unsigned int i;
    
    for (i = 0;i < 16;i++)
    {
        out[i] = in1[i] ^ in2[i];
    }
    
    return;
}

void
LorawanMICTrailer::aes128_cmac_generate_subkeys (uint8_t K[16], uint8_t K1[16], uint8_t K2[16])
{
    uint8_t tmp[16];
    uint8_t L[16];
    uint8_t Rb[16];
    unsigned int i;
    
    for (i = 0;i < 16;i++)
    {
        tmp[i] = 0;
        Rb[i] = 0;
    }
    
    Rb[15] = 0x87;
    
    aes128 (K, tmp, L);
    
    if ((L[0] & 0x80) == 0)   /*  MSB of L    */
    {
        leftshift_1bit (L, K1);
    }
    else
    {
        leftshift_1bit (L, tmp);
        xor_128 (tmp, Rb, K1);
    }
    
    if ((K1[0] & 0x80) == 0)
    {
        leftshift_1bit (K1, K2);
    }
    else
    {
        leftshift_1bit(K1, tmp);
        xor_128 (tmp, Rb, K2);
    }
    
    return;
}

void
LorawanMICTrailer::padding_128 (uint8_t *in, uint8_t out[16], unsigned int length)
{
    unsigned int i;
    
    for (i = 0;i < 16;i++)
    {
        if (i < length)
        {
            out[i] = in[i];
        }
        else if (i == length)
        {
            /*  first byte to pad   */
            out[i] = 0x80;
        }
        else
        {
            out[i] = 0x00;
        }
    }
    
    return;
}

void
LorawanMICTrailer::aes128 (uint8_t K[16], uint8_t M[16], uint8_t O[16])
{
    const uint8_t Nr = 10;  /*  number of rounds 10 for 128bit key  */
    //const uint8_t Nb = 4;
    uint8_t state[4][4];
    uint8_t w[44][4];
    unsigned int i, j;
    
    /*  taken from NIST publication for AES: https://www.nist.gov/publications/advanced-encryption-standard-aes*/
    const uint8_t sbox[16][16] = {
        {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
        {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
        {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
        {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
        {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
        {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
        {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
        {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
        {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
        {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
        {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
        {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
        {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
        {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
        {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
        {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}
    };
    
    /*  move input into state matrix    */
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            state[j][i] = M[(4 * i) + j];
        }
    }
    
    /*  cipher algorithm    */
    aes128_addroundkeys(state, w, 0);
    
    for (i = 1;i < Nr;i++)
    {
        aes128_subbytes(state, sbox);
        aes128_shiftrows(state);
        aes128_mixcolumns(state);
        aes128_addroundkeys(state, w, i);
    }
    
    aes128_subbytes(state, sbox);
    aes128_shiftrows(state);
    aes128_addroundkeys(state, w, Nr);
    
    /*  move state matrix to output */
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            O[(4 * i) + j] = state[j][i];
        }
    }
    
    return;
}

void
LorawanMICTrailer::aes128_subbytes (uint8_t state[4][4], const uint8_t sbox[16][16])
{
    unsigned int i, j, row, col;
    
    
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            col = state[i][j] & 0x0f; /*    lower 4 bits    */
            row = state[i][j] >> 4;   /*  upper 4 bits  */
            
            /*  update  state val   */
            state[i][j] = sbox[row][col];
        }
    }
    
    return;
}

void
LorawanMICTrailer::aes128_shiftrows (uint8_t state[4][4])
{
    unsigned int i;
    uint8_t temp;
    
    /*  first row doesn't shift */
    
    /*  second row: shift bytes left once*/
    temp = state[1][0];
    
    for (i = 0;i < 3;i++)
    {
        state[1][i] = state[1][i + 1];
    }
    
    state[1][3] = temp;
    
    /*  third row: shift bytes left twice -> swap bytes two spaces apart    */
    
    for (i = 0;i < 2;i++)
    {
        temp = state[2][i];
        state[2][i] = state[2][i + 2];
        state[2][i + 2] = temp;
    }
    
    /*  fourth row: shift bytes left thrice -> shift left once  */
    temp = state[3][3];
    
    for (i = 3;i > 0;i--)
    {
        state[3][i] =  state[3][i - 1];
    }
    
    state[3][0] = temp;
    
    return;
}

void
LorawanMICTrailer::aes128_mixcolumns (uint8_t state[4][4])
{
    unsigned int i, j;
    uint8_t temp[4];
    
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            temp[j] = gfmul (state[j][i], 0x02) ^ gfmul (state[(j + 1) % 4][i], 0x03) ^ state[(j + 2) % 4][i] ^ state[(j + 3) % 4][i];
        }
        
        /*  update col  */
        for (j = 0;j < 4;j++)
        {
            state[j][i] = temp[j];
        }
    }
    
    return;
}

uint8_t
LorawanMICTrailer::gfmul (uint8_t x, uint8_t y)
{
    uint16_t mulres, x16;
    unsigned int i;
    
    /*  cycle through bits and xor  */
    for (i = 0, mulres = 0, x16 = x;i < 8;i++)
    {
        if (y & 0x01)   /*  check rightmost bit   */
        {
            mulres ^= (x16 << i);
        }
        
        y >>= 1;
    }
    
    /*  modulo result with x^8 + x^4 + x^3 + x + 1  */
    for (i = 0;i < 8;i++)
    {
        if (mulres & (0x8000 >> i))
        {
            mulres ^= (((mulres & (0x8000 >> i)) >> 8) * 0x001b);
        }
    }
    
    return (uint8_t)(mulres & 0x00ff);  /*  return rightmost byte   */
}

void 
LorawanMICTrailer::aes128_keyexpansion (uint8_t K[16], uint8_t w[44][4], const uint8_t sbox[16][16])
{
    unsigned int i, j;
    uint8_t temp[4];
    
    const uint8_t  rcon[11][4] = {
        {0x8d, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0x00, 0x00},
        {0x04, 0x00, 0x00, 0x00},
        {0x08, 0x00, 0x00, 0x00},
        {0x10, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00},
        {0x40, 0x00, 0x00, 0x00},
        {0x80, 0x00, 0x00, 0x00},
        {0x1b, 0x00, 0x00, 0x00},
        {0x36, 0x00, 0x00, 0x00}
    };
    
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            w[i][j] = K[(i * 4) + j];
        }
    }
    
    for (i = 4;i < 44;i++)
    {
        for (j = 0;j < 4;j++)
        {
            temp[j] = w[i - 1][j];
        }
        
        if (i % 4 == 0)
        {
            rotword (temp);
            subword (temp, sbox);
            
            for (j = 0;j < 4;j++)
            {
                temp[j] ^= rcon[i / 4][j];
            }
        }
        /*  condition in original algorithm for Nk > 6 not included as Nk = 4 for aes128    */
        
        for (j = 0;j < 4;j++)
        {
            w[i][j] = w[i - 4][j] ^ temp[j];
        }
    }
    
    return;
}

void
LorawanMICTrailer::aes128_addroundkeys (uint8_t state[4][4], uint8_t w[44][4], unsigned int round)
{
    unsigned int i, j;
    
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            state[j][i] ^= w[(4 * round) + i][j];
        }
    }
    
    return;
}

void
LorawanMICTrailer::rotword (uint8_t word[4])
{
    uint8_t temp;
    unsigned int i;
    
    /*  shift bytes left once   */
    temp = word[0];
    
    for (i = 0;i < 3;i++)
    {
        word[i] = word[i + 1];
    }
    
    word[3] = temp;
    
    return;
}

void
LorawanMICTrailer::subword (uint8_t word[4], const uint8_t sbox[16][16])
{
    uint8_t row, col, i;
    
    for (i = 0;i < 4;i++)
    {
        col = word[i] & 0x0f;
        row = word[i] >> 4;
        
        word[i] = sbox[row][col];
    }
    
    return;
}

uint32_t
LorawanMICTrailer::aes128_cmac_4 (uint8_t xNwkSIntKey[16], uint8_t *Bx_msg, uint8_t len)
{
    uint8_t Mlast[16], padded[16], X[16], Y[16];
    uint8_t K1[16], K2[16];
    unsigned int i, n;
    bool flag;
    uint32_t mic;
    
    aes128_cmac_generate_subkeys (xNwkSIntKey, K1, K2);
    
    n = (len + 15) / 16;
    
    if (n == 0)
    {
        n = 1;
        flag = false;
    }
    else if (len % 16 == 0)
    {
        flag = true;
    }
    else 
    {
        flag = false;
    }
    
    /*  handle last block   */
    if (flag == true)
    {
        xor_128 (&(Bx_msg[16 * (n - 1)]), K1, Mlast);
    }
    else
    {
        padding_128 (&(Bx_msg[16 * (n - 1)]), padded, len % 16);
        xor_128 (padded, K2, Mlast);
    }
    
    for (i = 0;i < 16;i++)
    {
        X[i] = 0;
    }
    
    for (i = 0;i < (n - 1);i++)
    {
        xor_128 (X, &(Bx_msg[16 * i]), Y);
        aes128 (xNwkSIntKey, Y, X);
    }
    
    xor_128 (X, Mlast, Y);
    aes128 (xNwkSIntKey, Y, X);
    
    /*  mic is first 4 octets of entire code */
    
    for (i = 0, mic = 0;i < 4;i++)
    {
        mic |= X[i];
        mic <<= 8;
    }
    
    return mic;
}

uint32_t
LorawanMICTrailer::CalcMIC (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t xNwkSIntKey[16])
{
    uint8_t B0_msg[msgLen + 16];
    uint32_t mic;
    unsigned int j;
    
    /*  concatenate Bx and msg  */
    
    for (j = 0;j < 16;j++)
    {
        B0_msg[j] = B0[j];
    }
    
    for (j = 0;j < msgLen;j++)
    {
        B0_msg[j + 16] = msg[j];
    }
    
    mic = aes128_cmac_4(xNwkSIntKey, B0_msg, msgLen + (uint8_t)16);
    
    return mic;
}

/*  for LoRaWAN 1.1 Network Servers UL MIC  */
uint32_t
LorawanMICTrailer::CalcMIC_1_1_UL (uint8_t msgLen, uint8_t *msg, uint8_t B0[16], uint8_t B1[16], uint8_t SNwkSIntKey[16], uint8_t FNwkSIntKey[16])
{
    uint32_t mic_f, mic_s, mic;
    
    mic_f = CalcMIC (msgLen, msg, B0, FNwkSIntKey);
    mic_s = CalcMIC (msgLen, msg, B1, SNwkSIntKey);
    
    mic = 0;
    mic |= (mic_s & 0xffff0000);    /*  first two bytes of each */
    mic |= ((mic_f & 0xffff0000) >> 16);
    
    return mic;
}

void
LorawanMICTrailer::GenerateB0DL (uint8_t B0[16], uint16_t ConfFCnt, uint32_t DevAddr, uint32_t xFCntDwn, uint8_t msgLen)
{
    B0[0] = 0x49;
    B0[1] = (uint8_t)(ConfFCnt >> 8);
    B0[2] = (uint8_t)(ConfFCnt & 0x00ff);
    B0[3] = 0x00;
    B0[4] = 0x00;
    B0[5] = 0x01;
    B0[6] = (uint8_t)(DevAddr >> 24);
    B0[7] = (uint8_t)((DevAddr & 0x00ff0000) >> 16);
    B0[8] = (uint8_t)((DevAddr & 0x0000ff00) >> 8);
    B0[9] = (uint8_t)(DevAddr & 0x000000ff);
    B0[10] = (uint8_t)(xFCntDwn >> 24);
    B0[11] = (uint8_t)((xFCntDwn & 0x00ff0000) >> 16);
    B0[12] = (uint8_t)((xFCntDwn & 0x0000ff00) >> 8);
    B0[13] = (uint8_t)(xFCntDwn & 0x000000ff);
    B0[14] = 0x00;
    B0[15] = msgLen;
    
    return;
}

void
LorawanMICTrailer::GenerateB0UL (uint8_t B0[16], uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen)
{
    B0[0] = 0x49;
    B0[1] = 0x00;
    B0[2] = 0x00;
    B0[3] = 0x00;
    B0[4] = 0x00;
    B0[5] = 0x00;
    B0[6] = (uint8_t)(DevAddr >> 24);
    B0[7] = (uint8_t)((DevAddr & 0x00ff0000) >> 16);
    B0[8] = (uint8_t)((DevAddr & 0x0000ff00) >> 8);
    B0[9] = (uint8_t)(DevAddr & 0x000000ff);
    B0[10] = (uint8_t)(FCntUp >> 24);
    B0[11] = (uint8_t)((FCntUp & 0x00ff0000) >> 16);
    B0[12] = (uint8_t)((FCntUp & 0x0000ff00) >> 8);
    B0[13] = (uint8_t)(FCntUp & 0x000000ff);
    B0[14] = 0x00;
    B0[15] = msgLen;
    
    return;
}

void
LorawanMICTrailer::GenerateB1UL (uint8_t B1[16], uint16_t ConfFCnt, uint8_t TxDr, uint8_t TxCh, uint32_t DevAddr, uint32_t FCntUp, uint8_t msgLen)
{
    B1[0] = 0x49;
    B1[1] = (uint8_t)(ConfFCnt >> 8);
    B1[2] = (uint8_t)(ConfFCnt & 0x00ff);
    B1[3] = TxDr;
    B1[4] = TxCh;
    B1[5] = 0x00;
    B1[6] = (uint8_t)(DevAddr >> 24);
    B1[7] = (uint8_t)((DevAddr & 0x00ff0000) >> 16);
    B1[8] = (uint8_t)((DevAddr & 0x0000ff00) >> 8);
    B1[9] = (uint8_t)(DevAddr & 0x000000ff);
    B1[10] = (uint8_t)(FCntUp >> 24);
    B1[11] = (uint8_t)((FCntUp & 0x00ff0000) >> 16);
    B1[12] = (uint8_t)((FCntUp & 0x0000ff00) >> 8);
    B1[13] = (uint8_t)(FCntUp & 0x000000ff);
    B1[14] = 0x00;
    B1[15] = msgLen;
    
    return;
}

}
}
