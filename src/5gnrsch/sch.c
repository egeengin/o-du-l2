/*******************************************************************************
################################################################################
#   Copyright (c) [2017-2019] [Radisys]                                        #
#                                                                              #
#   Licensed under the Apache License, Version 2.0 (the "License");            #
#   you may not use this file except in compliance with the License.           #
#   You may obtain a copy of the License at                                    #
#                                                                              #
#       http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                              #
#   Unless required by applicable law or agreed to in writing, software        #
#   distributed under the License is distributed on an "AS IS" BASIS,          #
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#   See the License for the specific language governing permissions and        #
#   limitations under the License.                                             #
################################################################################
*******************************************************************************/

/************************************************************************
 
     Name:     sch.c
  
     Type:     C source file
  
     Desc:     C source code for scheduler fucntions
  
     File:     sch.c
  
**********************************************************************/

/** @file sch.c
@brief This file implements the schedulers main access to MAC layer code.
*/
#include "common_def.h"	
#include "du_app_mac_inf.h"
#include "lrg.h"
#include "tfu.h"
#include "du_log.h"
#include "rgr.h"
#include "rg_sch_inf.h"
#include "rg_sch.h"

#include "tfu.x"           /* TFU types */
#include "lrg.x"           /* layer management typedefs for MAC */
#include "rgr.x"           /* layer management typedefs for MAC */
#include "rg_sch_inf.x"         /* typedefs for Scheduler */
#include "mac_sch_interface.h"
#include "sch.h"
#include "sch_utils.h"

void SchFillCfmPst(Pst *reqPst,Pst *cfmPst,RgMngmt *cfm);

/* local defines */
SchCellCfgCfmFunc SchCellCfgCfmOpts[] = 
{
   packSchCellCfgCfm,     /* LC */
   MacProcSchCellCfgCfm,  /* TC */
   packSchCellCfgCfm      /* LWLC */
};

SchSliceCfgRspFunc SchSliceCfgRspOpts[] =
{
   packSchSliceCfgRsp,     /* LC */
   MacProcSchSliceCfgRsp,  /* TC */
   packSchSliceCfgRsp      /* LWLC */

};

SchSliceReCfgRspFunc SchSliceReCfgRspOpts[] =
{
   packSchSliceReCfgRsp,     /* LC */
   MacProcSchSliceReCfgRsp,  /* TC */
   packSchSliceReCfgRsp      /* LWLC */
};
/**
 * @brief Task Initiation function. 
 *
 * @details
 *
 *     Function : schActvInit
 *     
 *     This function is supplied as one of parameters during MAC's 
 *     task registration. MAC will invoke this function once, after
 *     it creates and attaches this TAPA Task to a system task.
 *     
 *  @param[in]  Ent Entity, the entity ID of this task.     
 *  @param[in]  Inst Inst, the instance ID of this task.
 *  @param[in]  Region Region, the region ID registered for memory 
 *              usage of this task.
 *  @param[in]  Reason Reason.
 *  @return  int
 *      -# ROK
 **/
uint8_t schActvInit(Ent entity, Inst instId, Region region, Reason reason)
{
   Inst inst = (instId  - SCH_INST_START);

   /* Initialize the MAC TskInit structure to zero */
   memset ((uint8_t *)&schCb[inst], 0, sizeof(schCb));

   /* Initialize the MAC TskInit with received values */
   schCb[inst].schInit.ent = entity;
   schCb[inst].schInit.inst = inst;
   schCb[inst].schInit.region = region;
   schCb[inst].schInit.pool = 0;
   schCb[inst].schInit.reason = reason;
   schCb[inst].schInit.cfgDone = FALSE;
   schCb[inst].schInit.acnt = FALSE;
   schCb[inst].schInit.usta = FALSE;
   schCb[inst].schInit.trc = FALSE;
   schCb[inst].schInit.procId = ODU_GET_PROCID();

   return ROK;
} /* schActvInit */

/**
 * @brief Scheduler instance Configuration Handler. 
 *
 * @details
 *
 *     Function : SchInstCfg
 *     
 *     This function in called by SchProcGenCfgReq(). It handles the
 *     general configurations of the scheduler instance. Returns
 *     reason for success/failure of this function.
 *     
 *  @param[in]  RgCfg *cfg, the Configuaration information 
 *  @return  uint16_t
 *      -# LCM_REASON_NOT_APPL 
 *      -# LCM_REASON_INVALID_MSGTYPE
 *      -# LCM_REASON_MEM_NOAVAIL
 **/
uint8_t SchInstCfg(RgCfg *cfg, Inst  dInst)
{
   uint16_t ret = LCM_REASON_NOT_APPL;
   Inst     inst = (dInst - SCH_INST_START);

   DU_LOG("\nDEBUG  -->  SCH : Entered SchInstCfg()");
   /* Check if Instance Configuration is done already */
   if (schCb[inst].schInit.cfgDone == TRUE)
   {
      return LCM_REASON_INVALID_MSGTYPE;
   }
   /* Update the Pst structure for LM interface */
   memcpy(&schCb[inst].schInit.lmPst,
	 &cfg->s.schInstCfg.genCfg.lmPst,
	 sizeof(Pst));

   schCb[inst].schInit.inst = inst;
   schCb[inst].schInit.lmPst.srcProcId = schCb[inst].schInit.procId;
   schCb[inst].schInit.lmPst.srcEnt = schCb[inst].schInit.ent;
   schCb[inst].schInit.lmPst.srcInst = schCb[inst].schInit.inst +
      SCH_INST_START;
   schCb[inst].schInit.lmPst.event = EVTNONE;

   schCb[inst].schInit.region = cfg->s.schInstCfg.genCfg.mem.region;
   schCb[inst].schInit.pool = cfg->s.schInstCfg.genCfg.mem.pool;
   schCb[inst].genCfg.tmrRes = cfg->s.schInstCfg.genCfg.tmrRes;
#ifdef LTE_ADV
   schCb[inst].genCfg.forceCntrlSrbBoOnPCel =  cfg->s.schInstCfg.genCfg.forceCntrlSrbBoOnPCel;
   schCb[inst].genCfg.isSCellActDeactAlgoEnable =  cfg->s.schInstCfg.genCfg.isSCellActDeactAlgoEnable;
#endif
   schCb[inst].genCfg.startCellId    = cfg->s.schInstCfg.genCfg.startCellId;

   /* Initialzie the timer queue */   
   memset(&schCb[inst].tmrTq, 0, sizeof(CmTqType) * SCH_TQ_SIZE);
   /* Initialize the timer control point */
   memset(&schCb[inst].tmrTqCp, 0, sizeof(CmTqCp));
   schCb[inst].tmrTqCp.tmrLen = RGSCH_TQ_SIZE;

   /* SS_MT_TMR needs to be enabled as schActvTmr needs instance information */
   /* Timer Registration request to system services */
   if (ODU_REG_TMR_MT(schCb[inst].schInit.ent, dInst,
	    (int)schCb[inst].genCfg.tmrRes, schActvTmr) != ROK)
   {
      DU_LOG("\nERROR  -->  SCH : SchInstCfg(): Failed to "
	    "register timer.");
      return (LCM_REASON_MEM_NOAVAIL);
   }   
              
   /* Set Config done in TskInit */
   schCb[inst].schInit.cfgDone = TRUE;
   DU_LOG("\nINFO   -->  SCH : Scheduler gen config done");

   return ret;
}

/**
 * @brief Layer Manager Configuration request handler. 
 *
 * @details
 *
 *     Function : SchProcGenCfgReq
 *     
 *     This function handles the configuration
 *     request received at scheduler instance from the Layer Manager.
 *     -# Based on the cfg->hdr.elmId.elmnt value it invokes one of the
 *        functions rgHdlGenCfg() or rgHdlSapCfg().
 *     -# Invokes RgMiLrgSchCfgCfm() to send back the confirmation to the LM.
 *     
 *  @param[in]  Pst *pst, the post structure     
 *  @param[in]  RgMngmt *cfg, the configuration parameter's structure
 *  @return  S16
 *      -# ROK
 **/
uint8_t SchProcGenCfgReq(Pst *pst, RgMngmt *cfg)
{
   uint8_t   ret = LCM_PRIM_OK;
   uint16_t  reason = LCM_REASON_NOT_APPL;
   RgMngmt   cfm;
   Pst       cfmPst;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : GENERAL_CFG_REQ\n");
#endif

   if(pst->dstInst < SCH_INST_START)
   {
      DU_LOG("\nERROR  -->  SCH : Invalid inst ID");
      DU_LOG("\nERROR  -->  SCH : SchProcGenCfgReq(): "
	    "pst->dstInst=%d SCH_INST_START=%d", pst->dstInst,SCH_INST_START); 
      return ROK;
   }
   DU_LOG("\nINFO   -->  SCH : Received scheduler gen config");
   /* Fill the post structure for sending the confirmation */
   memset(&cfmPst, 0 , sizeof(Pst));
   SchFillCfmPst(pst, &cfmPst, cfg);

   memset(&cfm, 0, sizeof(RgMngmt));

#ifdef LMINT3
   cfm.hdr.transId =
      cfg->hdr.transId;
#endif

   cfm.hdr.elmId.elmnt = cfg->hdr.elmId.elmnt;
   switch(cfg->hdr.elmId.elmnt)
   {
      case STSCHINST:
	 reason = SchInstCfg(&cfg->t.cfg,pst->dstInst );
	 break;
      default:
	 ret = LCM_PRIM_NOK;
	 reason = LCM_REASON_INVALID_ELMNT;
	 DU_LOG("\nERROR  -->  SCH : Invalid Elmnt=%d", cfg->hdr.elmId.elmnt);
	 break;
   }

   if (reason != LCM_REASON_NOT_APPL)
   {
      ret = LCM_PRIM_NOK;
   }

   cfm.cfm.status = ret;
   cfm.cfm.reason = reason;

   SchSendCfgCfm(&cfmPst, &cfm);
   /*   SCH_FREE(pst->region, pst->pool, (Data *)cfg, sizeof(RgMngmt)); */

   return ROK;
}/*-- SchProcGenCfgReq --*/

