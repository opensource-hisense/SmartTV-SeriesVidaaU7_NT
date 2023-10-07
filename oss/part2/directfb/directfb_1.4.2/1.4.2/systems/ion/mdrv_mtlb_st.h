#ifndef _MDRV_MTLB_ST_H_
#define _MDRV_MTLB_ST_H_
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum {
    E_MTLB_MIU_INVALID = -1,
    E_MTLB_MIU_0 = 0x1,
    E_MTLB_MIU_1 = 0x2,
    E_MTLB_MIU_2 = 0x4,
    E_MTLB_MIU_NUM = 3
} EN_MTLB_Miu;

typedef enum {
    E_MTLB_CLIENT_INVALIDE = 0,
    E_MTLB_GE_Src = 0x1,
    E_MTLB_GE_Dest = 0x2,
    E_MTLB_GOP = 0x4,
    E_MTLB_GPD = 0x8,
    E_MTLB_JPD = 0x10,
    E_MTLB_MFE = 0x20,
    E_MTLB_DIP = 0x40,
    E_MTLB_VDEC = 0x80,
    E_MTLB_MVOP = 0x100,
    E_MTLB_XC = 0x200,
    E_MTLB_NUM = 10
} EN_MTLB_TLB_Client;

typedef enum {
    E_MTLB_GOP0 = 0x1,
    E_MTLB_GOP1 = 0x2,
    E_MTLB_GOP2 = 0x4,
    E_MTLB_GOP3 = 0x8,
    E_MTLB_GOP4 = 0x10,
    E_MTLB_GOP5 = 0x20,
    E_MTLB_GOP6 = 0x40 
} EN_MTLB_GOPID;


typedef struct {
    int MIU_Has_TLB;  //MIU has TLB for some ip internally
    int GE_Has_TLB;   //GE has dedicated TLB
    int GOP_Has_TLB;  //GOP has dedicated TLB 

    unsigned int TLB_GOP_IDS; //which GOP can access memory by TLB
    unsigned int TLB_Support_Clients; //tlb support clients except GE&GOP
    
    int GE_DEDICAT_TLB_Switchoff_Support; //ge its dedicated TLB support on-off switch
    int GOP_DEDICAT_TLB_Switchoff_Support; //gop its dedicated TLB support on-off switch
    int GE_Need_CQFlush; //when ge its dedicated TLB on-off switch, whether it need flush command queue 

    int TLB_MAPTO_MIU; //which miu has kernel memory mtlb space may be map to

    unsigned long tlb_table_addr_miu0;  //the offset of tlb  table in MIU0
    unsigned long tlb_table_addr_miu1;  //the offset of tlb  table in MIU1
    unsigned long tlb_table_addr_miu2;  //the offset of tlb  table in MIU2    
} mtlb_hardware_info;

typedef struct {
    unsigned int TLB_Clients_Enabled; //EN_MTLB_TLB_Client
} mtlb_hardware_status;

typedef struct {
    unsigned int clients; //EN_MTLB_TLB_Client
    int gopidmask;  //for specify gop id, when client is gop
    int enable;
} mtlb_tlbclient_enable;


#endif // _MDRV_MTLB_ST_H_

