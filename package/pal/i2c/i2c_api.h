#ifndef _I2C_API_H_
#define _I2C_API_H_

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the given I2C bus
 *
 * The bus MUST not be acquired before initializing it, as this is handled
 * internally by the i2c_init function!
 *
 * @param[in] dev       I2C device port number to initialize
 */
int i2c_init(int dev, int sda, int scl);

/**
 * @brief   Get mutually exclusive access to the given I2C bus
 *
 * In case the I2C device is busy, this function will block until the bus is
 * free again.
 *
 * @param[in] dev       I2C device to lock for starting some action
 */
void i2c_lock(int dev);

/**
 * @brief   Release the given I2C device to be used by others
 *
 * @param[in] dev       I2C device to unlock after some action is done
 */
void i2c_unlock(int dev);

/**
 * @brief   Convenience function for reading one byte from a given register
 *          address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */

int i2c_read_reg(int dev, uint16_t addr, uint8_t reg, uint8_t *data, uint8_t flags);

/**
 * @brief   Convenience function for reading several bytes from a given
 *          register address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into @p data
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */
int i2c_read_regs(int dev, uint16_t addr, uint8_t reg, uint8_t *data, size_t len, uint8_t flags);

/**
 * @brief   Convenience function for reading one byte from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */

int i2c_read_byte(int dev, uint16_t addr, uint8_t *data, uint8_t flags);

/**
 * @brief   Convenience function for reading bytes from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into @p data
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */

int i2c_read_bytes(int dev, uint16_t addr, uint8_t *data, size_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing a single byte onto the bus
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          byte to write to the device
 * @param[in] flags         optional flags
 *
 * @return                  0 When success otherwise negative
 */
int i2c_write_byte(int dev, uint16_t addr, uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing several bytes onto the bus
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          array holding the bytes to write to the device
 * @param[in] len           the number of bytes to write
 * @param[in] flags         optional flags
 *
 * @return                  0 When success otherwise negative
 */
int i2c_write_bytes(int dev, uint16_t addr, const uint8_t *data, size_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing one byte to a given
 *          register address
 *
 * @note    This function is using a continuous sequence for writing to the
 *          specified register address. It first writes the register then data.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[in]  data         byte to write
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */
int i2c_write_reg(int dev, uint16_t addr, uint8_t reg, uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing data to a given register address
 *
 * @note    This function is using a continuous sequence for writing to the
 *          specified register address. It first writes the register then data.
 *
 * @pre     i2c_lock must be called before accessing the bus
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to write
 * @param[in]  flags        optional flags
 *
 * @return                  0 When success otherwise negative
 */
int i2c_write_regs(int dev, uint16_t addr, uint8_t reg, const uint8_t *data, size_t len, uint8_t flags);

#ifdef __cplusplus
}
#endif

#endif /* _I2C_API_H_ */