/**
 * @brief slot indication from MAC to SCH.
 *
 * @details
 *
 *     Function : MacSchSlotInd 
 *      
 *      This API is invoked by PHY to indicate slot indication to Scheduler for
 *      a cell.
 *           
 *  @param[in]  Pst            *pst
 *  @param[in]  SlotTimingInfo    *slotInd
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
uint8_t MacSchSlotInd(Pst *pst, SlotTimingInfo *slotInd)
{
   Inst  inst = pst->dstInst-SCH_INST_START;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_SLOT_IND_TO_SCH\n");
#endif

   schProcessSlotInd(slotInd, inst);

   return ROK;
}  /* MacSchSlotInd */

/*******************************************************************
 *
 * @brief Processes Rach indication from MAC 
 *
 * @details
 *
 *    Function : MacSchRachInd
 *
 *    Functionality:
 *      Processes Rach indication from MAC
 *
 * @params[in] 
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t MacSchRachInd(Pst *pst, RachIndInfo *rachInd)
{
   Inst  inst = pst->dstInst-SCH_INST_START;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_RACH_IND_TO_SCH\n");
#endif

   DU_LOG("\nINFO  -->  SCH : Received Rach indication");
   schProcessRachInd(rachInd, inst);
   return ROK;
}

/*******************************************************************
 *
 * @brief Processes CRC indication from MAC 
 *
 * @details
 *
 *    Function : MacSchCrcInd
 *
 *    Functionality:
 *      Processes CRC indication from MAC
 *
 * @params[in] Post structure
 *             Crc Indication
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t MacSchCrcInd(Pst *pst, CrcIndInfo *crcInd)
{
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_CRC_IND_TO_SCH\n");
#endif

   switch(crcInd->crcInd[0])
   {
      case CRC_FAILED:
	 DU_LOG("\nDEBUG  -->  SCH : Received CRC indication. CRC Status [FAILURE]");
	 break;
      case CRC_PASSED:
	 DU_LOG("\nDEBUG  -->  SCH : Received CRC indication. CRC Status [PASS]");
	 break;
      default:
	 DU_LOG("\nDEBUG  -->  SCH : Invalid CRC state %d", crcInd->crcInd[0]);
	 return RFAILED;
   }
   return ROK;
}

#ifdef NR_TDD
/**
 *@brief Returns TDD periodicity in micro seconds
 *
 * @details
 * 
 * Function : schGetPeriodicityInMsec 
 * 
 * This API retunrs TDD periodicity in micro seconds
 * 
 * @param[in] DlUlTxPeriodicity 
 * @return  periodicityInMsec
 * **/

uint16_t schGetPeriodicityInMsec(DlUlTxPeriodicity tddPeriod)
{
   uint16_t  periodicityInMsec = 0;
   switch(tddPeriod)
   {
      case TX_PRDCTY_MS_0P5:
      {
         periodicityInMsec = 500;
         break;
      }
      case TX_PRDCTY_MS_0P625:
      {
         periodicityInMsec = 625;
         break;
      }
      case TX_PRDCTY_MS_1:
      {
         periodicityInMsec = 1000;
         break;
      }
      case TX_PRDCTY_MS_1P25:
      {
         periodicityInMsec = 1250;
         break;
      }
      case TX_PRDCTY_MS_2:
      {
         periodicityInMsec = 2000;
         break;
      }
      case TX_PRDCTY_MS_2P5:
      {
         periodicityInMsec = 2500;
         break;
      }
      case TX_PRDCTY_MS_5:
      {
         periodicityInMsec = 5000;
         break;
      }
      case TX_PRDCTY_MS_10:
      {
         periodicityInMsec = 10000;
         break;
      }
      default:
      {
	 DU_LOG("\nERROR  -->  SCH : Invalid DlUlTxPeriodicity:%d", tddPeriod);
      }
   }

   return periodicityInMsec;
}


/**
 * @brief init TDD slot config 
 *
 * @details
 *
 *     Function : schInitTddSlotCfg 
 *      
 *      This API is invoked after receiving schCellCfg
 *           
 *  @param[in]  schCellCb *cell
 *  @param[in]  SchCellCfg *schCellCfg
 *  @return  void
 **/
void schInitTddSlotCfg(SchCellCb *cell, SchCellCfg *schCellCfg)
{
   uint16_t periodicityInMicroSec = 0;
   int8_t slotIdx, symbIdx;

   periodicityInMicroSec = schGetPeriodicityInMsec(schCellCfg->tddCfg.tddPeriod);
   cell->numSlotsInPeriodicity = (periodicityInMicroSec * pow(2, schCellCfg->numerology))/1000;
   cell->slotFrmtBitMap = 0;
   cell->symbFrmtBitMap = 0;
   for(slotIdx = cell->numSlotsInPeriodicity-1; slotIdx >= 0; slotIdx--)
   {
      symbIdx = 0;
      /* If the first and last symbol are the same, the entire slot is the same type */
      if((schCellCfg->tddCfg.slotCfg[slotIdx][symbIdx] == schCellCfg->tddCfg.slotCfg[slotIdx][MAX_SYMB_PER_SLOT-1]) &&
              schCellCfg->tddCfg.slotCfg[slotIdx][symbIdx] != FLEXI_SLOT)
      {
         switch(schCellCfg->tddCfg.slotCfg[slotIdx][symbIdx])
         {
            case DL_SLOT:
            {
               /*BitMap to be set to 00 */
               cell->slotFrmtBitMap = (cell->slotFrmtBitMap<<2);
               break;
            }
            case UL_SLOT:
            {
               /*BitMap to be set to 01 */
               cell->slotFrmtBitMap = ((cell->slotFrmtBitMap<<2) | (UL_SLOT));
               break;
            }
            default:
               DU_LOG("\nERROR  -->  SCH : Invalid slot Config in schInitTddSlotCfg");
           }
         continue;
      }
      /* slot config is flexible. First set slotBitMap to 10 */
      cell->slotFrmtBitMap = ((cell->slotFrmtBitMap<<2) | (FLEXI_SLOT));

      /* Now set symbol bitmap */ 
      for(symbIdx = MAX_SYMB_PER_SLOT-1; symbIdx >= 0; symbIdx--)
      {
         switch(schCellCfg->tddCfg.slotCfg[slotIdx][symbIdx])
         {
            case DL_SLOT:
            {
               /*symbol BitMap to be set to 00 */
               cell->symbFrmtBitMap = (cell->symbFrmtBitMap<<2);
               break;
            }
            case UL_SLOT:
            {
               /*symbol BitMap to be set to 01 */
               cell->symbFrmtBitMap = ((cell->symbFrmtBitMap<<2) | (UL_SLOT));
               break;
            }
            case FLEXI_SLOT:
            {
               /*symbol BitMap to be set to 10 */
               cell->symbFrmtBitMap = ((cell->symbFrmtBitMap<<2) | (FLEXI_SLOT));
               break;
            }
            default:
	       DU_LOG("\nERROR  -->  SCH : Invalid slot Config in schInitTddSlotCfg");
         }
      }
   }
}
#endif

/**
 * @brief Fill SSB start symbol
 *
 * @details
 *
 *     Function : fillSsbStartSymb 
 *      
 *      This API stores SSB start index per beam
 *           
 *  @param[in]  SchCellCb     *cellCb
 *  @return  int
 *      -# ROK 
 *      -# RFAILED 
 **/
void fillSsbStartSymb(SchCellCb *cellCb)
{
   uint8_t cnt, scs, symbIdx, ssbStartSymbArr[SCH_MAX_SSB_BEAM];

   scs = cellCb->cellCfg.ssbSchCfg.scsCommon;

   memset(ssbStartSymbArr, 0, sizeof(SCH_MAX_SSB_BEAM));
   symbIdx = 0;
   /* Determine value of "n" based on Section 4.1 of 3GPP TS 38.213 */
   switch(scs)
   {
      case SCS_15KHZ:
	 {
            if(cellCb->cellCfg.dlFreq <= 300000)
	       cnt = 2;/* n = 0, 1 */
            else
               cnt = 4; /* n = 0, 1, 2, 3 */
	    for(uint8_t idx=0; idx<cnt; idx++)
	    {
	       /* start symbol determined using {2, 8} + 14n */
	       ssbStartSymbArr[symbIdx++] = 2 +  MAX_SYMB_PER_SLOT*idx;
	       ssbStartSymbArr[symbIdx++] = 8 +  MAX_SYMB_PER_SLOT*idx;
	    }
	 }
	 break;
      case SCS_30KHZ:
	 {
            if(cellCb->cellCfg.dlFreq <= 300000)
	       cnt = 1;/* n = 0 */
            else
               cnt = 2; /* n = 0, 1 */
	    for(uint8_t idx=0; idx<cnt; idx++)
	    {
	       /* start symbol determined using {4, 8, 16, 20} + 28n */
	       ssbStartSymbArr[symbIdx++] = 4 +  MAX_SYMB_PER_SLOT*idx;
	       ssbStartSymbArr[symbIdx++] = 8 +  MAX_SYMB_PER_SLOT*idx;
	       ssbStartSymbArr[symbIdx++] = 16 +  MAX_SYMB_PER_SLOT*idx;
	       ssbStartSymbArr[symbIdx++] = 20 +  MAX_SYMB_PER_SLOT*idx;
            }
	 }
	 break;
      default:
	 DU_LOG("\nERROR  -->  SCH : SCS %d is currently not supported", scs);
   }
   memset(cellCb->ssbStartSymbArr, 0, sizeof(SCH_MAX_SSB_BEAM));
   memcpy(cellCb->ssbStartSymbArr, ssbStartSymbArr, SCH_MAX_SSB_BEAM);

}


