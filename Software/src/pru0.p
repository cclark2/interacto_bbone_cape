.setcallreg r29.w0
.origin 0
.entrypoint MEMACCESSPRUDATARAM


#include "pru_common.hp"
#include "mem_map.h"

 // on gpio1
  #define LCD_CS (1<<6)
  #define LCD_RS (1<<7)
  #define LCD_WR (1<<2)
  #define LCD_RD (1<<28)
  
  // on pru
  #define LCD_DB8 (1<<0)
  #define LCD_DB9 (1<<1)
  #define LCD_DB10 (1<<2)
  #define LCD_DB11 (1<<3)
  #define LCD_DB12 (1<<5)
  #define LCD_DB13 (1<<7)
  #define LCD_DB14 (1<<14)
  #define LCD_DB15 (1<<15)
  
  
#define GPIO1 0x4804c000
#define GPIO2 0x481ac000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

#define exit r6
#define currBuff r7
#define swapBuff r10
#define buff0 r11
#define buff1 r12
#define pixelByteCount   r13
#define numPixelBytes r8

#define pin_set r20
#define pin_clear r21
#define pin  r22

#define lcd_bits r30


.macro  CS_SET
    MOV pin, LCD_CS
    SBBO pin, pin_set, 0, 4
.endm

.macro  CS_CLEAR
    MOV pin, LCD_CS
    SBBO pin, pin_clear, 0, 4
.endm

.macro  RS_SET
    MOV pin, LCD_RS
    SBBO pin, pin_set, 0, 4
.endm

.macro  RS_CLEAR
    MOV pin, LCD_RS
    SBBO pin, pin_clear, 0, 4
.endm

.macro  WR_SET
    MOV pin, LCD_WR
    SBBO pin, pin_set, 0, 4
.endm

.macro  WR_CLEAR
    MOV pin, LCD_WR
    SBBO pin, pin_clear, 0, 4
.endm

.macro  RD_SET
    MOV pin, LCD_RD
    SBBO pin, pin_set, 0, 4
.endm

.macro  RD_CLEAR
    MOV pin, LCD_RD
    SBBO pin, pin_clear, 0, 4
.endm




.macro DOUT
.mparam data
    MOV lcd_bits, 0
    MOV r0, data
    AND r0, r0, 0xF
    OR  lcd_bits, lcd_bits, r0
    
    MOV r0, data
    AND r0, r0, 0x10
    LSL r0, r0, 1
    OR  lcd_bits, lcd_bits, r0
    
    MOV r0, data
    AND r0, r0, 0x20
    LSL r0, r0, 2
    OR  lcd_bits, lcd_bits, r0
    
    MOV r0, data
    AND r0, r0, 0xC0
    LSL r0, r0, 8
    OR  lcd_bits, lcd_bits, r0
.endm


.macro DATA_WRITE
.mparam data
    DOUT data
    WR_CLEAR
    CALL Delay
    WR_SET
.endm

.macro CMD_WRITE
.mparam cmd
    RS_CLEAR
    DATA_WRITE cmd
    RS_SET
    CALL Delay
.endm

.macro  nop
    MOV r0,r0
.endm


MEMACCESSPRUDATARAM:

MOV exit,EXIT_CTRL*4

