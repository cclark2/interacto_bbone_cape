
//
//      nunchuck.c
//

#ifndef PORT
//#define PORT "/dev/i2c-1"    // reserved for capes and requiring pullup resistors
#define PORT "/dev/i2c-3"      // p9:pin19=SCL, p9:pin20:SDA
#endif


// 303 accelerometer
#define ADDR_ACCEL_303  0x19            // wii nunchuck address: 0x52
// 303 magnetometer
#define ADDR_MAG_303  0x1e            // wii nunchuck address: 0x52
// 330 accelerometer
#define ADDR_ACCEL_330  0x18            // wii nunchuck address: 0x52
// 330 gyroscope
#define ADDR_GYRO_330  0x6b            // wii nunchuck address: 0x52

// camera
#define ADDR_CAM  0x3C            // wii nunchuck address: 0x52


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <sstream>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mem_map.h"

#define error(x...) { fprintf(stderr, "\nE%d: ", __LINE__); fprintf(stderr, x); fprintf(stderr, "\n\n"); exit(1); }

#define DDR_BASEADDR 0x80000000
#define OFFSET_DDR	 (224*1024*1024)
#define PRU_DDR_SIZE (16*1024*1024)
#define PRU0_DDR    ( DDR_BASEADDR + OFFSET_DDR )
#define PRU1_DDR    ( PRU0_DDR +PRU_DDR_SIZE)