/**
 * @brief init cellCb based on cellCfg
 *
 * @details
 *
 *     Function : schInitCellCb 
 *      
 *      This API is invoked after receiving schCellCfg
 *           
 *  @param[in]  schCellCb *cell
 *  @param[in]  SchCellCfg *schCellCfg
 *  @return  int
 *      -# ROK 
 *      -# RFAILED 
 **/
uint8_t schInitCellCb(Inst inst, SchCellCfg *schCellCfg)
{
   SchCellCb *cell= NULLP;
   SCH_ALLOC(cell, sizeof(SchCellCb));
   if(!cell)
   {
      DU_LOG("\nERROR  -->  SCH : Memory allocation failed in schInitCellCb");
      return RFAILED;
   }

   cell->cellId = schCellCfg->cellId; 
   cell->instIdx = inst;
   switch(schCellCfg->numerology)
   {
      case SCH_NUMEROLOGY_0:
	 {
	    cell->numSlots = SCH_MU0_NUM_SLOTS;
	 }
	 break;
      case SCH_NUMEROLOGY_1:
	 {
	    cell->numSlots = SCH_MU1_NUM_SLOTS;
	 }
	 break;
      case SCH_NUMEROLOGY_2:
	 {
	    cell->numSlots = SCH_MU2_NUM_SLOTS;
	 }
	 break;
      case SCH_NUMEROLOGY_3:
	 {
	    cell->numSlots = SCH_MU3_NUM_SLOTS;
	 }
	 break;
      case SCH_NUMEROLOGY_4:
	 {
	    cell->numSlots = SCH_MU4_NUM_SLOTS;
	 }
	 break;
      default:
	 DU_LOG("\nERROR  -->  SCH : Numerology %d not supported", schCellCfg->numerology);
   }
#ifdef NR_TDD
   schInitTddSlotCfg(cell, schCellCfg);   
#endif

   SCH_ALLOC(cell->schDlSlotInfo, cell->numSlots * sizeof(SchDlSlotInfo*));
   if(!cell->schDlSlotInfo)
   {
      DU_LOG("\nERROR  -->  SCH : Memory allocation failed in schInitCellCb for schDlSlotInfo");
      return RFAILED;
   }

   SCH_ALLOC(cell->schUlSlotInfo, cell->numSlots * sizeof(SchUlSlotInfo*));
   if(!cell->schUlSlotInfo)
   {
      DU_LOG("\nERROR  -->  SCH : Memory allocation failed in schInitCellCb for schUlSlotInfo");
      return RFAILED;
   }

   for(uint8_t idx=0; idx<cell->numSlots; idx++)
   {
      SchDlSlotInfo *schDlSlotInfo;
      SchUlSlotInfo *schUlSlotInfo;

      /* DL Alloc */
      SCH_ALLOC(schDlSlotInfo, sizeof(SchDlSlotInfo));
      if(!schDlSlotInfo)
      {
         DU_LOG("\nERROR  -->  SCH : Memory allocation failed in schInitCellCb");
         return RFAILED;
      }

      /* UL Alloc */
      SCH_ALLOC(schUlSlotInfo, sizeof(SchUlSlotInfo));
      if(!schUlSlotInfo)
      {
	 DU_LOG("\nERROR  -->  SCH : Memory allocation failed in schInitCellCb");
	 return RFAILED;
      }

      schInitDlSlot(schDlSlotInfo);
      schInitUlSlot(schUlSlotInfo);

      cell->schDlSlotInfo[idx] = schDlSlotInfo;
      cell->schUlSlotInfo[idx] = schUlSlotInfo;

   }
   cell->firstSsbTransmitted = false;
   cell->firstSib1Transmitted = false;
   fillSsbStartSymb(cell);
   cmLListInit(&cell->ueToBeScheduled);
   schCb[inst].cells[inst] = cell;

   DU_LOG("\nINFO  -->  SCH : Cell init completed for cellId:%d", cell->cellId);

   return ROK;   
}

/**
 * @brief Fill SIB1 configuration
 *
 * @details
 *
 *     Function : fillSchSib1Cfg
 *
 *     Fill SIB1 configuration
 *
 *  @param[in]  uint8_t bandwidth : total available bandwidth
 *              uint8_t numSlots : total slots per SFN
 *              SchSib1Cfg *sib1SchCfg : cfg to be filled
 *              uint16_t pci : physical cell Id
 *              uint8_t offsetPointA : offset
 *  @return  void
 **/