// Enable OCP master port (default direct connect output
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4


    MOV       swapBuff, LCD_SWAP_BUFF*4
    MOV       buff0, LCD_BUFF_0*4
    LBBO      buff0, buff0, 0, 4
    MOV       buff1, LCD_BUFF_1*4
    LBBO      buff1, buff1, 0, 4
    
    MOV       currBuff, buff0
    MOV       numPixelBytes, 320*240*2


    MOV pin_set, GPIO1 | GPIO_SETDATAOUT
    MOV pin_clear, GPIO1 | GPIO_CLEARDATAOUT

    
    
  
    // Send notification to Host for program completion
    MOV       r31.b0, PRU0_ARM_INTERRUPT+16

    // Halt the processor
    HALT
    
    
    //////////////////////////////////////////////////
    CS_CLEAR
    CALL Delay
    
    CMD_WRITE 0x28
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x11
    DATA_WRITE 0x00
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xC0
    DATA_WRITE 0x26
    DATA_WRITE 0x04
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xC1
    DATA_WRITE 0x04
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xC5
    DATA_WRITE 0x34
    DATA_WRITE 0x50
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x36
    DATA_WRITE 0x88
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xB1
    DATA_WRITE 0x00
    DATA_WRITE 0x18
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xB6
    DATA_WRITE 0x0A
    DATA_WRITE 0xA2
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xC7
    DATA_WRITE 0xC0
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x3A
    DATA_WRITE 0x55
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xE0
    DATA_WRITE 0x1F
    DATA_WRITE 0x1B
    DATA_WRITE 0x18
    DATA_WRITE 0x0B
    DATA_WRITE 0x0F
    DATA_WRITE 0x09
    DATA_WRITE 0x46
    DATA_WRITE 0xB5
    DATA_WRITE 0x37
    DATA_WRITE 0x0A
    DATA_WRITE 0x0C
    DATA_WRITE 0x07
    DATA_WRITE 0x07
    DATA_WRITE 0x05
    DATA_WRITE 0x00
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0xE1
    DATA_WRITE 0x00
    DATA_WRITE 0x24
    DATA_WRITE 0x27
    DATA_WRITE 0x04
    DATA_WRITE 0x10
    DATA_WRITE 0x06
    DATA_WRITE 0x39
    DATA_WRITE 0x74
    DATA_WRITE 0x48
    DATA_WRITE 0x05
    DATA_WRITE 0x13
    DATA_WRITE 0x38
    DATA_WRITE 0x38
    DATA_WRITE 0x3A
    DATA_WRITE 0x1F
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x2A
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0xEF
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x2B
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0x01
    DATA_WRITE 0x3F
    
    
    //////////////////////////////////////////////////
    
    CMD_WRITE 0x29
    
    

    #ifdef kkk
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x10
    CS_SET
    CALL Delay
    
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x11
    CS_SET
    CALL Delay
    
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x13
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x28
    CS_SET
CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xC0
    DATA_WRITE 0x26
    DATA_WRITE 0x04
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xC1
    DATA_WRITE 0x04
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xC5
    DATA_WRITE 0x34
    DATA_WRITE 0x40
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x36
    DATA_WRITE 0x88
    CS_SET
  CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xB1
    DATA_WRITE 0x00
    DATA_WRITE 0x18
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xB6
    DATA_WRITE 0x0A
    DATA_WRITE 0xE2
    CS_SET
 CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xC7
    DATA_WRITE 0xC0
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x3A
    DATA_WRITE 0x55
    CS_SET
	CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xE0
    DATA_WRITE 0x1F
    DATA_WRITE 0x1B
    DATA_WRITE 0x18
    DATA_WRITE 0x0B
    DATA_WRITE 0x0F
    DATA_WRITE 0x09
    DATA_WRITE 0x46
    DATA_WRITE 0xB5
    DATA_WRITE 0x37
    DATA_WRITE 0x0A
    DATA_WRITE 0x0C
    DATA_WRITE 0x07
    DATA_WRITE 0x07
    DATA_WRITE 0x05
    DATA_WRITE 0x00
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0xE1
    DATA_WRITE 0x00
    DATA_WRITE 0x24
    DATA_WRITE 0x27
    DATA_WRITE 0x04
    DATA_WRITE 0x10
    DATA_WRITE 0x06
    DATA_WRITE 0x39
    DATA_WRITE 0x74
    DATA_WRITE 0x48
    DATA_WRITE 0x05
    DATA_WRITE 0x13
    DATA_WRITE 0x38
    DATA_WRITE 0x38
    DATA_WRITE 0x3A
    DATA_WRITE 0x1F
    CS_SET
    CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x2A
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0xEF
    CS_SET
	CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x2B
    DATA_WRITE 0x00
    DATA_WRITE 0x00
    DATA_WRITE 0x01
    DATA_WRITE 0x3F
    CS_SET
CALL Delay
    //////////////////////////////////////////////////
    CS_CLEAR
    CMD_WRITE 0x29
    CS_SET
CALL Delay

    DOUT 0xAA
    #endif
    
    
    
    
    
    
    
    start:
    MOV r1, 10
BLINK:
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY:
    SUB r0, r0, 1
    QBNE DELAY, r0, 0
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY2:
    SUB r0, r0, 1
    QBNE DELAY2, r0, 0
    SUB r1, r1, 1
    QBNE BLINK, r1, 0
    
