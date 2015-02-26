// Module for interfacing with 0.96 OLED SCREEN with I2C interface
/* Usage Notes
  Works only with i2c 0.96" OLED Displays with Device Address 0x3C - Hardcoded Device ID
  Does Default Setup Automatically
  oled.setup(sda,scl) => Sets up the Screen uses default 0x3C i2c device address for 0.96" Cheap OLED Modules
  oled.clear() => To Clear Screen
  oled.line(line number, font size, "text to be printed" ) // Font Size can be 1 or 2
  oled.print(x,y, font size, "text to be printed" ) // Font Size can be 1 or 2
  oled.invert(val) // val can be 1 or 0

*/
//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "driver/i2c_oled_fonts.h"

#define MAX_SMALL_FONTS 21 //SMALL FONTS
#define MAX_BIG_FONTS 16 //BIG FONTS

#define NORMALDISPLAY 0xA6
#define INVERTDISPLAY 0xA7

void ICACHE_FLASH_ATTR OLED_writeReg(uint8_t reg_addr,unsigned char val)
{
  unsigned id = 0;
  unsigned address = 0x3C;
  
  platform_i2c_send_start(id);
  platform_i2c_send_address( id, (u16)address, 0); // PLATFORM_I2C_DIRECTION_TRANSMITTER
  platform_i2c_send_byte(id,reg_addr);
  platform_i2c_send_byte(id,val);
  platform_i2c_send_stop(id);
}

void ICACHE_FLASH_ATTR OLED_writeCmd(unsigned char I2C_Command)
{
  OLED_writeReg(0x00,I2C_Command);
}

void ICACHE_FLASH_ATTR OLED_writeDat(unsigned char Data)
{
  OLED_writeReg(0x40,Data);
}

void ICACHE_FLASH_ATTR OLED_Fill(unsigned char fill_Data)
{
  unsigned char m,n;
  for(m=0;m<8;m++)
  {
    OLED_writeCmd(0xb0+m);    //page0-page1
    OLED_writeCmd(0x00);    //low column start address
    OLED_writeCmd(0x10);    //high column start address
    for(n=0;n<132;n++)
      {
        OLED_writeDat(fill_Data);
      }
  }
}

void ICACHE_FLASH_ATTR OLED_SetPos(unsigned char x, unsigned char y)
{   
  OLED_writeCmd(0xb0+y);
  OLED_writeCmd(((x&0xf0)>>4)|0x10);
  OLED_writeCmd((x&0x0f)|0x01);
}

void ICACHE_FLASH_ATTR OLED_CLS()
{
  OLED_Fill(0x00); 
}

void ICACHE_FLASH_ATTR OLED_ON()
{
  OLED_writeCmd(0X8D);
  OLED_writeCmd(0X14);
  OLED_writeCmd(0XAF);
}

void ICACHE_FLASH_ATTR OLED_OFF()
{
  OLED_writeCmd(0X8D);
  OLED_writeCmd(0X10);
  OLED_writeCmd(0XAE);
}
// Lua: speed = i2c.setup(sda, scl)
static int oled_setup( lua_State *L )
{
  unsigned id = 0;
  unsigned sda = luaL_checkinteger( L, 1);
  unsigned scl = luaL_checkinteger( L, 2 );
  unsigned address = 0x3C;

  MOD_CHECK_ID( i2c, id );
  MOD_CHECK_ID( gpio, sda );
  MOD_CHECK_ID( gpio, scl );

  if(scl==0 || sda==0)
    return luaL_error( L, "no i2c for D0" );
  
  lua_pushinteger( L, platform_i2c_setup( id, sda, scl, PLATFORM_I2C_SPEED_SLOW ) );

  // Setup OLED Registers
  OLED_writeCmd(0xae); // turn off oled panel
  OLED_writeCmd(0x00); // set low column address 
  OLED_writeCmd(0x10); // set high column address 
  OLED_writeCmd(0x40); // set start line address 
  OLED_writeCmd(0x81); // set contrast control register 
  
  OLED_writeCmd(0xa0);
  OLED_writeCmd(0xc0);
  
  OLED_writeCmd(0xa6); // set normal display 
  OLED_writeCmd(0xa8); // set multiplex ratio(1 to 64) 
  OLED_writeCmd(0x3f); // 1/64 duty 
  OLED_writeCmd(0xd3); // set display offset 
  OLED_writeCmd(0x00); // not offset 
  OLED_writeCmd(0xd5); // set display clock divide ratio/oscillator frequency 
  OLED_writeCmd(0x80); // set divide ratio 
  OLED_writeCmd(0xd9); // set pre-charge period 
  OLED_writeCmd(0xf1); 
  OLED_writeCmd(0xda); // set com pins hardware configuration 
  OLED_writeCmd(0x12); 
  OLED_writeCmd(0xdb); // set vcomh 
  OLED_writeCmd(0x40); 
  OLED_writeCmd(0x8d); // set Charge Pump enable/disable 
  OLED_writeCmd(0x14); // set(0x10) disable
  OLED_writeCmd(0xaf); // turn on oled panel  
    
  OLED_Fill(0x00);  //OLED CLS

 return 1;
}