void fillSchSib1Cfg(uint8_t mu, uint8_t bandwidth, uint8_t numSlots, SchSib1Cfg *sib1SchCfg, uint16_t pci, uint8_t offsetPointA)
{
   uint8_t coreset0Idx = 0;
   uint8_t searchSpace0Idx = 0;
   //uint8_t ssbMuxPattern = 0;
   uint8_t numRbs = 0;
   uint8_t numSymbols = 0;
   uint8_t offset = 0;
   uint8_t oValue = 0;
   //uint8_t numSearchSpacePerSlot = 0;
   uint8_t mValue = 0;
   uint8_t firstSymbol = 0; /* need to calculate using formula mentioned in 38.213 */
   uint8_t slotIndex = 0;
   uint8_t FreqDomainResource[FREQ_DOM_RSRC_SIZE] = {0};
   uint16_t tbSize = 0;
   uint8_t ssbIdx = 0;

   PdcchCfg *pdcch = &(sib1SchCfg->sib1PdcchCfg);
   PdschCfg *pdsch = &(sib1SchCfg->sib1PdschCfg);
   BwpCfg *bwp = &(sib1SchCfg->bwp);

   coreset0Idx     = sib1SchCfg->coresetZeroIndex;
   searchSpace0Idx = sib1SchCfg->searchSpaceZeroIndex;

   /* derive the sib1 coreset0 params from table 13-1 spec 38.213 */
   //ssbMuxPattern = coresetIdxTable[coreset0Idx][0];
   numRbs        = coresetIdxTable[coreset0Idx][1];
   numSymbols    = coresetIdxTable[coreset0Idx][2];
   offset        = coresetIdxTable[coreset0Idx][3];

   /* derive the search space params from table 13-11 spec 38.213 */
   oValue                = searchSpaceIdxTable[searchSpace0Idx][0];
   //numSearchSpacePerSlot = searchSpaceIdxTable[searchSpace0Idx][1];
   mValue                = searchSpaceIdxTable[searchSpace0Idx][2];
   firstSymbol           = searchSpaceIdxTable[searchSpace0Idx][3];

   /* calculate the n0, need to add the formulae, as of now the value is 0 
    * Need to add the even and odd values of i during configuration 
    * [(O . 2^u + i . M )  ] mod numSlotsPerSubframe 
    * assuming u = 0, i = 0, numSlotsPerSubframe = 10
    * Also, from this configuration, coreset0 is only on even subframe */
   slotIndex = (int)((oValue*pow(2, mu)) + floor(ssbIdx*mValue))%numSlots;
   sib1SchCfg->n0 = slotIndex;

   /* fill BWP */
   switch(bandwidth)
   {
      case BANDWIDTH_20MHZ:
	 {
            bwp->freqAlloc.numPrb = TOTAL_PRB_20MHZ_MU0;
	 }
	 break;
      case BANDWIDTH_100MHZ:
	 {
            bwp->freqAlloc.numPrb = TOTAL_PRB_100MHZ_MU1;
	 }
	 break;
      default:
	 DU_LOG("\nERROR  -->  SCH : Bandwidth %d not supported", bandwidth);

   }
   bwp->freqAlloc.startPrb = 0;
   bwp->subcarrierSpacing  = 0;         /* 15Khz */
   bwp->cyclicPrefix       = 0;              /* normal */

   /* fill the PDCCH PDU */
   pdcch->coresetCfg.coreSetSize = numRbs;
   pdcch->coresetCfg.startSymbolIndex = firstSymbol;
   pdcch->coresetCfg.durationSymbols = numSymbols;
   
   /* Fill Bitmap for PRBs in coreset */
   fillCoresetFeqDomAllocMap(((offsetPointA-offset)/6), (numRbs/6), FreqDomainResource);
   covertFreqDomRsrcMapToIAPIFormat(FreqDomainResource, pdcch->coresetCfg.freqDomainResource);

   pdcch->coresetCfg.cceRegMappingType = 1; /* coreset0 is always interleaved */
   pdcch->coresetCfg.regBundleSize = 6;    /* spec-38.211 sec 7.3.2.2 */
   pdcch->coresetCfg.interleaverSize = 2;  /* spec-38.211 sec 7.3.2.2 */
   pdcch->coresetCfg.coreSetType = 0;
   pdcch->coresetCfg.shiftIndex = pci;
   pdcch->coresetCfg.precoderGranularity = 0; /* sameAsRegBundle */
   pdcch->numDlDci = 1;
   pdcch->dci.rnti = SI_RNTI;
   pdcch->dci.scramblingId = pci;
   pdcch->dci.scramblingRnti = 0;
   pdcch->dci.cceIndex = 0;
   pdcch->dci.aggregLevel = 4;
   pdcch->dci.beamPdcchInfo.numPrgs = 1;
   pdcch->dci.beamPdcchInfo.prgSize = 1;
   pdcch->dci.beamPdcchInfo.digBfInterfaces = 0;
   pdcch->dci.beamPdcchInfo.prg[0].pmIdx = 0;
   pdcch->dci.beamPdcchInfo.prg[0].beamIdx[0] = 0;
   pdcch->dci.txPdcchPower.powerValue = 0;
   pdcch->dci.txPdcchPower.powerControlOffsetSS = 0;
   /* Storing pdschCfg pointer here. Required to access pdsch config while
      fillig up pdcch pdu */
   pdcch->dci.pdschCfg = pdsch; 

   /* fill the PDSCH PDU */
   uint8_t cwCount = 0;
   pdsch->pduBitmap = 0; /* PTRS and CBG params are excluded */
   pdsch->rnti = 0xFFFF; /* SI-RNTI */
   pdsch->pduIndex = 0;
   pdsch->numCodewords = 1;
   for(cwCount = 0; cwCount < pdsch->numCodewords; cwCount++)
   {
      pdsch->codeword[cwCount].targetCodeRate = 308;
      pdsch->codeword[cwCount].qamModOrder = 2;
      pdsch->codeword[cwCount].mcsIndex = sib1SchCfg->sib1Mcs;
      pdsch->codeword[cwCount].mcsTable = 0; /* notqam256 */
      pdsch->codeword[cwCount].rvIndex = 0;
      tbSize = schCalcTbSize(sib1SchCfg->sib1PduLen + TX_PAYLOAD_HDR_LEN);
      pdsch->codeword[cwCount].tbSize = tbSize;
   }
   pdsch->dataScramblingId                   = pci;
   pdsch->numLayers                          = 1;
   pdsch->transmissionScheme                 = 0;
   pdsch->refPoint                           = 0;
   pdsch->dmrs.dlDmrsSymbPos                 = 4; /* Bitmap value 00000000000100 i.e. using 3rd symbol for PDSCH DMRS */
   pdsch->dmrs.dmrsConfigType                = 0; /* type-1 */
   pdsch->dmrs.dlDmrsScramblingId            = pci;
   pdsch->dmrs.scid                          = 0;
   pdsch->dmrs.numDmrsCdmGrpsNoData          = 1;
   pdsch->dmrs.dmrsPorts                     = 0x0001;
   pdsch->dmrs.mappingType                   = DMRS_MAP_TYPE_A; /* Type-A */
   pdsch->dmrs.nrOfDmrsSymbols               = NUM_DMRS_SYMBOLS;
   pdsch->dmrs.dmrsAddPos                    = DMRS_ADDITIONAL_POS;

   pdsch->pdschFreqAlloc.resourceAllocType   = 1; /* RAT type-1 RIV format */
   /* the RB numbering starts from coreset0, and PDSCH is always above SSB */
   pdsch->pdschFreqAlloc.freqAlloc.startPrb  = offsetPointA + SCH_SSB_NUM_PRB;
   pdsch->pdschFreqAlloc.freqAlloc.numPrb    = schCalcNumPrb(tbSize,sib1SchCfg->sib1Mcs, NUM_PDSCH_SYMBOL);
   pdsch->pdschFreqAlloc.vrbPrbMapping       = 0; /* non-interleaved */
   pdsch->pdschTimeAlloc.rowIndex            = 1;
   /* This is Intel's requirement. PDSCH should start after PDSCH DRMS symbol */
   pdsch->pdschTimeAlloc.timeAlloc.startSymb = 3; /* spec-38.214, Table 5.1.2.1-1 */
   pdsch->pdschTimeAlloc.timeAlloc.numSymb   = NUM_PDSCH_SYMBOL;
   pdsch->beamPdschInfo.numPrgs              = 1;
   pdsch->beamPdschInfo.prgSize              = 1;
   pdsch->beamPdschInfo.digBfInterfaces      = 0;
   pdsch->beamPdschInfo.prg[0].pmIdx         = 0;
   pdsch->beamPdschInfo.prg[0].beamIdx[0]    = 0;
   pdsch->txPdschPower.powerControlOffset    = 0;
   pdsch->txPdschPower.powerControlOffsetSS  = 0;

}

/**
 * @brief cell config from MAC to SCH.
 *
 * @details
 *
 *     Function : macSchCellCfgReq
 *      
 *      This API is invoked by MAC to send cell config to SCH
 *           
 *  @param[in]  Pst            *pst
 *  @param[in]  SchCellCfg     *schCellCfg
 *  @return  int
 *      -# ROK 
 *      -# RFAILED 
 **/
uint8_t SchHdlCellCfgReq(Pst *pst, SchCellCfg *schCellCfg)
{
   uint8_t ret = ROK;
   SchCellCb *cellCb;
   SchCellCfgCfm schCellCfgCfm;
   Pst rspPst;
   Inst inst = pst->dstInst-1; 
   uint8_t coreset0Idx = 0;
   uint8_t numRbs = 0;
   uint8_t offset = 0;
   uint8_t freqDomainResource[FREQ_DOM_RSRC_SIZE] = {0};
   SchPdschConfig pdschCfg;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_SCH_CELL_CFG\n");
#endif 

   schInitCellCb(inst, schCellCfg);
   cellCb = schCb[inst].cells[inst]; //cells is of MAX_CELLS, why inst
   cellCb->macInst = pst->srcInst;

   /* derive the SIB1 config parameters */
   fillSchSib1Cfg(schCellCfg->numerology, schCellCfg->bandwidth, cellCb->numSlots,
	 &(schCellCfg->sib1SchCfg), schCellCfg->phyCellId,
	 schCellCfg->ssbSchCfg.ssbOffsetPointA);
   memcpy(&cellCb->cellCfg, schCellCfg, sizeof(SchCellCfg));

   /* Fill coreset frequencyDomainResource bitmap */
   coreset0Idx = cellCb->cellCfg.schInitialDlBwp.pdcchCommon.commonSearchSpace.coresetId;
   numRbs = coresetIdxTable[coreset0Idx][1];
   offset = coresetIdxTable[coreset0Idx][3];
   fillCoresetFeqDomAllocMap(((cellCb->cellCfg.ssbSchCfg.ssbOffsetPointA - offset)/6), (numRbs/6), freqDomainResource);
   covertFreqDomRsrcMapToIAPIFormat(freqDomainResource, \
      cellCb->cellCfg.schInitialDlBwp.pdcchCommon.commonSearchSpace.freqDomainRsrc);

   /* Fill K0 - K1 table for common cfg*/ 
   BuildK0K1Table(cellCb, &cellCb->cellCfg.schInitialDlBwp.k0K1InfoTbl, true, cellCb->cellCfg.schInitialDlBwp.pdschCommon,
   pdschCfg, DEFAULT_UL_ACK_LIST_COUNT, defaultUlAckTbl);
   
   BuildK2InfoTable(cellCb, cellCb->cellCfg.schInitialUlBwp.puschCommon.timeDomRsrcAllocList,\
   cellCb->cellCfg.schInitialUlBwp.puschCommon.numTimeDomRsrcAlloc, &cellCb->cellCfg.schInitialUlBwp.msg3K2InfoTbl, \
   &cellCb->cellCfg.schInitialUlBwp.k2InfoTbl);
   /* Initializing global variables */
   cellCb->actvUeBitMap = 0;
   cellCb->boIndBitMap = 0;

   /* Fill and send Cell config confirm */
   memset(&rspPst, 0, sizeof(Pst));
   FILL_PST_SCH_TO_MAC(rspPst, pst->dstInst);
   rspPst.event = EVENT_SCH_CELL_CFG_CFM;

   schCellCfgCfm.cellId = schCellCfg->cellId; 
   schCellCfgCfm.rsp = RSP_OK;

   ret = (*SchCellCfgCfmOpts[rspPst.selector])(&rspPst, &schCellCfgCfm);
   return ret;

}

