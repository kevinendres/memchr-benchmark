#ifndef __PAPI_EVENTS_H_
#define __PAPI_EVENTS_H_

#include <papi.h>
#include <string.h>

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
const int instruction_counting_events[10] = {PAPI_FUL_CCY, PAPI_FUL_ICY, PAPI_TOT_CYC, \
PAPI_TOT_INS, PAPI_VEC_DP, PAPI_VEC_SP, PAPI_DP_OPS, PAPI_SP_OPS};
const int data_access_events[10] = {PAPI_LD_INS, PAPI_LST_INS, PAPI_MEM_WCY, PAPI_PRF_DM, \
PAPI_REF_CYC, PAPI_RES_STL, PAPI_SR_INS, PAPI_STL_CCY, PAPI_STL_ICY};
const int TLB_events[10] = {PAPI_TLB_DM, PAPI_TLB_IM};

void PAPI_error(int PAPI_return)
{
    printf("PAPI error %d: %s\n", PAPI_return, PAPI_strerror(PAPI_return));
    exit(EXIT_FAILURE);
}

void load_PAPI_events(int* event_set, int* event_category)
{
    for (int i = 0; i < 10; ++i) {
        if (PAPI_add_event(*event_set, event_category[i]) != PAPI_OK) {
            //PAPI_error(11);
        }
    }
}

void choose_event_category(char* optarg, int* event_category)
{
    if (strcmp(optarg, "L1") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = L1_events[i];
        }
    }
    else if (strcmp(optarg, "L2d") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = L2_data_events[i];
        }
    }
    else if (strcmp(optarg, "L2i") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = L2_instruction_events[i];
        }
    }
    else if (strcmp(optarg, "L3") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = L3_events[i];
        }
    }
    else if (strcmp(optarg, "branching") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = conditional_branching_events[i];
        }
    }
    else if (strcmp(optarg, "cachelines") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = cache_request_events[i];
        }
    }
    else if (strcmp(optarg, "instructions") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = instruction_counting_events[i];
        }
    }
    else if (strcmp(optarg, "data") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = data_access_events[i];
        }
    }
    else if (strcmp(optarg, "TLB") == 0) {
        for (int i = 0; i < 10; ++i) {
            event_category[i] = TLB_events[i];
        }
    }
    else{
        printf("PAPI event category \"%s\" unknown\n", optarg);
    }
}

#endif // __PAPI_EVENTS_H_