void ICACHE_FLASH_ATTR OLED_ShowStr( unsigned char x, unsigned char y, const char *ch, unsigned char TextSize )
{
  unsigned char c = 0,i = 0,j = 0;
  switch(TextSize)
  {
    case 1:
    {
      while(ch[j] != '\0')
      {
        c = ch[j] - 32;
        if(x > 126)
        {
          x = 0;
          y++;
        }
        OLED_SetPos(x,y);
        for(i=0;i<6;i++)
          OLED_writeDat(F6x8[c][i]);
        x += 6;
        j++;
      }
    }break;
    case 2:
    {
      while(ch[j] != '\0')
      {
        c = ch[j] - 32;
        if(x > 120)
        {
          x = 0;
          y++;
        }
        OLED_SetPos(x,y);
        for(i=0;i<8;i++)
          OLED_writeDat(F8X16[c*16+i]);
        OLED_SetPos(x,y+1);
        for(i=0;i<8;i++)
          OLED_writeDat(F8X16[c*16+i+8]);
        x += 8;
        j++;
      }
    }break; 
  }
}
// oled.showstring(x,y,1,"hello")
void ICACHE_FLASH_ATTR OLED_Print(lua_State *L)
{
  unsigned char x = luaL_checkinteger( L, 1);
  unsigned char y = luaL_checkinteger( L, 2);
  unsigned char TextSize = luaL_checkinteger( L, 3);
  const char *ch = luaL_checkstring(L, 4);

  uint8_t step;
  if (TextSize==1)
    step = x*6; 
  else if (TextSize==2)
    step = x*8; 
  else 
    step = x;
  OLED_ShowStr(step,y,ch,TextSize);
}

void ICACHE_FLASH_ATTR OLED_Line(lua_State *L)
{
  
  unsigned char y = luaL_checkinteger( L, 1);
  unsigned char TextSize = luaL_checkinteger( L, 2);
  const char *ch = luaL_checkstring(L, 3);

  uint8 step;
  uint8 len = strlen(ch);
  uint8 maxf = 0;
  
  y--;
  
  if (TextSize==1) { 
    maxf = MAX_SMALL_FONTS;
  }
  else if (TextSize==2) {
    y *= 2;
    maxf = MAX_BIG_FONTS;
  }

  if ((TextSize>=1) && (TextSize<=2)) {
    os_memset(ch+len+1,' ',maxf-len);
    os_memset(ch+maxf,'\0',1);

    OLED_ShowStr(0,y,ch,TextSize); 
  }

}

void ICACHE_FLASH_ATTR OLED_Invert(lua_State *L)
{
  unsigned char state = luaL_checkinteger( L, 1);
  if(state==1)
  OLED_writeCmd(INVERTDISPLAY);
  else
  OLED_writeCmd(NORMALDISPLAY);
}


// Module function map
#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE oled_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( oled_setup ) },
  { LSTRKEY( "clear" ),  LFUNCVAL( OLED_CLS ) },
 // { LSTRKEY( "on" ),  LFUNCVAL( OLED_ON ) },
 // { LSTRKEY( "off" ),  LFUNCVAL( OLED_OFF ) },
  { LSTRKEY( "print" ),  LFUNCVAL( OLED_Print ) },
  { LSTRKEY( "line" ),  LFUNCVAL( OLED_Line ) },
  { LSTRKEY( "invert" ),  LFUNCVAL( OLED_Invert ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "SLOW" ), LNUMVAL( PLATFORM_I2C_SPEED_SLOW ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_oled( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_OLED, oled_map );
  
  // Add the stop bits and parity constants (for i2c.setup)
  // MOD_REG_NUMBER( L, "FAST", PLATFORM_I2C_SPEED_FAST );
  MOD_REG_NUMBER( L, "SLOW", PLATFORM_I2C_SPEED_SLOW );   
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