/*******************************************************************
 *
 * @brief Processes DL RLC BO info from MAC
 *
 * @details
 *
 *    Function : MacSchDlRlcBoInfo
 *
 *    Functionality:
 *       Processes DL RLC BO info from MAC
 *
 * @params[in] 
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t MacSchDlRlcBoInfo(Pst *pst, DlRlcBoInfo *dlBoInfo)
{
   uint8_t  lcId = 0;
   uint16_t ueId = 0;
   bool isLcIdValid = false;
   SchUeCb *ueCb = NULLP;
   SchCellCb *cell = NULLP;
   Inst  inst = pst->dstInst-SCH_INST_START;
   CmLListCp *lcLL = NULLP;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_DL_RLC_BO_INFO_TO_SCH\n");
#endif

   DU_LOG("\nDEBUG  -->  SCH : Received RLC BO Status indication LCId [%d] BO [%d]", dlBoInfo->lcId, dlBoInfo->dataVolume);
   cell = schCb[inst].cells[inst];

   if(cell == NULLP)
   {
      DU_LOG("\nERROR  -->  SCH : MacSchDlRlcBoInfo(): Cell does not exists");
      return RFAILED;
   }

   GET_UE_ID(dlBoInfo->crnti, ueId);
   ueCb = &cell->ueCb[ueId-1];
   lcId  = dlBoInfo->lcId;
   CHECK_LCID(lcId, isLcIdValid);
   if(isLcIdValid == FALSE)
   {
      DU_LOG("ERROR --> SCH: LCID:%d is not valid", lcId);
      return RFAILED;
   }

   /*Expected when theres a case of Retransmission Failure or Resetablishment
    *By Zero BO, the RLC is informing that previous data can be cleared out
    *Thus clearing out the LC from the Lc priority list*/
   if(dlBoInfo->dataVolume == 0)
   {
      /*Check the LC is Dedicated or default and accordingly LCList will
       * be used*/
      if(ueCb->dlInfo.dlLcCtxt[lcId].isDedicated)
      {
         lcLL = &(ueCb->dlLcPrbEst.dedLcInfo->dedLcList);
      }
      else
      {
         lcLL = &(ueCb->dlLcPrbEst.defLcList);
      }
      handleLcLList(lcLL, lcId, DELETE);
      return ROK;
   }

   if(lcId == SRB0_LCID)
   {
      cell->raCb[ueId -1].msg4recvd = true;
      cell->raCb[ueId -1].dlMsgPduLen = dlBoInfo->dataVolume;
      
   }
   else
   {
      /* TODO : These part of changes will be corrected during DL scheduling as
       * per K0 - K1 -K2 */
      SET_ONE_BIT(ueId, cell->boIndBitMap);
      if(ueCb->dlInfo.dlLcCtxt[lcId].lcId == lcId)
      {
         ueCb->dlInfo.dlLcCtxt[lcId].bo = dlBoInfo->dataVolume;
      }
      else
      {
         DU_LOG("ERROR --> SCH: LCID:%d is not configured in SCH Cb",lcId);
         return RFAILED;
      }
   }
   
   /* Adding UE Id to list of pending UEs to be scheduled */
   addUeToBeScheduled(cell, ueId);
   return ROK;
}

/*******************************************************************
 *
 * @brief Processes BSR indiation from MAC
 *
 * @details
 *
 *    Function : MacSchBsr
 *
 *    Functionality:
 *       Processes DL BSR from MAC
 *
 * @params[in]    Pst pst
 *                UlBufferStatusRptInd bsrInd
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t MacSchBsr(Pst *pst, UlBufferStatusRptInd *bsrInd)
{
   Inst           schInst       = pst->dstInst-SCH_INST_START;
   SchCellCb      *cellCb       = NULLP;
   SchUeCb        *ueCb         = NULLP;
   uint8_t        lcgIdx = 0;

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_SHORT_BSR\n");
#endif

   DU_LOG("\nDEBUG  -->  SCH : Received BSR");
   if(bsrInd == NULLP)
   {
      DU_LOG("\nERROR  -->  SCH : BSR Ind is empty");
      return RFAILED;
   }
   cellCb = schCb[schInst].cells[schInst];
   if(cellCb == NULLP)
   {
      DU_LOG("\nERROR  -->  SCH : CellCb is empty");
      return RFAILED;
   }
   ueCb = schGetUeCb(cellCb, bsrInd->crnti);

   if(ueCb == NULLP)
   {
      DU_LOG("\nERROR  -->  SCH : UeCB is empty");
      return RFAILED;
   }

   ueCb->bsrRcvd = true;
   /* store dataVolume per lcg in uecb */
   for(lcgIdx = 0; lcgIdx < bsrInd->numLcg; lcgIdx++)
   {
      ueCb->bsrInfo[bsrInd->dataVolInfo[lcgIdx].lcgId].priority = 1; //TODO: determining LCG priority?
      ueCb->bsrInfo[bsrInd->dataVolInfo[lcgIdx].lcgId].dataVol = bsrInd->dataVolInfo[lcgIdx].dataVol;
   }
   
   /* Adding UE Id to list of pending UEs to be scheduled */
   addUeToBeScheduled(cellCb, ueCb->ueId);
   return ROK;
}

/*******************************************************************
 *
 * @brief Processes SR UCI indication from MAC 
 *
 * @details
 *
 *    Function : MacSchSrUciInd
 *
 *    Functionality:
 *      Processes SR UCI indication from MAC
 *
 * @params[in] Post structure
 *             UCI Indication
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t MacSchSrUciInd(Pst *pst, SrUciIndInfo *uciInd)
{
   Inst  inst = pst->dstInst-SCH_INST_START;

   SchUeCb   *ueCb; 
   SchCellCb *cellCb = schCb[inst].cells[inst];

#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTSCH : EVENT_UCI_IND_TO_SCH\n");
#endif

   DU_LOG("\nDEBUG  -->  SCH : Received SR");

   ueCb = schGetUeCb(cellCb, uciInd->crnti);
   
   if(ueCb->state == SCH_UE_STATE_INACTIVE)
   {
      DU_LOG("\nERROR  -->  SCH : Crnti %d is inactive", uciInd->crnti);
      return ROK;  
   }

   if(uciInd->numSrBits)
   {
      ueCb->srRcvd = true;
      
      /* Adding UE Id to list of pending UEs to be scheduled */
      addUeToBeScheduled(cellCb, ueCb->ueId);
   }
   return ROK;
}

/*******************************************************************
 *
 * @brief Allocates requested PRBs for DL
 *
 * @details
 *
 *    Function : allocatePrbDl
 *
 *    Functionality:
 *      Allocates requested PRBs in DL
 *      Keeps track of allocated PRB (using bitmap) and remaining PRBs
 *
 * @params[in] prbAlloc table
 *             Start symbol
 *             Number of symbols
 *             Start PRB
 *             Number of PRBs
 *
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t allocatePrbDl(SchCellCb *cell, SlotTimingInfo slotTime, \
   uint8_t startSymbol, uint8_t symbolLength, uint16_t *startPrb, uint16_t numPrb)
{
   uint8_t        symbol = 0;
   uint16_t       broadcastPrbStart=0, broadcastPrbEnd=0;
   FreePrbBlock   *freePrbBlock = NULLP;
   CmLList        *freePrbNode = NULLP;
   PduTxOccsaion  ssbOccasion=0, sib1Occasion=0;
   SchDlSlotInfo  *schDlSlotInfo = cell->schDlSlotInfo[slotTime.slot];
   SchPrbAlloc    *prbAlloc = &schDlSlotInfo->prbAlloc;

   /* If startPrb is set to MAX_NUM_RB, it means startPrb is not known currently.
    * Search for an appropriate location in PRB grid and allocate requested resources */
   if(*startPrb == MAX_NUM_RB)
   {
      /* Check if SSB/SIB1 is also scheduled in this slot  */
      ssbOccasion = schCheckSsbOcc(cell, slotTime);
      sib1Occasion = schCheckSib1Occ(cell, slotTime);

      if(ssbOccasion && sib1Occasion)
      {
         broadcastPrbStart = cell->cellCfg.ssbSchCfg.ssbOffsetPointA; 
         broadcastPrbEnd = broadcastPrbStart + SCH_SSB_NUM_PRB + cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.numPrb -1;
      }
      else if(ssbOccasion)
      {
         broadcastPrbStart = cell->cellCfg.ssbSchCfg.ssbOffsetPointA;
         broadcastPrbEnd = broadcastPrbStart + SCH_SSB_NUM_PRB -1;
      }
      else if(sib1Occasion)
      {
         broadcastPrbStart = cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.startPrb;
         broadcastPrbEnd = broadcastPrbStart + cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.numPrb -1;
      }

      /* Iterate through all free PRB blocks */
      freePrbNode = prbAlloc->freePrbBlockList.first; 
      while(freePrbNode)
      {
         freePrbBlock = (FreePrbBlock *)freePrbNode->node; 

         /* If broadcast message is scheduled in this slot, then check if its PRBs belong to the current free block.
          * Since SSB/SIB1 PRB location is fixed, these PRBs cannot be allocated to other message in same slot */
         if((ssbOccasion || sib1Occasion) && 
            ((broadcastPrbStart >= freePrbBlock->startPrb) && (broadcastPrbStart <= freePrbBlock->endPrb)) && \
            ((broadcastPrbEnd >= freePrbBlock->startPrb) && (broadcastPrbEnd <= freePrbBlock->endPrb)))
         {
            /* Implmentation is done such that highest-numbered free-RB is allocated first */ 
            if((freePrbBlock->endPrb > broadcastPrbEnd) && ((freePrbBlock->endPrb - broadcastPrbEnd) >= numPrb))
            {
               /* If sufficient free PRBs are available above bradcast message then,
                * endPrb = freePrbBlock->endPrb
                * startPrb = endPrb - numPrb +1;
                */
               *startPrb = freePrbBlock->endPrb - numPrb +1;
               break;
            }
            else if((broadcastPrbStart > freePrbBlock->startPrb) && ((broadcastPrbStart - freePrbBlock->startPrb) >= numPrb))
            {
               /* If free PRBs are available below broadcast message then,
                * endPrb = broadcastPrbStart - 1
                * startPrb = endPrb - numPrb +1
                */
               *startPrb = broadcastPrbStart - numPrb; 
               break;
            }
            else
            {
               freePrbNode = freePrbNode->next;
               continue;
            }
         }
         else
         {
            /* Check if requested number of blocks can be allocated from the current block */ 
            if (freePrbBlock->numFreePrb < numPrb)
            {
               freePrbNode = freePrbNode->next;
               continue;
            }
            *startPrb = freePrbBlock->endPrb - numPrb +1;
            break;  
         }
      }

      /* If no free block can be used to allocated request number of RBs */
      if(*startPrb == MAX_NUM_RB)
         return RFAILED;
   }

   /* If startPrb is known already, check if requested PRBs are available for allocation */
   else
   {
      freePrbNode = isPrbAvailable(&prbAlloc->freePrbBlockList, *startPrb, numPrb);
      if(!freePrbNode)
      {
         DU_LOG("\nERROR  -->  SCH: Requested DL PRB unavailable");
         return RFAILED;
      }
   }

   /* Update bitmap to allocate PRBs */
   for(symbol=startSymbol; symbol < (startSymbol+symbolLength); symbol++)
   {
      if(fillPrbBitmap(prbAlloc->prbBitMap[symbol], *startPrb, numPrb) != ROK)
      {
         DU_LOG("\nERROR  -->  SCH: fillPrbBitmap() failed for symbol [%d] in DL", symbol);
         return RFAILED;
      }
   }

   /* Update the remaining number for free PRBs */
   removeAllocatedPrbFromFreePrbList(&prbAlloc->freePrbBlockList, freePrbNode, *startPrb, numPrb);

   return ROK;
}

