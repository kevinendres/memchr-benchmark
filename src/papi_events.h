#ifndef __PAPI_EVENTS_H_
#define __PAPI_EVENTS_H_

#include <papi.h>

const int L1_events[10] = {PAPI_L1_DCM, PAPI_L1_ICM, PAPI_L1_LDM, PAPI_L1_STM, PAPI_L1_TCM};
const int L2_data_events[10] = {PAPI_L2_TCA, PAPI_L2_TCM, PAPI_L2_TCR, PAPI_L2_TCW, PAPI_L2_LDM, PAPI_L2_STM, \
PAPI_L2_DCA, PAPI_L2_DCM, PAPI_L2_DCR, PAPI_L2_DCW};
const int L2_instruction_events[10] = {PAPI_L2_TCA, PAPI_L2_TCM, PAPI_L2_TCR, PAPI_L2_TCW, PAPI_L2_LDM, \
PAPI_L2_STM, PAPI_L2_ICA, PAPI_L2_ICH, PAPI_L2_ICM, PAPI_L2_ICR};
const int L3_events[10] = {PAPI_L3_DCA, PAPI_L3_DCR, PAPI_L3_DCW, PAPI_L3_ICA, PAPI_L3_ICR, PAPI_L3_LDM, \
PAPI_L3_TCA, PAPI_L3_TCM, PAPI_L3_TCR, PAPI_L3_TCW};
const int conditional_branching_events[10] = {PAPI_BR_CN, PAPI_BR_INS, PAPI_BR_MSP, PAPI_BR_NTK, \
PAPI_BR_PRC, PAPI_BR_TKN, PAPI_BR_UCN};
const int cache_request_events[10] = {PAPI_CA_CLN, PAPI_CA_ITV, PAPI_CA_SHR, PAPI_CA_SNP};
const int instruct_counting_events[10] = {PAPI_FUL_CCY, PAPI_FUL_ICY, PAPI_TOT_CYC, \
PAPI_TOT_INS, PAPI_VEC_DP, PAPI_VEC_SP, PAPI_DP_OPS, PAPI_SP_OPS};
const int data_access_events[10] = {PAPI_LD_INS, PAPI_LST_INS, PAPI_MEM_WCY, PAPI_PRF_DM, \
PAPI_REF_CYC, PAPI_RES_STL, PAPI_SR_INS, PAPI_STL_CCY, PAPI_STL_ICY};
const int TLB_events[10] = {PAPI_TLB_DM, PAPI_TLB_IM};

void create_PAPI_event_set(int* event_set)
{
    for (int i = 0; i < 10; ++i) {
        PAPI_add_event(*event_set, L1_events[i]);
    }

}

#endif // __PAPI_EVENTS_H_