qba start
    
    
    
    
  Loop:
  
  
  
  
  
  
  
  
   
      MOV   pixelByteCount, 0
    
 
      SBBO  r0.w0, currBuff, pixelByteCount, 2
      ADD   pixelByteCount, pixelByteCount, 2

      Pixels:
        
        
        
        
 #ifdef sdfsdfd       
        /* check given values */
	if( u16AddrX >= 240 )
	{
		u16AddrX = 239;
	}
	if( u16AddrY >= 320 )
	{
		u16AddrY = 319;
	}
	/* calculate address */
	u32Address = 256 * (U32)u16AddrY + u16AddrX;
	
	/* setup column and page address for this write */
	au8Data[0] = 0x00;
	au8Data[1] = (U8)u32Address;
	au8Data[2] = 0x00;
	au8Data[3] = 0xEF;
	ILI9340WriteRegister( 0x2A, au8Data, 4 );
	au8Data[0] = (U8)(u32Address >> 16);
	au8Data[1] = (U8)(u32Address >> 8);
	au8Data[2] = 0x01;
	au8Data[3] = 0x3F;
	ILI9340WriteRegister( 0x2B, au8Data, 4 );

	/* set chip select */
	ILI9340_NCS( ILI9340_LOW );
	
	/* set up writing register in index */
	ILI9340_RS( ILI9340_LOW );
	ILI9340_DATA = 0x2C;
	ILI9340_NWR( ILI9340_LOW );
	ILI9340_NWR( ILI9340_HIGH );
	
	/* set correct signalling for consecutive writing */
	ILI9340_RS( ILI9340_HIGH );
	ILI9340_NRD( ILI9340_HIGH );

	/* write the complete ILI9340 */
	for( u32ii = 0; u32ii < u32Length; u32ii++ )
	{
		ILI9340_DATA = *pu8Data++;
		ILI9340_NWR( ILI9340_LOW );
		ILI9340_NWR( ILI9340_HIGH );
		ILI9340_DATA = *pu8Data++;
		ILI9340_NWR( ILI9340_LOW );
		ILI9340_NWR( ILI9340_HIGH );
		
		if( u8IncrAddr == FALSE )
		{
			/* set to original pointer for next pixel */
			pu8Data -= 2;
		}
	}
	
	/* clear chip select */
	ILI9340_NCS( ILI9340_HIGH );
  #endif
  
  

        SBBO  r0.w0, currBuff, pixelByteCount, 2
        ADD   pixelByteCount, pixelByteCount, 2
        QBGE  Pixels, pixelByteCount, numPixelBytes
    
      ADD   currBuff, currBuff, pixelByteCount
    
    MOV r2, 1<<22
    //MOV r3, GPIO1 | GPIO_DATAOUT
    LBBO r1, r3, 0, 4
    XOR  r1, r1, r2
    SBBO r1, r3, 0, 4
    
    LBBO r0, swapBuff, 0, 4
    QBEQ Loop, r0, 0
    
    MOV   r0, 0
    SBBO  r0, swapBuff, 0, 4
    MOV   r0, buff0
    MOV   buff0, buff1
    MOV   buff1, r0
    MOV   currBuff, buff0

    
    
    
    
    
    
    
    
    LBBO r0, exit, 0, 4
    QBEQ Quit, r0, 1
    
    QBA Loop
    
    
    

    
    
    Quit:
    // Send notification to Host for program completion
    MOV       r31.b0, PRU0_ARM_INTERRUPT+16

    // Halt the processor
    HALT
    
    
    
    
    
    
    
    
    
    Delay:
  MOV r0, 0x000000F0
  n:
    SUB r0, r0, 1
    QBNE n, r0, 0
  RET
    
    
#ifdef junk
start:
    MOV r1, 10
BLINK:
    MOV r2, LCD_CS|LCD_RS|LCD_WR|LCD_RD
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
    MOV r30, 0
    MOV r0, 0x00100000
DELAY:
    SUB r0, r0, 1
    QBNE DELAY, r0, 0
    MOV r2, LCD_CS|LCD_RS|LCD_WR|LCD_RD
    MOV r3, GPIO1 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
    MOV r30, LCD_DB8|LCD_DB9|LCD_DB10|LCD_DB11|LCD_DB12|LCD_DB13|LCD_DB14|LCD_DB15
    MOV r0, 0x00100000

DELAY2:
    SUB r0, r0, 1
    QBNE DELAY2, r0, 0
    SUB r1, r1, 1
    QBNE BLINK, r1, 0
    
    LBBO r0, exit, 0, 4
    QBEQ Quit, r0, 1
    
qba start
   #endif
   
   
   