/*******************************************************************
 *
 * @brief Allocates requested PRBs for UL
 *
 * @details
 *
 *    Function : allocatePrbUl
 *
 *    Functionality:
 *      Allocates requested PRBs in UL
 *      Keeps track of allocated PRB (using bitmap) and remaining PRBs
 *
 * @params[in] prbAlloc table
 *             Start symbol
 *             Number of symbols
 *             Start PRB
 *             Number of PRBs
 *
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t allocatePrbUl(SchCellCb *cell, SlotTimingInfo slotTime, \
   uint8_t startSymbol, uint8_t symbolLength, uint16_t *startPrb, uint16_t numPrb)
{
   uint8_t        symbol = 0;
   uint16_t       prachStartPrb, prachNumPrb, prachEndPrb;
   bool           isPrachOccasion;
   FreePrbBlock   *freePrbBlock = NULLP;
   CmLList        *freePrbNode = NULLP;
   SchPrbAlloc    *prbAlloc = NULLP;

   if(cell == NULLP)
   {
      DU_LOG("\nERROR  --> SCH : allocatePrbUl(): Received cellCb is null");
      return RFAILED;
   }
   
   prbAlloc =   &cell->schUlSlotInfo[slotTime.slot]->prbAlloc;
   /* If startPrb is set to MAX_NUM_RB, it means startPrb is not known currently.
    * Search for an appropriate location in PRB grid and allocate requested resources */
   if(*startPrb == MAX_NUM_RB)
   {
      /* Check if PRACH is also scheduled in this slot */
      isPrachOccasion = schCheckPrachOcc(cell, slotTime);
      if(isPrachOccasion)
      {
         prachStartPrb =  cell->cellCfg.schRachCfg.msg1FreqStart;
         prachNumPrb = schCalcPrachNumRb(cell);
         prachEndPrb = prachStartPrb + prachNumPrb -1;
      }

      /* Iterate through all free PRB blocks */
      freePrbNode = prbAlloc->freePrbBlockList.first; 
      while(freePrbNode)
      {
         freePrbBlock = (FreePrbBlock *)freePrbNode->node; 

         /* If PRACH is scheduled in this slot, then check if its PRBs belong to the current free block.
          * PRBs required for PRACH cannot be allocated to any other message */
         if((isPrachOccasion) &&
            ((prachStartPrb >= freePrbBlock->startPrb) && (prachStartPrb <= freePrbBlock->endPrb)) &&
            ((prachEndPrb >= freePrbBlock->startPrb) && (prachEndPrb <= freePrbBlock->endPrb)))
         {
            /* Implmentation is done such that highest-numbered free-RB is allocated first */ 
            if((freePrbBlock->endPrb > prachEndPrb) && ((freePrbBlock->endPrb - prachEndPrb) >= numPrb))
            {
               /* If sufficient free PRBs are available above PRACH message then,
                * endPrb = freePrbBlock->endPrb
                * startPrb = endPrb - numPrb +1;
                */
               *startPrb = freePrbBlock->endPrb - numPrb +1;
               break;
            }
            else if((prachStartPrb > freePrbBlock->startPrb) && ((prachStartPrb - freePrbBlock->startPrb) >= numPrb))
            {
               /* If free PRBs are available below PRACH message then,
                * endPrb = prachStartPrb - 1
                * startPrb = endPrb - numPrb +1
                */
               *startPrb = prachStartPrb - numPrb; 
               break;
            }
            else
            {
               freePrbNode = freePrbNode->next;
               continue;
            } 
         }
         else
         {
            /* Check if requested number of PRBs can be allocated from currect block */
            if(freePrbBlock->numFreePrb < numPrb)
            {
               freePrbNode = freePrbNode->next;
               continue;
            }
            *startPrb = freePrbBlock->endPrb - numPrb +1;
            break;
         }
      }

      /* If no free block can be used to allocated requested number of RBs */
      if(*startPrb == MAX_NUM_RB)
         return RFAILED;
   }
   else
   {
      /* If startPrb is known already, check if requested PRBs are available for allocation */
      freePrbNode = isPrbAvailable(&prbAlloc->freePrbBlockList, *startPrb, numPrb);
      if(!freePrbNode)
      {
         DU_LOG("\nERROR  -->  SCH: Requested UL PRB unavailable");
         return RFAILED;
      }
   }

   /* Update bitmap to allocate PRBs */
   for(symbol=startSymbol; symbol < (startSymbol+symbolLength); symbol++)
   {
      if(fillPrbBitmap(prbAlloc->prbBitMap[symbol], *startPrb, numPrb) != ROK)
      {
         DU_LOG("\nERROR  -->  SCH: fillPrbBitmap() failed for symbol [%d] in UL", symbol);
         return RFAILED;
      }
   }

   /* Update the remaining number for free PRBs */
   removeAllocatedPrbFromFreePrbList(&prbAlloc->freePrbBlockList, freePrbNode, *startPrb, numPrb);

   return ROK;
}

/*******************************************************************
 *
 * @brief Add UE to ueToBeScheduled List
 *
 * @details
 *
 *    Function : addUeToBeScheduled
 *
 *    Functionality:
 *      Search if UE entry present in the list
 *      If yes, return.
 *      If no, add UE to the list
 *
 * @params[in] Cell control block
 *             Ue Idx to be added
 *
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t addUeToBeScheduled(SchCellCb *cell, uint8_t ueIdToAdd)
{
   uint8_t *ueId;
   CmLList *node;

   /* Search if UE entry is already present in ueToBeScheduled list.
    * If yes, another entry for same UE not needed. Hence, return */
   node = cell->ueToBeScheduled.first;
   while(node)
   {
      ueId = (uint8_t *)node->node;
      if(*ueId == ueIdToAdd)
         return ROK;
      node = node->next;
   }

   /* If UE entry not present already, add UE to the end of ueToBeScheduled list */
   SCH_ALLOC(ueId, sizeof(uint8_t));
   if(!ueId)
   {
      DU_LOG("\nERROR  -->  SCH : Memory allocation failure in addUeToBeScheduled");
      return RFAILED;
   }
   *ueId = ueIdToAdd;
   if(addNodeToLList(&cell->ueToBeScheduled, ueId, NULLP) != ROK)
   {
      DU_LOG("\nERROR  --> SCH : Failed to add ueId [%d] to cell->ueToBeScheduled list", *ueId);
      return RFAILED;
   }
   return ROK;
}
 
/*******************************************************************************
 *
 * @brief Try to find Best Free Block with Max Num PRB 
 *
 * @details
 *
 *    Function : searchLargestFreeBlock
 *
 *    Functionality:
 *     Finds the FreeBlock with MaxNum of FREE PRB considering SSB/SIB1 ocassions.
 *
 * @params[in] I/P > prbAlloc table (FreeBlock list)
 *             I/P > Slot timing Info
 *             O/P > Start PRB
 *             I/P > Direction (UL/DL)
 *       
 *
 * @return Max Number of Free PRB 
 *         If 0, then no Suitable Free Block
 *
 * ********************************************************************************/

