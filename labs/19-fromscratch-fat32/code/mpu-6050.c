#include "rpi.h"
#include "mpu-6050.h"

void imu_wr(uint8_t addr, uint8_t reg, uint8_t v) {
#if !(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#   error "Not Little Endian!"
#endif
    uint16_t cmd = reg | (v << 8);
    i2c_write(addr, (uint8_t *) &cmd, 2);
}

uint8_t imu_rd(uint8_t addr, uint8_t reg) {
    uint8_t v;
    i2c_write(addr, &reg, 1);
    i2c_read(addr, &v, 1);
    return v;
}

int imu_rd_n(uint8_t addr, uint8_t base_reg, uint8_t *v, uint32_t n) {
    return i2c_write(addr, &base_reg, 1) && i2c_read(addr, v, n);
}

enum {
    PWR_MGMT_1 = 107,
    DEVICE_RESET = 1 << 7,
    SLEEP = 1 << 6,
    USER_CTRL = 106,
    FIFO_EN = 1 << 6,
    ALL_RESET = 0b111,
    INT_PIN_CFG = 55,
    LATCH_INT_EN = 1 << 5,
    INT_READ_CLEAR = 1 << 4,
    INT_ENABLE = 56,
    DATA_RDY_EN = 1 << 0,
    INT_STATUS = 58,
    DATA_RDY_INT = 1 << 0,
    ACCEL_CONFIG = 28,
    ACCEL_FS_SEL_2G = 0b00 << 3,
    ACCEL_XOUT_H = 59,
    GYRO_CONFIG = 27,
    GYRO_FS_SEL_500DPS = 0b01 << 3,
    GYRO_XOUT_H = 67,
};

void mpu6050_reset(uint8_t addr) {
    delay_ms(100);
    imu_wr(addr, PWR_MGMT_1, DEVICE_RESET); // ~p. 41
    delay_ms(100);
    assert((imu_rd(addr, PWR_MGMT_1) & SLEEP) && "device did not start in sleep mode"); // ~p. 41 && Dawson
    imu_wr(addr, PWR_MGMT_1, 0); // ~p. 41
    delay_ms(100);
    imu_wr(addr, USER_CTRL, FIFO_EN); // ~p. 39
    delay_ms(100);
    imu_wr(addr, INT_PIN_CFG, LATCH_INT_EN | INT_READ_CLEAR); // ~p. 27
    imu_wr(addr, INT_ENABLE, DATA_RDY_EN); // ~p. 28
}

accel_t mpu6050_accel_init(uint8_t addr, unsigned accel_g) {
    assert(accel_g == accel_2g && "only 2g supported");
    imu_wr(addr, ACCEL_CONFIG, ACCEL_FS_SEL_2G); // ~p. 15
    return (accel_t) { .addr = addr, .g = 2, .hz = 20 };
}

int accel_has_data(const accel_t *h) {
    return imu_rd(h->addr, INT_STATUS) & DATA_RDY_INT; //~p. 29
}

// blocking read of accel: returns raw (x,y,z) reading.
imu_xyz_t accel_rd(const accel_t *h) {
    while(!accel_has_data(h))
        ; //spin
    uint8_t v[6];
    imu_rd_n(h->addr, ACCEL_XOUT_H, v, 6); //~p. 300
    return (imu_xyz_t) {
        // Big endian device
        .x = (int16_t) ((v[0] << 8) | v[1]),
        .y = (int16_t) ((v[2] << 8) | v[3]),
        .z = (int16_t) ((v[4] << 8) | v[5]),
    };
}

// scale a reading returned by <accel_rd> to the sensitivity
// the device was initialized to.
imu_xyz_t accel_scale(accel_t *h, imu_xyz_t xyz) {
    return (imu_xyz_t) {
        .x = xyz.x * 2000 / SHRT_MAX,
        .y = xyz.y * 2000 / SHRT_MAX,
        .z = xyz.z * 2000 / SHRT_MAX,
    };
}

gyro_t mpu6050_gyro_init(uint8_t addr, unsigned gyro_dps) {
    assert(gyro_dps == gyro_500dps && "only 500dps supported");
    imu_wr(addr, GYRO_CONFIG, GYRO_FS_SEL_500DPS); // ~p. 14
    return (gyro_t) { .addr = addr, .dps = 500, .hz = 20 };
}

int gyro_has_data(const gyro_t *h) {
    return imu_rd(h->addr, INT_STATUS) & DATA_RDY_INT;
}

// raw reading of gyro.
imu_xyz_t gyro_rd(const gyro_t *h) {
    while(!gyro_has_data(h))
        ; //spin
    uint8_t v[6];
    imu_rd_n(h->addr, GYRO_XOUT_H, v, 6);
    return (imu_xyz_t) {
        // Big endian device
        .x = (int16_t) ((v[0] << 8) | v[1]),
        .y = (int16_t) ((v[2] << 8) | v[3]),
        .z = (int16_t) ((v[4] << 8) | v[5]),
    };
}

// scale reading by the initialized dps.
imu_xyz_t gyro_scale(gyro_t *h, imu_xyz_t xyz) {
    return (imu_xyz_t) {
        .x = xyz.x * 500 / SHRT_MAX,
        .y = xyz.y * 500 / SHRT_MAX,
        .z = xyz.z * 500 / SHRT_MAX,
    };
}