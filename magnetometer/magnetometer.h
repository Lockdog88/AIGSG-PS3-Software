#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

#include "bmx1xx.h"
#include "qm_gpio.h"


#define DEBUG0

#define BMC150_ADDR 0x13

#define BMC150_REG_HIGH_THRESHOLD 0x50
#define BMC150_REG_LOW_THRESHOLD 0x4F
#define BMC150_REG_HIGH_THRESHOLD_AXES_SET 0x4D
#define BMC150_REG_LOW_THRESHOLD_AXES_SET 0x4D
#define BMC150_REG_INTERRUPT 0x4E

#define X_H_THRESHOLD 0x2A /*XDATA/16*/
#define X_L_THRESHOLD 0x10 /*XDATA/16*/

#define M_PI 3.14159265358979323846

void mag_init(void);
void bmc150_mag_set_h_threshold(uint8_t threshold);
void bmc150_mag_set_l_threshold(uint8_t threshold);
void bmc150_mag_set_threshold(uint8_t low, uint8_t high);
void bmc150_clear_threshold(void);

/* Clear Interrupt Register */
void bmc150_mag_read_reg_int(void);

void bmc150_mag_callback(void* data);

#endif /* MAGNETOMETER_H_ */