uint16_t searchLargestFreeBlock(SchCellCb *cell, SlotTimingInfo slotTime,uint16_t *startPrb, Direction dir)
{
   uint16_t       reservedPrbStart=0, reservedPrbEnd=0, maxFreePRB = 0;
   FreePrbBlock   *freePrbBlock = NULLP;
   CmLList        *freePrbNode = NULLP;
   SchPrbAlloc    *prbAlloc = NULLP;
   bool           checkOccasion = FALSE;

   *startPrb = 0; /*Initialize the StartPRB to zero*/

   /*Based on Direction, Reserved Messsages will differi.e.
    * DL >> SSB and SIB1 ocassions wheres for UL, PRACH ocassions to be checked
    * and reserved before allocation for dedicated DL/UL msg*/
   if(dir == DIR_DL)
   {
      SchDlSlotInfo  *schDlSlotInfo = cell->schDlSlotInfo[slotTime.slot];
      PduTxOccsaion  ssbOccasion=0, sib1Occasion=0;

      prbAlloc = &schDlSlotInfo->prbAlloc;

      ssbOccasion = schCheckSsbOcc(cell, slotTime);
      sib1Occasion = schCheckSib1Occ(cell, slotTime);

      checkOccasion = TRUE;
      if(ssbOccasion && sib1Occasion)
      {
         reservedPrbStart = cell->cellCfg.ssbSchCfg.ssbOffsetPointA; 
         reservedPrbEnd = reservedPrbStart + SCH_SSB_NUM_PRB + \
                          cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.numPrb -1;
      }
      else if(ssbOccasion)
      {
         reservedPrbStart = cell->cellCfg.ssbSchCfg.ssbOffsetPointA;
         reservedPrbEnd = reservedPrbStart + SCH_SSB_NUM_PRB -1;
      }
      else if(sib1Occasion)
      {
         reservedPrbStart = cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.startPrb;
         reservedPrbEnd = reservedPrbStart + cell->cellCfg.sib1SchCfg.sib1PdschCfg.pdschFreqAlloc.freqAlloc.numPrb -1;
      }
      else
      {
         checkOccasion = FALSE;  
      }
   }
   else if(dir == DIR_UL)
   {
      prbAlloc = &cell->schUlSlotInfo[slotTime.slot]->prbAlloc;

      /* Check if PRACH is also scheduled in this slot */
      checkOccasion = schCheckPrachOcc(cell, slotTime);
      if(checkOccasion)
      {
         reservedPrbStart =  cell->cellCfg.schRachCfg.msg1FreqStart;
         reservedPrbEnd = reservedPrbStart + (schCalcPrachNumRb(cell)) -1;
      }
   }
   else
   {
      DU_LOG("\nERROR --> SCH: Invalid Direction!");
      return (maxFreePRB);
   }

   freePrbNode = prbAlloc->freePrbBlockList.first; 
   while(freePrbNode)
   {
      freePrbBlock = (FreePrbBlock *)freePrbNode->node;

      /*For block with same numFreeBlocks, choose the one with HighestPRB range
       *Since FreeBLockList are arranged in Descending order of PRB range thus Skipping this block*/
      if(maxFreePRB >= freePrbBlock->numFreePrb) 
      {
         //skip this block
         freePrbNode = freePrbNode->next;
         continue;
      }

      /* If Broadcast/Prach message is scheduled in this slot, then check if its PRBs belong to the current free block.
       * Since SSB/SIB1 PRB location is fixed, these PRBs cannot be allocated to other message in same slot */
      if(checkOccasion && 
            ((reservedPrbStart >= freePrbBlock->startPrb) && (reservedPrbStart <= freePrbBlock->endPrb)) && \
            ((reservedPrbEnd >= freePrbBlock->startPrb) && (reservedPrbEnd <= freePrbBlock->endPrb)))
      {

         /* Implmentation is done such that highest-numbered free-RB is Checked first
            and freePRB in this block is greater than Max till now */
         if((freePrbBlock->endPrb > reservedPrbEnd) && ((freePrbBlock->endPrb - reservedPrbEnd) > maxFreePRB))
         {
            /* If sufficient free PRBs are available above reserved message*/
            *startPrb = reservedPrbEnd + 1;
            maxFreePRB = (freePrbBlock->endPrb - reservedPrbEnd);		 
         }
         /*Also check the other freeBlock (i.e. Above the reserved message) for MAX FREE PRB*/
         if((reservedPrbStart > freePrbBlock->startPrb) && ((reservedPrbStart - freePrbBlock->startPrb) > maxFreePRB))
         {
            /* If free PRBs are available below reserved message*/
            *startPrb = freePrbBlock->startPrb;
            maxFreePRB = (reservedPrbStart - freePrbBlock->startPrb);
         }
      }
      else  //Best Block
      {
         if(maxFreePRB < freePrbBlock->numFreePrb)
         {
            *startPrb = freePrbBlock->startPrb;
            maxFreePRB = freePrbBlock->numFreePrb;
         }

      }
      freePrbNode = freePrbNode->next;
   }  
   return(maxFreePRB);
}

/*******************************************************************************
 *
 * @brief This function is used to send Slice Cfg rsp to MAC
 *
 * @details
 *
 *    Function : SchSendSliceCfgRspToMac
 *
 *    Functionality:
 *     function is used to send Slice Cfg rsp to MAC
 *
 * @params[in] Pst *pst, SchSliceCfgRsp sliceCfgRsp
 *
 * @return- void
 *
 * ********************************************************************************/
void SchSendSliceCfgRspToMac(Inst inst, SchSliceCfgRsp sliceCfgRsp)
{
   Pst rspPst;
   
   memset(&rspPst, 0, sizeof(Pst));
   FILL_PST_SCH_TO_MAC(rspPst, inst);
   rspPst.event = EVENT_SLICE_CFG_RSP_TO_MAC;
   
   SchSliceCfgRspOpts[rspPst.selector](&rspPst, &sliceCfgRsp);

}
/*******************************************************************************
 *
 * @brief fill slice configuration response
 *
 * @details
 *
 *    Function : fillSliceCfgRsp
 *
 *    Functionality:
 *     fill slice configuration response
 *
 * @params[in] SchCellCb, SchSliceCfgReq, SchSliceCfgRsp,uint8_t  count
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 *
 * ********************************************************************************/
uint8_t fillSliceCfgRsp(bool sliceReCfg, SchSliceCfg *storedSliceCfg, SchCellCb *cellCb, SchSliceCfgReq *schSliceCfgReq, SchSliceCfgRsp *schSliceCfgRsp, uint8_t *count)
{
   bool sliceFound = false;
   uint8_t cfgIdx = 0, sliceIdx = 0;

   schSliceCfgRsp->numSliceCfgRsp  = schSliceCfgReq->numOfConfiguredSlice;
   SCH_ALLOC(schSliceCfgRsp->listOfSliceCfgRsp, schSliceCfgRsp->numSliceCfgRsp * sizeof(SliceRsp*));
   if(schSliceCfgRsp->listOfSliceCfgRsp == NULLP)
   {
      DU_LOG("\nERROR  --> SCH : Memory allocation failed at fillSliceCfgRsp");
      return RFAILED;
   }
   
   for(cfgIdx = 0; cfgIdx<schSliceCfgRsp->numSliceCfgRsp ; cfgIdx++)
   {
      sliceFound = false;
      /* Here comparing the slice cfg request with the slice stored in cellCfg */
      if(sliceReCfg != true)
      {
         for(sliceIdx = 0; sliceIdx<cellCb->cellCfg.plmnInfoList.numSliceSupport; sliceIdx++)
         {
            if(!memcmp(&schSliceCfgReq->listOfConfirguration[cfgIdx]->snssai, cellCb->cellCfg.plmnInfoList.snssai[sliceIdx], sizeof(Snssai)))
            {
               (*count)++;
               sliceFound = true;
               break;
            }
         }
      }
      else
      {
         /* Here comparing the slice cfg request with the slice stored in SchDb */
         if(storedSliceCfg->listOfConfirguration)
         {
            for(sliceIdx = 0; sliceIdx<storedSliceCfg->numOfSliceConfigured; sliceIdx++)
            {
               if(!memcmp(&schSliceCfgReq->listOfConfirguration[cfgIdx]->snssai, &storedSliceCfg->listOfConfirguration[sliceIdx]->snssai,\
                        sizeof(Snssai)))
               {
                  (*count)++;
                  sliceFound = true;
                  break;
               }
            }
         }
      }

      SCH_ALLOC(schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx], sizeof(SliceRsp));
      if(schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx] == NULLP)
      {
         DU_LOG("\nERROR  -->  SCH : Failed to allocate memory in fillSliceCfgRsp");
         return RFAILED;
      }

      
      schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx]->snssai = schSliceCfgReq->listOfConfirguration[cfgIdx]->snssai;
      if(sliceFound == true)
         schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx]->rsp    = RSP_OK;
      else
      {
         schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx]->rsp    = RSP_NOK;
         schSliceCfgRsp->listOfSliceCfgRsp[cfgIdx]->cause  = SLICE_NOT_FOUND;
      }
   }
   return ROK;
}

/*******************************************************************************
 *
 * @brief This function is used to store the slice configuration Sch DB
 *
 * @details
 *
 *    Function : addSliceCfgInSchDb 
 *
 *    Functionality:
 *     function is used to store the slice configuration Sch DB
 *
 * @params[in] SchSliceCfg *storeSliceCfg, SchSliceCfgReq *cfgReq,
 * SchSliceCfgRsp cfgRsp, uint8_t count
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 *
 * ********************************************************************************/