int main()
{
  // gpio 2.3 (timer7) = lcd data gate = 67 (gpmc_oen_ren)
  // gpio 1.6 = lcd cs = 38 (gpmc_ad6)
  // gpio 1.7 = lcd rs = 39 (gpmc_ad7)
  // gpio 1.2 = lcd wr = 34 (gpmc_ad2)
  // gpio 1.28 = lcd rd = 60 (gpmc_ben1)
  // gpio 2.2 (timer4) lcd reset 66 (gpmc_advn_ale)
  
  // gpio 3.14 (spi1_sclk) = lcd db8 = pru0.0 = 110 (mcasp0_aclkx)
  // gpio 3.15 (spi1_d0) = lcd db9 = pru0.1 = 111 (mcasp0_fsx)
  // gpio 3.16 (spi1_d1) = lcd db10 = pru0.2 = 112 (mcasp0_axr0)
  // gpio 3.17 (spi1_cs0) = lcd db11 = pru0.3 = 113 (mcasp0_ahclkr)
  // gpio 3.19 = lcd db12 = pru0.5 =115 (mcasp0_fsr)
  // gpio 3.21 = lcd db13 = pru0.7 = 117 (mcasp0_ahclkx)
  // gpio 1.12 = lcd db14 = pru0.14 = 44 (gpmc_ad12)
  // gpio 1.13 = lcd db15 = pru0.15 = 45 (gpmc_ad13)
  
  system( "echo 67 > /sys/class/gpio/unexport" );
  system( "echo 38 > /sys/class/gpio/unexport" );
  system( "echo 39 > /sys/class/gpio/unexport" );
  system( "echo 34 > /sys/class/gpio/unexport" );
  system( "echo 60 > /sys/class/gpio/unexport" );
  system( "echo 66 > /sys/class/gpio/unexport" );
  
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_oen_ren" ) )
    throw std::runtime_error( "Failed to set mux for lcd data gate" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad6" ) )
    throw std::runtime_error( "Failed to set mux for lcd cs" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad7" ) )
    throw std::runtime_error( "Failed to set mux for lcd rs" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad2" ) )
    throw std::runtime_error( "Failed to set mux for lcd wr" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ben1" ) )
    throw std::runtime_error( "Failed to set mux for lcd rd" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_advn_ale" ) )
    throw std::runtime_error( "Failed to set mux for lcd reset" );
  
  if( 0 != system( "echo 67 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd data gate" );
  if( 0 != system( "echo 38 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd cs" );
  if( 0 != system( "echo 39 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd rs" );
  if( 0 != system( "echo 34 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd wr" );
  if( 0 != system( "echo 60 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd rd" );
  if( 0 != system( "echo 66 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd reset" );
    
  if( 0 != system( "echo out > /sys/class/gpio/gpio67/direction" ) )
    throw std::runtime_error( "Failed to set lcd data gate gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio38/direction" ) )
    throw std::runtime_error( "Failed to set lcd cs gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio39/direction" ) )
    throw std::runtime_error( "Failed to set lcd rs gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio34/direction" ) )
    throw std::runtime_error( "Failed to set lcd wr gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio60/direction" ) )
    throw std::runtime_error( "Failed to set lcd rd gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio66/direction" ) )
    throw std::runtime_error( "Failed to set lcd reset gpio direction" );
    
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_aclkx" ) )
    throw std::runtime_error( "Failed to set mux for lcd db8" );
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_fsx" ) )
    throw std::runtime_error( "Failed to set mux for lcd db9" );
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_axr0" ) )
    throw std::runtime_error( "Failed to set mux for lcd db10" );
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_ahclkr" ) )
    throw std::runtime_error( "Failed to set mux for lcd db11" );
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_fsr" ) )
    throw std::runtime_error( "Failed to set mux for lcd db12" );
  if( 0 != system( "echo 05 > /sys/kernel/debug/omap_mux/mcasp0_ahclkx" ) )
    throw std::runtime_error( "Failed to set mux for lcd db13" );
  if( 0 != system( "echo 06 > /sys/kernel/debug/omap_mux/gpmc_ad12" ) )
    throw std::runtime_error( "Failed to set mux for lcd db14" );
  if( 0 != system( "echo 06 > /sys/kernel/debug/omap_mux/gpmc_ad13" ) )
    throw std::runtime_error( "Failed to set mux for lcd db15" );
  
  
  
  std::ofstream lcd_pru_gate( "/sys/class/gpio/gpio67/value" );
  std::ofstream lcd_pru_cs( "/sys/class/gpio/gpio38/value" );
  std::ofstream lcd_pru_rs( "/sys/class/gpio/gpio39/value" );
  std::ofstream lcd_pru_wr( "/sys/class/gpio/gpio34/value" );
  std::ofstream lcd_pru_rd( "/sys/class/gpio/gpio60/value" );
  std::ofstream lcd_pru_reset( "/sys/class/gpio/gpio66/value" );
  
  if( !lcd_pru_gate.is_open() || !lcd_pru_cs.is_open() || !lcd_pru_rs.is_open() || !lcd_pru_wr.is_open()
      || !lcd_pru_rd.is_open() || !lcd_pru_reset.is_open()  )
    throw std::runtime_error( "Failed to open lcd pru gpios" );

 
  lcd_pru_cs << '1' << std::endl;
  lcd_pru_rs << '1' << std::endl;
  lcd_pru_wr << '1' << std::endl;
  lcd_pru_rd << '1' << std::endl;
  lcd_pru_gate << '0' << std::endl;
  lcd_pru_reset << '0' << std::endl;
  usleep( 100000 );
  lcd_pru_reset << '1' << std::endl;

  
  
  
  
  
  
  // gpio 0.22 (ehrpwm2a) = lcd cs = 22 (gpmc_ad8)
  // gpio 0.23 (ehrpwm2b) = lcd rs = 23 (gpmc_ad9)
  // gpio 0.30 (uart4_rxd) = lcd wr = 30 (gpmc_wait0)
  // gpio 1.1 = lcd rd = 33 (gpmc_ad1)
  // gpio 0.26 = lcd reset = 26 (gpmc_ad10)
  // gpio 0.2 (uart2_rxd) = lcd db8 = 2 (spi0_sclk)
  // gpio 0.3 (uart2_txd) = lcd db9 = 3 (spi0_d0)
  // gpio 0.4 (i2c1_sda) = lcd db10 = 4 (spi0_d1)
  // gpio 0.5 (i2c1_scl) = lcd db11 = 5 (spi0_cs0)
  // gpio 0.8 (uart4_ctsn) = lcd db12 = 8 (lcd_data12)
  // gpio 0.9 (uart4_rtsn) = lcd db13 = 9 (lcd_data13)
  // gpio 0.10 (uart5_ctsn) = lcd db14 = 10 (lcd_data14)
  // gpio 0.11 (uart5_rtsn) = lcd db15 = 11 (lcd_data15)
 
  system( "echo 22 > /sys/class/gpio/unexport" );
  system( "echo 23 > /sys/class/gpio/unexport" );
  system( "echo 30 > /sys/class/gpio/unexport" );
  system( "echo 33 > /sys/class/gpio/unexport" );
  system( "echo 26 > /sys/class/gpio/unexport" );
  system( "echo 2 > /sys/class/gpio/unexport" );
  system( "echo 3 > /sys/class/gpio/unexport" );
  system( "echo 4 > /sys/class/gpio/unexport" );
  system( "echo 5 > /sys/class/gpio/unexport" );
  system( "echo 8 > /sys/class/gpio/unexport" );
  system( "echo 9 > /sys/class/gpio/unexport" );
  system( "echo 10 > /sys/class/gpio/unexport" );
  system( "echo 11 > /sys/class/gpio/unexport" );
  
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad8" ) )
    throw std::runtime_error( "Failed to set mux for lcd cs" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad9" ) )
    throw std::runtime_error( "Failed to set mux for lcd rs" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_wait0" ) )
    throw std::runtime_error( "Failed to set mux for lcd wr" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad1" ) )
    throw std::runtime_error( "Failed to set mux for lcd rd" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad10" ) )
    throw std::runtime_error( "Failed to set mux for lcd reset" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/spi0_sclk" ) )
    throw std::runtime_error( "Failed to set mux for lcd db8" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/spi0_d0" ) )
    throw std::runtime_error( "Failed to set mux for lcd db9" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/spi0_d1" ) )
    throw std::runtime_error( "Failed to set mux for lcd db10" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/spi0_cs0" ) )
    throw std::runtime_error( "Failed to set mux for lcd db11" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/lcd_data12" ) )
    throw std::runtime_error( "Failed to set mux for lcd db12" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/lcd_data13" ) )
    throw std::runtime_error( "Failed to set mux for lcd db13" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/lcd_data14" ) )
    throw std::runtime_error( "Failed to set mux for lcd db14" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/lcd_data15" ) )
    throw std::runtime_error( "Failed to set mux for lcd db15" );
    
  if( 0 != system( "echo 22 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd cs" );
  if( 0 != system( "echo 23 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd rs" );
  if( 0 != system( "echo 30 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd wr" );
  if( 0 != system( "echo 33 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd rd" );
  if( 0 != system( "echo 26 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd reset" );
  if( 0 != system( "echo 2 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db8" );
  if( 0 != system( "echo 3 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db9" );
  if( 0 != system( "echo 4 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db10" );
  if( 0 != system( "echo 5 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db11" );
  if( 0 != system( "echo 8 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db12" );
  if( 0 != system( "echo 9 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db13" );
  if( 0 != system( "echo 10 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db14" );
  if( 0 != system( "echo 11 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export lcd db15" );
    
  if( 0 != system( "echo out > /sys/class/gpio/gpio22/direction" ) )
    throw std::runtime_error( "Failed to set lcd cs gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio23/direction" ) )
    throw std::runtime_error( "Failed to set lcd rs gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio30/direction" ) )
    throw std::runtime_error( "Failed to set lcd wr gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio33/direction" ) )
    throw std::runtime_error( "Failed to set lcd rd gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio26/direction" ) )
    throw std::runtime_error( "Failed to set lcd reset gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio2/direction" ) )
    throw std::runtime_error( "Failed to set lcd db8 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio3/direction" ) )
    throw std::runtime_error( "Failed to set lcd db9 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio4/direction" ) )
    throw std::runtime_error( "Failed to set lcd db10 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio5/direction" ) )
    throw std::runtime_error( "Failed to set lcd db11 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio8/direction" ) )
    throw std::runtime_error( "Failed to set lcd db12 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio9/direction" ) )
    throw std::runtime_error( "Failed to set lcd db13 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio10/direction" ) )
    throw std::runtime_error( "Failed to set lcd db14 gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio11/direction" ) )
    throw std::runtime_error( "Failed to set lcd db15 gpio direction" );
    

    
    
    
  std::ofstream lcd_cs( "/sys/class/gpio/gpio22/value" );
  std::ofstream lcd_rs( "/sys/class/gpio/gpio23/value" );
  std::ofstream lcd_wr( "/sys/class/gpio/gpio30/value" );
  std::ofstream lcd_rd( "/sys/class/gpio/gpio33/value" );
  std::ofstream lcd_reset( "/sys/class/gpio/gpio26/value" );
  std::ofstream lcd_db8( "/sys/class/gpio/gpio2/value" );
  std::ofstream lcd_db9( "/sys/class/gpio/gpio3/value" );
  std::ofstream lcd_db10( "/sys/class/gpio/gpio4/value" );
  std::ofstream lcd_db11( "/sys/class/gpio/gpio5/value" );
  std::ofstream lcd_db12( "/sys/class/gpio/gpio8/value" );
  std::ofstream lcd_db13( "/sys/class/gpio/gpio9/value" );
  std::ofstream lcd_db14( "/sys/class/gpio/gpio10/value" );
  std::ofstream lcd_db15( "/sys/class/gpio/gpio11/value" );
  
  if( !lcd_cs.is_open() || !lcd_rs.is_open() || !lcd_wr.is_open() || !lcd_rd.is_open() || 
      !lcd_reset.is_open() || !lcd_db8.is_open() || !lcd_db9.is_open() || !lcd_db10.is_open() ||
      !lcd_db11.is_open() || !lcd_db12.is_open() || !lcd_db13.is_open() || !lcd_db14.is_open() ||
      !lcd_db15.is_open() )
    throw std::runtime_error( "Failed to open lcd gpios" );

  lcd_cs << '1' << std::endl;
  lcd_rs << '1' << std::endl;
  lcd_wr << '1' << std::endl;
  lcd_rd << '1' << std::endl;
  lcd_reset << '0' << std::endl;
  usleep( 100000 );
  lcd_reset << '1' << std::endl;
  lcd_db8 << '0' << std::endl;
  lcd_db9 << '0' << std::endl;
  lcd_db10 << '0' << std::endl;
  lcd_db11 << '0' << std::endl;
  lcd_db12 << '0' << std::endl;
  lcd_db13 << '0' << std::endl;
  lcd_db14 << '0' << std::endl;
  lcd_db15 << '0' << std::endl;

  
  

  
  // gpio 2.24 = cam reset = 88 (lcd_pclk)
  // gpio 1.29 = cam led = 61 (gpmc_csn0)
  // gpio 2.5 (timer5) = cam data gate = 69 (gpmc_ben0_cle)

  // gpio 2.6 = cam data 0 = pru1.0 = 70 (lcd_data0)
  // gpio 2.7 = cam data 1 = pru1.1 = 71 (lcd_data1)
  // gpio 2.8 = cam data 2 = pru1.2 = 72 (lcd_data2)
  // gpio 2.9 = cam data 3 = pru1.3 = 73 (lcd_data3)
  // gpio 2.10 = cam data 4 = pru1.4 = 74 (lcd_data4)
  // gpio 2.11 = cam data 5 = pru1.5 = 75 (lcd_data5)
  // gpio 2.12 = cam data 6 = pru1.6 = 76 (lcd_data6)
  // gpio 2.13 = cam data 7 = pru1.7 = 77 (lcd_data7)
  // gpio 2.22 = cam vsync = pru1.8 = 86 (lcd_vsync)
  // gpio 2.23 = cam hsync = pru1.9 = 87 (lcd_hsync)
  // gpio 0.14 (uart1_rxd) = cam clock = pru1.16 = 14 (uart1_rxd)

  system( "echo 88 > /sys/class/gpio/unexport" );
  system( "echo 61 > /sys/class/gpio/unexport" );
  system( "echo 69 > /sys/class/gpio/unexport" );
  

  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/lcd_pclk" ) )
    throw std::runtime_error( "Failed to set mux for cam reset gpio" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_csn0" ) )
    throw std::runtime_error( "Failed to set mux for cam led gpio" );
  if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ben0_cle" ) )
    throw std::runtime_error( "Failed to set mux for cam data gate gpio" );
    
  if( 0 != system( "echo 88 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export cam reset gpio" );
  if( 0 != system( "echo 61 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export cam led gpio" );
  if( 0 != system( "echo 69 > /sys/class/gpio/export" ) )
    throw std::runtime_error( "Failed to export cam data gate gpio" );
    
  if( 0 != system( "echo out > /sys/class/gpio/gpio88/direction" ) )
    throw std::runtime_error( "Failed to set cam reset gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio61/direction" ) )
    throw std::runtime_error( "Failed to set cam led gpio direction" );
  if( 0 != system( "echo out > /sys/class/gpio/gpio69/direction" ) )
    throw std::runtime_error( "Failed to set cam data gate gpio direction" );
    
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data0" ) )
    throw std::runtime_error( "Failed to set mux for cam data 0 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data1" ) )
    throw std::runtime_error( "Failed to set mux for cam data 1 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data2" ) )
    throw std::runtime_error( "Failed to set mux for cam data 2 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data3" ) )
    throw std::runtime_error( "Failed to set mux for cam data 3 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data4" ) )
    throw std::runtime_error( "Failed to set mux for cam data 4 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data5" ) )
    throw std::runtime_error( "Failed to set mux for cam data 5 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data6" ) )
    throw std::runtime_error( "Failed to set mux for cam data 6 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_data7" ) )
    throw std::runtime_error( "Failed to set mux for cam data 7 pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_vsync" ) )
    throw std::runtime_error( "Failed to set mux for cam vsync pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/lcd_hsync" ) )
    throw std::runtime_error( "Failed to set mux for cam hsync pru" );
  if( 0 != system( "echo 36 > /sys/kernel/debug/omap_mux/uart1_rxd" ) )
    throw std::runtime_error( "Failed to set mux for cam clock pru" );





  std::ofstream cam_reset( "/sys/class/gpio/gpio88/value" );
  std::ofstream cam_led( "/sys/class/gpio/gpio61/value" );
  std::ofstream cam_data( "/sys/class/gpio/gpio69/value" );
  
  if( !cam_reset.is_open() || !cam_led.is_open() || !cam_data.is_open() )
    throw std::runtime_error( "Failed to open cam gpios" );

  cam_reset << '1' << std::endl;
  cam_led << '1' << std::endl;
  cam_data << '0' << std::endl;
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  system("modprobe uio_pruss");
 
  unsigned int ret;
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_init ();		

  if( 0 != prussdrv_open(PRU_EVTOUT_0) )
    throw std::runtime_error( "Failed to open pru0" );
  if( 0 != prussdrv_open(PRU_EVTOUT_1) )
    throw std::runtime_error( "Failed to open pru1" );

  prussdrv_pruintc_init(&pruss_intc_initdata);

  volatile uint32_t *pru0_data, *pru1_data, *pru_shared;
  
  prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, (void**)&pru0_data);
  prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, (void**)&pru1_data);
  prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void**)&pru_shared);
  
  memset( (void*)pru0_data, 0, 8*1024);
  memset( (void*)pru1_data, 0, 8*1024);
  memset( (void*)pru_shared, 0, 12*1024);

  volatile uint16_t *pru0_ddr, *pru1_ddr;	

  int mem_fd = open("/dev/mem", O_RDWR);
  if (mem_fd < 0)
    throw std::runtime_error( "Failed to open /dev/mem" );

  pru0_ddr= (uint16_t*)mmap(0, PRU_DDR_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, PRU0_DDR);
  if (pru0_ddr == NULL)
    throw std::runtime_error( "Failed to map pru0 ddr" );
    
  pru1_ddr= (uint16_t*)mmap(0, PRU_DDR_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, PRU1_DDR);
  if (pru1_ddr == NULL)
    throw std::runtime_error( "Failed to map pru1 ddr" );
    
    
  memset( (void*)pru0_ddr, 0, PRU_DDR_SIZE);
  memset( (void*)pru1_ddr, 0, PRU_DDR_SIZE);
    
    
    
    
    
    
  
  
int i=0, n=0, readcnt=1;
  
  // initialization: open port, ioctl address, send 0x40/0x00 init to nunchuck:
  int fd_accel_303 = open(PORT, O_RDWR);
  if(fd_accel_303<0) error("cant open %s - %m", PORT);
  if(ioctl(fd_accel_303, I2C_SLAVE, ADDR_ACCEL_303) < 0) error("cant ioctl %s:0x%02x - %m", PORT, ADDR_ACCEL_303);
  
  int fd_mag_303 = open(PORT, O_RDWR);
  if(fd_mag_303<0) error("cant open %s - %m", PORT);
  if(ioctl(fd_mag_303, I2C_SLAVE, ADDR_MAG_303) < 0) error("cant ioctl %s:0x%02x - %m", PORT, ADDR_MAG_303);
  
  int fd_accel_330 = open(PORT, O_RDWR);
  if(fd_accel_330<0) error("cant open %s - %m", PORT);
  if(ioctl(fd_accel_330, I2C_SLAVE, ADDR_ACCEL_330) < 0) error("cant ioctl %s:0x%02x - %m", PORT, ADDR_ACCEL_330);
  
  int fd_gyro_330 = open(PORT, O_RDWR);
  if(fd_gyro_330<0) error("cant open %s - %m", PORT);
  if(ioctl(fd_gyro_330, I2C_SLAVE, ADDR_GYRO_330) < 0) error("cant ioctl %s:0x%02x - %m", PORT, ADDR_GYRO_330);
  
  int fd_cam = open(PORT, O_RDWR);
  if(fd_cam<0) error("cant open %s - %m", PORT);
  if(ioctl(fd_cam, I2C_SLAVE, ADDR_CAM) < 0) error("cant ioctl %s:0x%02x - %m", PORT, ADDR_CAM);
  
// camera

  // at power up, start the I2C commands, the camera must be powered with the reset low, then put the reset high and then turn on the clock
  
  // write camera register

  //Address Data Comments
  //0x0B 0x00 White Line OFF
  //0x58 0x20 Exposure Time
  //0x05 0x00 Frame Rate Quarter
  //0x1A 0xFF
  //0x1B 0xB3
  //0x11 0x4A
  //0x14 0x33
  //0x04 0x0D RGB, 352x288, OUT ON
  //0x1F 0x0B
  //0x1E 0xC3
  //0x0E 0x1E

   //I2CWrite(0x02,0x00); //Set Camera Active
   //I2CWrite(0x03,0xCF); //PLL On (last nibble correct?)
   //I2CWrite(0x04,0x50); //JPEG On, DOUT On, PICMODE:1280x1024
   //I2CWrite(0x05,0x80); //0x80 = max framerate, 0x00 = 1/4 framerate
   //I2CWrite(0xE6,0x08); //0x08 = 4 byte output units (was 0x00)
   //I2CWrite(0x0E,0xB0); //Changed b/c of PICSIZ (was 0xAC)
   //I2CWrite(0x11,0x6A); //Changed b/c of PICSIZ (was 0x02)
   //I2CWrite(0x14,0x33); //Changed b/c of PICSIZ (same as default)
   //// Need to change SPCOUNT (on addresses 0x1F and 0x1E) ?

  if(write(fd_cam, "\x03\x02", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_CAM);
  

  ////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////

  pru0_data[EXIT_CTRL] = 0;
  pru1_data[EXIT_CTRL] = 0;
  
  pru1_data[CAM_SWAP_BUFF] = 0;
  pru1_data[CAM_BUFF_0] = PRU1_DDR;
  pru1_data[CAM_BUFF_1] = PRU1_DDR+640*480*2;
  int frameCount = 0;
  
  
  
 
  
  
  
  
  
  
  // 303 accelerometer
  if(write(fd_accel_303, "\x20\x97", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_ACCEL_303);

  
  // 303 magnetometer
  if(write(fd_mag_303, "\x00\x9C", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_MAG_303);
  if(write(fd_mag_303, "\x02\x00", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_MAG_303);

   // 330 accelerometer
  if(write(fd_accel_330, "\x20\x97", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_ACCEL_330);

  // 330 gyroscope
  if(write(fd_gyro_330, "\x20\xCF", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_GYRO_330);
  
unsigned char buf[6];

  while(frameCount < 1000)
  {
  
  


  signed short int x_accel_303, y_accel_303, z_accel_303;
  signed short int x_mag_303, y_mag_303, z_mag_303;
  signed short int x_accel_330, y_accel_330, z_accel_330;
  signed short int x_gyro_303, y_gyro_303, z_gyro_303;
  
++frameCount;
    if(write(fd_accel_303, "\xA8", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_ACCEL_303);

    
    n = read(fd_accel_303, buf, 6); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_ACCEL_303);

    x_accel_303 = buf[0] | (buf[1]<<8);
     y_accel_303 = buf[2] | (buf[3]<<8);
      z_accel_303 = buf[4] | (buf[5]<<8);
      


    if(write(fd_mag_303, "\x31", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_MAG_303);

    n = read(fd_mag_303, buf, 2); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_MAG_303);
    //std::cout << std::setw(4)<<(int)buf[0] << " ";
    //std::cout << std::setw(4)<<(int)buf[1] << " ";

    if(write(fd_mag_303, "\x03", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_MAG_303);

    n = read(fd_mag_303, buf, 6); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_MAG_303);

   
    x_mag_303 = buf[1] | (buf[0]<<8);
     y_mag_303 = buf[3] | (buf[2]<<8);
      z_mag_303 = buf[5] | (buf[4]<<8);

      

    if(write(fd_accel_330, "\xA8", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_ACCEL_330);

  
    n = read(fd_accel_330, buf, 6); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_ACCEL_330);

   
   x_accel_330 = buf[0] | (buf[1]<<8);
     y_accel_330 = buf[2] | (buf[3]<<8);
      z_accel_330 = buf[4] | (buf[5]<<8);
 
 
    if(write(fd_gyro_330, "\x26", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_GYRO_330);

  
    n = read(fd_gyro_330, buf, 1); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_GYRO_330);
    //std::cout << std::setw(4)<<(int)buf[0] << " ";

    if(write(fd_gyro_330, "\xA8", 1)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_GYRO_330);

    n = read(fd_gyro_330, buf, 6); // read one byte (at index i)
    if(n<0) error("read error %s:0x%02x - %m", PORT, ADDR_GYRO_330);

    x_gyro_303 = buf[0] | (buf[1]<<8);
     y_gyro_303 = buf[2] | (buf[3]<<8);
      z_gyro_303 = buf[4] | (buf[5]<<8);

      
      std::cout<<"\033[2J"<< "\033[" << 0 << ";" << 0 << ";H";

      std::cout<< "Accel: x = " << std::setw(6) << x_accel_303 << " y = " <<std::setw(6)<<y_accel_303 << " z = " << std::setw(6) <<z_accel_303  << std::endl<<std::endl;
      std::cout<< "  Mag: x = " << std::setw(6) << x_mag_303 << " y = " << std::setw(6) <<y_mag_303 << " z = " <<std::setw(6) << z_mag_303 << std::endl<<std::endl;
      std::cout<< "Accel: x = " << std::setw(6) << x_accel_330 << " y = " << std::setw(6) <<y_accel_330 << " z = " <<std::setw(6) << z_accel_330 << std::endl<<std::endl;
      std::cout<< " Gyro: x = " << std::setw(6) << x_gyro_303 << " y = " << std::setw(6) <<y_gyro_303 << " z = " << std::setw(6) <<z_gyro_303 << std::endl <<std::endl;
    

usleep(10000);

  }
  
  
  
  
  
  
 /* 
   prussdrv_exec_program (PRU0, "./pru0.bin");
  prussdrv_exec_program (PRU1, "./pru1.bin");
  
  
  std::cout <<"Get ready, it's photo time!"<<std::endl;
  sleep(2);
  frameCount = 0;
   pru1_data[CAM_SWAP_BUFF] = 1;
    while( 1 == pru1_data[CAM_SWAP_BUFF] ){}
    ++frameCount;
    
    volatile uint16_t *buffer;
    if( frameCount & 0x1 )
    {
      cam_led << '0' << std::endl;
      buffer = pru1_ddr;
    }
    else
    {
      cam_led << '1' << std::endl;
      buffer = pru1_ddr+640*480;
    }
      
    std::stringstream file;
    file << frameCount << ".bmp";
    
    //for(int i = 0; i <640*3;++i)
    //std::cout<<buffer[i]<<' ';
    cv::Mat rgb565(480, 640, CV_8UC2, (void*)buffer);
    cv::Mat rgb888;
    cv::cvtColor(rgb565, rgb888, cv::COLOR_BGR5652BGR);
    
    cv::imwrite( file.str(), rgb888 );
  
  
  pru0_data[EXIT_CTRL] = 1;
  pru1_data[EXIT_CTRL] = 1;
   if(write(fd_cam, "\x1F\x40", 2)<0) error("cant setup %s:0x%02x - %m", PORT, ADDR_CAM);
   
   prussdrv_pru_wait_event (PRU_EVTOUT_0);
  prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);
  prussdrv_pru_wait_event (PRU_EVTOUT_1);
  prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);
  
*/
 
  std::cout << "Exit..." << std::endl;
  
 
  
  
  cam_reset.close();
  cam_led.close();
  cam_data.close();
  
  

  prussdrv_pru_disable (PRU0);
  prussdrv_pru_disable (PRU1);
  prussdrv_exit ();
  
  if( 0 != system( "echo 88 > /sys/class/gpio/unexport" ) )
    throw std::runtime_error( "Failed to unexport cam reset gpio" );
  if( 0 != system( "echo 61 > /sys/class/gpio/unexport" ) )
    throw std::runtime_error( "Failed to unexport cam led gpio" );
  if( 0 != system( "echo 69 > /sys/class/gpio/unexport" ) )
    throw std::runtime_error( "Failed to unexport cam data gate gpio" );
    
    
  
    
    
}
