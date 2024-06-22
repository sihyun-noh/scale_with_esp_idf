#ifndef __SCALE_READ_485_H__
#define __SCALE_READ_485_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UNIT_G = 0x01,
  UNIT_KG,
} weight_unit_t;

typedef struct {
  unsigned int scale_Max;
  unsigned int scale_Min;
  unsigned int e_d;
  weight_unit_t unit;
} indi_mode_spac_t;

typedef struct {
  unsigned char *data;
  int size;
  int weight_pos;
  int weight_len;
} printer_data_t;

typedef enum {
  STATE_NONE = 0x00,
  STATE_OK,
  STATE_ZERO_EVENT,
  STATE_STABLE_EVENT,
  STATE_TARE_EVENT,
  STATE_SIGN_EVENT,
  STATE_TRASH_CHECK_EVENT,
  STATE_PRINT_EVENT,
} weight_evnet_t;

typedef enum {
  MODEL_NONE = 0x00,
  MODEL_CAS_WTM500,
  MODEL_BAYKON_BX11,
  MODEL_CAS_NT301A,
  MODEL_CAS_EC_D_SERIES,
  MODEL_AND_CB_12K,
  MODEL_ACOM_PW_200,
  MODEL_CAS_SW_11,
  MODEL_INNOTEM_T25,
  MODEL_CAS_MW2_H,
} indicator_model_t;

typedef enum {
  DP_100 = 0x00,
  DP_10,
  DP_1,
  DP_0_1,
  DP_0_01,
  DP_0_001,
  DP_0_0001,
  DP_0_00001,
} decimal_point_t;

typedef enum {
  IS_X1 = 0x00,
  IS_X2,
  IS_X5,
} increment_size_t;

typedef struct Baykon_BX11_data {
  char state_A_B_C[3];
  char indicator[6];
  char tare[6];
} Baykon_BX11_data_t;

typedef struct cas_22byte_format {
  char states[2];
  char measurement_states[2];
  char lamp_states[1];
  char data[8];
  char relay[1];
  char unit[2];
} cas_22byte_format_t;

typedef struct {
  char states[2];
  char measurement_states[2];
  char lamp_states[1];
  char data[8];
  char unit[2];
} cas_nt301a_data_format2_t;

typedef struct {
  char states[2];
  char measurement_states[2];
  char sign[1];
  char data[8];
  char unit[2];
} empty_format_t;

typedef struct Common_data {
  indicator_model_t model;
  decimal_point_t DP;
  increment_size_t IS;
  weight_evnet_t event[30];
  indi_mode_spac_t spec;
  char weight_data[10];
} Common_data_t;

/**
 * @brief
 *
 * @return int
 */
int weight_uart_485_init(void);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */
int indicator_CAS_MW2_H_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_INNOTEM_T25_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_CAS_sw_11_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_ACOM_pw_200_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_AND_CB_12K_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_EC_D_Serise_data(Common_data_t *common_data);
/**
 * @brief
 *
 * @param common_data
 * @return int
 */
int indicator_CAS_NT301A_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @param common_data
 * @return int
 */

int indicator_WTM_500_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @return int
 */
int cas_zero_command(void);

/**
 * @brief
 *
 * @param data
 * @return int
 */

int indicator_BX11_data(Common_data_t *common_data);

/**
 * @brief
 *
 * @return int
 */
int baykon_bx11_zero_command(void);

/**
 * @brief weight printf
 *
 */
void weight_print_msg(char *s_weight, weight_unit_t unit);
#ifdef __cplusplus
}
#endif

#endif
