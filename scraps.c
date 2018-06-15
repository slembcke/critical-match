static void attr_pal(u8 x, u8 y, u8 pal){
	static const u8 SUB_MASK[] = {0x03, 0x0C, 0x30, 0xC0};
	static const u8 PAL[] = {0x00, 0x55, 0xAA, 0xFF};
	
	u8 mask = SUB_MASK[(x & 1) | ((y & 1) << 1)];
	u16 addr = 0x23C0 | (u8)((y & 0xFE) << 2) | (u8)(x >> 1);
	u8 value;
	
	px_addr(addr);
	// First read resets the IO register and returns garbage.
	value = PPU.vram.data;
	value = PPU.vram.data;
	
	px_addr(addr);
	PPU.vram.data = (value & ~mask) | (PAL[pal] & mask);
}
