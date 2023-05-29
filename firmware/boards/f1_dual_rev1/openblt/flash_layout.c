/** \brief   Array wit the layout of the flash memory.
 *  \details Also controls what part of the flash memory is reserved for the bootloader.
 *           If the bootloader size changes, the reserved sectors for the bootloader
 *           might need adjustment to make sure the bootloader doesn't get overwritten.
 */

/* STM32F103RCT6 - high-density device with 256 Kb of flash = 128 pages of 2Kb */
static const tFlashSector flashLayout[] =
{
  /* space is reserved for a bootloader configuration with all supported communication
   * interfaces enabled. when for example only UART is needed, than the space required
   * for the bootloader can be made a lot smaller here.
   */
  /* { 0x08000000, 0x00800 },              flash sector  0 - reserved for bootloader    */
  /* { 0x08000800, 0x00800 },              flash sector  1 - reserved for bootloader    */
  /* { 0x08001000, 0x00800 },              flash sector  2 - reserved for bootloader    */
  /* { 0x08001800, 0x00800 },              flash sector  3 - reserved for bootloader    */
  { 0x08002000, 0x00800 },              /* flash sector  4 - 2kb                        */
  { 0x08002800, 0x00800 },              /* flash sector  5 - 2kb                        */
  { 0x08003000, 0x00800 },              /* flash sector  6 - 2kb                        */
  { 0x08003800, 0x00800 },              /* flash sector  7 - 2kb                        */
#if (BOOT_NVM_SIZE_KB > 16)
  { 0x08004000, 0x00800 },              /* flash sector  8 - 2kb                        */
  { 0x08004800, 0x00800 },              /* flash sector  9 - 2kb                        */
  { 0x08005000, 0x00800 },              /* flash sector 10 - 2kb                        */
  { 0x08005800, 0x00800 },              /* flash sector 11 - 2kb                        */
  { 0x08006000, 0x00800 },              /* flash sector 12 - 2kb                        */
  { 0x08006800, 0x00800 },              /* flash sector 13 - 2kb                        */
  { 0x08007000, 0x00800 },              /* flash sector 14 - 2kb                        */
  { 0x08007800, 0x00800 },              /* flash sector 15 - 2kb                        */
#endif
#if (BOOT_NVM_SIZE_KB > 32)
  { 0x08008000, 0x00800 },              /* flash sector 16 - 2kb                        */
  { 0x08008800, 0x00800 },              /* flash sector 17 - 2kb                        */
  { 0x08009000, 0x00800 },              /* flash sector 18 - 2kb                        */
  { 0x08009800, 0x00800 },              /* flash sector 19 - 2kb                        */
  { 0x0800a000, 0x00800 },              /* flash sector 20 - 2kb                        */
  { 0x0800a800, 0x00800 },              /* flash sector 21 - 2kb                        */
  { 0x0800b000, 0x00800 },              /* flash sector 22 - 2kb                        */
  { 0x0800b800, 0x00800 },              /* flash sector 23 - 2kb                        */
  { 0x0800c000, 0x00800 },              /* flash sector 24 - 2kb                        */
  { 0x0800c800, 0x00800 },              /* flash sector 25 - 2kb                        */
  { 0x0800d000, 0x00800 },              /* flash sector 26 - 2kb                        */
  { 0x0800d800, 0x00800 },              /* flash sector 27 - 2kb                        */
  { 0x0800e000, 0x00800 },              /* flash sector 28 - 2kb                        */
  { 0x0800e800, 0x00800 },              /* flash sector 29 - 2kb                        */
  { 0x0800f000, 0x00800 },              /* flash sector 30 - 2kb                        */
  { 0x0800f800, 0x00800 },              /* flash sector 31 - 2kb                        */
#endif
#if (BOOT_NVM_SIZE_KB > 64)
  { 0x08010000, 0x00800 },              /* flash sector 32 - 2kb                        */
  { 0x08010800, 0x00800 },              /* flash sector 33 - 2kb                        */
  { 0x08011000, 0x00800 },              /* flash sector 34 - 2kb                        */
  { 0x08011800, 0x00800 },              /* flash sector 35 - 2kb                        */
  { 0x08012000, 0x00800 },              /* flash sector 36 - 2kb                        */
  { 0x08012800, 0x00800 },              /* flash sector 37 - 2kb                        */
  { 0x08013000, 0x00800 },              /* flash sector 38 - 2kb                        */
  { 0x08013800, 0x00800 },              /* flash sector 39 - 2kb                        */
  { 0x08014000, 0x00800 },              /* flash sector 40 - 2kb                        */
  { 0x08014800, 0x00800 },              /* flash sector 41 - 2kb                        */
  { 0x08015000, 0x00800 },              /* flash sector 42 - 2kb                        */
  { 0x08015800, 0x00800 },              /* flash sector 43 - 2kb                        */
  { 0x08016000, 0x00800 },              /* flash sector 44 - 2kb                        */
  { 0x08016800, 0x00800 },              /* flash sector 45 - 2kb                        */
  { 0x08017000, 0x00800 },              /* flash sector 46 - 2kb                        */
  { 0x08017800, 0x00800 },              /* flash sector 47 - 2kb                        */
  { 0x08018000, 0x00800 },              /* flash sector 48 - 2kb                        */
  { 0x08018800, 0x00800 },              /* flash sector 49 - 2kb                        */
  { 0x08019000, 0x00800 },              /* flash sector 50 - 2kb                        */
  { 0x08019800, 0x00800 },              /* flash sector 51 - 2kb                        */
  { 0x0801a000, 0x00800 },              /* flash sector 52 - 2kb                        */
  { 0x0801a800, 0x00800 },              /* flash sector 53 - 2kb                        */
  { 0x0801b000, 0x00800 },              /* flash sector 54 - 2kb                        */
  { 0x0801b800, 0x00800 },              /* flash sector 55 - 2kb                        */
  { 0x0801c000, 0x00800 },              /* flash sector 56 - 2kb                        */
  { 0x0801c800, 0x00800 },              /* flash sector 57 - 2kb                        */
  { 0x0801d000, 0x00800 },              /* flash sector 58 - 2kb                        */
  { 0x0801d800, 0x00800 },              /* flash sector 59 - 2kb                        */
  { 0x0801e000, 0x00800 },              /* flash sector 60 - 2kb                        */
  { 0x0801e800, 0x00800 },              /* flash sector 61 - 2kb                        */
  { 0x0801f000, 0x00800 },              /* flash sector 62 - 2kb                        */
  { 0x0801f800, 0x00800 },              /* flash sector 63 - 2kb                        */
#endif
#if (BOOT_NVM_SIZE_KB > 128)
  { 0x08020000, 0x00800 },              /* flash sector 64 - 2kb                        */
  { 0x08020800, 0x00800 },              /* flash sector 65 - 2kb                        */
  { 0x08021000, 0x00800 },              /* flash sector 66 - 2kb                        */
  { 0x08021800, 0x00800 },              /* flash sector 67 - 2kb                        */
  { 0x08022000, 0x00800 },              /* flash sector 68 - 2kb                        */
  { 0x08022800, 0x00800 },              /* flash sector 69 - 2kb                        */
  { 0x08023000, 0x00800 },              /* flash sector 70 - 2kb                        */
  { 0x08023800, 0x00800 },              /* flash sector 71 - 2kb                        */
  { 0x08024000, 0x00800 },              /* flash sector 72 - 2kb                        */
  { 0x08024800, 0x00800 },              /* flash sector 73 - 2kb                        */
  { 0x08025000, 0x00800 },              /* flash sector 74 - 2kb                        */
  { 0x08025800, 0x00800 },              /* flash sector 75 - 2kb                        */
  { 0x08026000, 0x00800 },              /* flash sector 76 - 2kb                        */
  { 0x08026800, 0x00800 },              /* flash sector 77 - 2kb                        */
  { 0x08027000, 0x00800 },              /* flash sector 78 - 2kb                        */
  { 0x08027800, 0x00800 },              /* flash sector 79 - 2kb                        */
  { 0x08028000, 0x00800 },              /* flash sector 80 - 2kb                        */
  { 0x08028800, 0x00800 },              /* flash sector 81 - 2kb                        */
  { 0x08029000, 0x00800 },              /* flash sector 82 - 2kb                        */
  { 0x08029800, 0x00800 },              /* flash sector 83 - 2kb                        */
  { 0x0802a000, 0x00800 },              /* flash sector 84 - 2kb                        */
  { 0x0802a800, 0x00800 },              /* flash sector 85 - 2kb                        */
  { 0x0802b000, 0x00800 },              /* flash sector 86 - 2kb                        */
  { 0x0802b800, 0x00800 },              /* flash sector 87 - 2kb                        */
  { 0x0802c000, 0x00800 },              /* flash sector 88 - 2kb                        */
  { 0x0802c800, 0x00800 },              /* flash sector 89 - 2kb                        */
  { 0x0802d000, 0x00800 },              /* flash sector 90 - 2kb                        */
  { 0x0802d800, 0x00800 },              /* flash sector 91 - 2kb                        */
  { 0x0802e000, 0x00800 },              /* flash sector 92 - 2kb                        */
  { 0x0802e800, 0x00800 },              /* flash sector 93 - 2kb                        */
  { 0x0802f000, 0x00800 },              /* flash sector 94 - 2kb                        */
  { 0x0802f800, 0x00800 },              /* flash sector 95 - 2kb                        */
  { 0x08030000, 0x00800 },              /* flash sector 96 - 2kb                        */
  { 0x08030800, 0x00800 },              /* flash sector 97 - 2kb                        */
  { 0x08031000, 0x00800 },              /* flash sector 98 - 2kb                        */
  { 0x08031800, 0x00800 },              /* flash sector 99 - 2kb                        */
  { 0x08032000, 0x00800 },              /* flash sector 100 - 2kb                       */
  { 0x08032800, 0x00800 },              /* flash sector 101 - 2kb                       */
  { 0x08033000, 0x00800 },              /* flash sector 102 - 2kb                       */
  { 0x08033800, 0x00800 },              /* flash sector 103 - 2kb                       */
  { 0x08034000, 0x00800 },              /* flash sector 104 - 2kb                       */
  { 0x08034800, 0x00800 },              /* flash sector 105 - 2kb                       */
  { 0x08035000, 0x00800 },              /* flash sector 106 - 2kb                       */
  { 0x08035800, 0x00800 },              /* flash sector 107 - 2kb                       */
  { 0x08036000, 0x00800 },              /* flash sector 108 - 2kb                       */
  { 0x08036800, 0x00800 },              /* flash sector 109 - 2kb                       */
  { 0x08037000, 0x00800 },              /* flash sector 110 - 2kb                       */
  { 0x08037800, 0x00800 },              /* flash sector 111 - 2kb                       */
  { 0x08038000, 0x00800 },              /* flash sector 112 - 2kb                       */
  { 0x08038800, 0x00800 },              /* flash sector 113 - 2kb                       */
  { 0x08039000, 0x00800 },              /* flash sector 114 - 2kb                       */
  { 0x08039800, 0x00800 },              /* flash sector 115 - 2kb                       */
  { 0x0803a000, 0x00800 },              /* flash sector 116 - 2kb                       */
  { 0x0803a800, 0x00800 },              /* flash sector 117 - 2kb                       */
  { 0x0803b000, 0x00800 },              /* flash sector 118 - 2kb                       */
  { 0x0803b800, 0x00800 },              /* flash sector 119 - 2kb                       */
  { 0x0803c000, 0x00800 },              /* flash sector 120 - 2kb                       */
  { 0x0803c800, 0x00800 },              /* flash sector 121 - 2kb                       */
  { 0x0803d000, 0x00800 },              /* flash sector 122 - 2kb                       */
  { 0x0803d800, 0x00800 },              /* flash sector 123 - 2kb                       */
  { 0x0803e000, 0x00800 },              /* flash sector 124 - 2kb                       */
  { 0x0803e800, 0x00800 },              /* flash sector 125 - 2kb                       */
  { 0x0803f000, 0x00800 },              /* flash sector 126 - 2kb                       */
  { 0x0803f800, 0x00800 },              /* flash sector 127 - 2kb                       */
#endif
#if (BOOT_NVM_SIZE_KB > 256)
#error "BOOT_NVM_SIZE_KB > 256 is currently not supported."
#endif
};


/*********************************** end of flash_layout.c *****************************/