uint8_t addSliceCfgInSchDb(SchSliceCfg *storeSliceCfg, SchSliceCfgReq *cfgReq, SchSliceCfgRsp cfgRsp, uint8_t count)
{
   uint8_t cfgIdx = 0, sliceIdx = 0; 
   
   if(count)
   {
      storeSliceCfg->numOfSliceConfigured = count;
      SCH_ALLOC(storeSliceCfg->listOfConfirguration, storeSliceCfg->numOfSliceConfigured * sizeof(SchRrmPolicyOfSlice*));
      if(storeSliceCfg->listOfConfirguration == NULLP)
      {
         DU_LOG("\nERROR  -->  SCH : Failed to allocate memory in addSliceCfgInSchDb");
         return RFAILED;
      }

      for(cfgIdx = 0; cfgIdx<storeSliceCfg->numOfSliceConfigured; cfgIdx++)
      {
         if(cfgRsp.listOfSliceCfgRsp[cfgIdx]->rsp == RSP_OK)
         {
            SCH_ALLOC(storeSliceCfg->listOfConfirguration[sliceIdx], sizeof(SchRrmPolicyOfSlice));
            if(storeSliceCfg->listOfConfirguration[sliceIdx] == NULLP)
            {
               DU_LOG("\nERROR  -->  SCH : Failed to allocate memory in addSliceCfgInSchDb");
               return RFAILED;
            }

            SCH_ALLOC(storeSliceCfg->listOfConfirguration[sliceIdx]->rrmPolicyRatioInfo, sizeof(SchRrmPolicyRatio));
            if(storeSliceCfg->listOfConfirguration[sliceIdx]->rrmPolicyRatioInfo == NULLP)
            {
               DU_LOG("\nERROR  -->  SCH : Failed to allocate memory in addSliceCfgInSchDb");
               return RFAILED;
            }

            memcpy(storeSliceCfg->listOfConfirguration[sliceIdx], cfgReq->listOfConfirguration[sliceIdx], sizeof(SchRrmPolicyOfSlice));
            sliceIdx++;
         }
      }
   }
   return ROK;
}

/*******************************************************************************
 *
 * @brief This function is used to free the slice cfg and re cfg request pointer
 *
 * @details
 *
 *    Function : freeSchSliceCfgReq 
 *
 *    Functionality:
 *     function is used to free the slice cfg and re cfg request pointer
 *
 * @params[in] Pst *pst, SchSliceCfgReq *schSliceCfgReq
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 * ********************************************************************************/
void freeSchSliceCfgReq(SchSliceCfgReq *cfgReq)
{
   uint8_t cfgIdx = 0;
   
   if(cfgReq)
   {
      if(cfgReq->numOfConfiguredSlice)
      {
         for(cfgIdx = 0; cfgIdx<cfgReq->numOfConfiguredSlice; cfgIdx++)
         {
            if(cfgReq->listOfConfirguration[cfgIdx])
            {
               SCH_FREE(cfgReq->listOfConfirguration[cfgIdx]->rrmPolicyRatioInfo, sizeof(SchRrmPolicyRatio));
               SCH_FREE(cfgReq->listOfConfirguration[cfgIdx], sizeof(SchRrmPolicyOfSlice));
            }
         }
         SCH_FREE(cfgReq->listOfConfirguration, cfgReq->numOfConfiguredSlice * sizeof(SchRrmPolicyOfSlice*));
      }
      SCH_FREE(cfgReq, sizeof(SchSliceCfgReq));
   }
}
/*******************************************************************************
 *
 * @brief This function is used to store the slice configuration Sch DB
 *
 * @details
 *
 *    Function : MacSchSliceCfgReq 
 *
 *    Functionality:
 *     function is used to store the slice configuration Sch DB
 *
 * @params[in] Pst *pst, SchSliceCfgReq *schSliceCfgReq
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 *
 * ********************************************************************************/
uint8_t MacSchSliceCfgReq(Pst *pst, SchSliceCfgReq *schSliceCfgReq)
{
   uint8_t count = 0;
   Inst   inst = pst->dstInst - 1;
   SchSliceCfgRsp sliceCfgRsp;

   DU_LOG("\nINFO  -->  SCH : Received Slice Cfg request from MAC");
   if(schSliceCfgReq)
   {
      if(schSliceCfgReq->listOfConfirguration)
      {
         /* filling the slice configuration response of each slice */
         if(fillSliceCfgRsp(false, NULLP, schCb[inst].cells[0], schSliceCfgReq, &sliceCfgRsp, &count) != ROK)
         {
            DU_LOG("\nERROR  -->  SCH : Failed to fill the slice cfg rsp");
            return RFAILED;
         }

         if(addSliceCfgInSchDb(&schCb[inst].sliceCfg, schSliceCfgReq, sliceCfgRsp, count) != ROK)
         {
            DU_LOG("\nERROR  -->  SCH : Failed to add slice cfg in sch database");
            return RFAILED;
         }
         freeSchSliceCfgReq(schSliceCfgReq);
         SchSendSliceCfgRspToMac(inst, sliceCfgRsp);
      }
   }
   else
   {
      DU_LOG("\nERROR  -->  SCH : Received SchSliceCfgReq is NULL");
   }
   return ROK;
}

/*******************************************************************************
 *
 * @brief This function is used to store the slice reconfiguration Sch DB
 *
 * @details
 *
 *    Function : modifySliceCfgInSchDb 
 *
 *    Functionality:
 *     function is used to store the slice re configuration Sch DB
 *
 * @params[in] Pst *pst, SchSliceCfgReq *schSliceCfgReq
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 *
 * ********************************************************************************/
uint8_t modifySliceCfgInSchDb(SchSliceCfg *storeSliceCfg, SchSliceCfgReq *cfgReq, SchSliceCfgRsp cfgRsp, uint8_t count)
{
   uint8_t cfgIdx = 0, sliceIdx = 0; 

   if(count)
   {
      if(storeSliceCfg->listOfConfirguration == NULLP)
      {
         DU_LOG("\nINFO  -->  SCH : Memory allocation failed in modifySliceCfgInSchDb");
         return RFAILED;
      }

      for(cfgIdx = 0; cfgIdx<cfgReq->numOfConfiguredSlice; cfgIdx++)
      {
         if(cfgRsp.listOfSliceCfgRsp[cfgIdx]->rsp == RSP_OK)
         {
            for(sliceIdx = 0; sliceIdx<storeSliceCfg->numOfSliceConfigured; sliceIdx++)
            {
               if(!memcmp(&storeSliceCfg->listOfConfirguration[sliceIdx]->snssai, &cfgReq->listOfConfirguration[cfgIdx]->snssai, sizeof(Snssai)))
               {
                  storeSliceCfg->listOfConfirguration[sliceIdx]->rrmPolicyRatioInfo = cfgReq->listOfConfirguration[cfgIdx]->rrmPolicyRatioInfo;
                  break;
               }
            }
         }
      }
   }
   freeSchSliceCfgReq(cfgReq);
   return ROK;
}
/*******************************************************************************
 *
 * @brief This function is used to send Slice re Cfg rsp to MAC
 *
 * @details
 *
 *    Function : SchSendSliceCfgRspToMac
 *
 *    Functionality:
 *     function is used to send Slice re Cfg rsp to MAC
 *
 * @params[in] Pst *pst, SchSliceCfgRsp schSliceReCfgRsp
 *
 * @return- void
 *
 * ********************************************************************************/
void SchSendSliceReCfgRspToMac(Inst inst, SchSliceCfgRsp schSliceReCfgRsp)
{
   Pst rspPst;
   
   memset(&rspPst, 0, sizeof(Pst));
   FILL_PST_SCH_TO_MAC(rspPst, inst);
   rspPst.event = EVENT_SLICE_RECFG_RSP_TO_MAC;
   
   SchSliceReCfgRspOpts[rspPst.selector](&rspPst, &schSliceReCfgRsp);
}
/*******************************************************************************
 *
 * @brief This function is used to store the slice reconfiguration Sch DB
 *
 * @details
 *
 *    Function : MacSchSliceReCfgReq 
 *
 *    Functionality:
 *     function is used to store the slice re configuration Sch DB
 *
 * @params[in] Pst *pst, SchSliceCfgReq *schSliceReCfgReq
 *
 * @return
 *        ROK - Success
 *        RFAILED - Failure
 *
 * ********************************************************************************/
uint8_t MacSchSliceReCfgReq(Pst *pst, SchSliceCfgReq *schSliceReCfgReq)
{
   uint8_t count = 0;
   Inst   inst = pst->dstInst - 1;
   SchSliceCfgRsp schSliceReCfgRsp;

   DU_LOG("\nINFO  -->  SCH : Received Slice ReCfg request from MAC");
   if(schSliceReCfgReq)
   {
      if(schSliceReCfgReq->listOfConfirguration)
      {
         /* filling the slice configuration response of each slice */
         if(fillSliceCfgRsp(true, &schCb[inst].sliceCfg, NULLP, schSliceReCfgReq, &schSliceReCfgRsp, &count) != ROK)
         {
            DU_LOG("\nERROR  -->  SCH : Failed to fill sch slice cfg response");
            return RFAILED;
         }
         
         /* Modify the slice configuration stored in schCb */
         if(modifySliceCfgInSchDb(&schCb[inst].sliceCfg, schSliceReCfgReq, schSliceReCfgRsp, count) != ROK)
         {
            DU_LOG("\nERROR  -->  SCH : Failed to modify slice cfg of SchDb");
            return RFAILED;
         }
         SchSendSliceReCfgRspToMac(inst, schSliceReCfgRsp);
      }
   }
   else
   {
      DU_LOG("\nERROR  -->  SCH : Received SchSliceCfgReq is NULL");
   }
   return ROK;
}

/**********************************************************************
  End of file
 **********************************************************************/
