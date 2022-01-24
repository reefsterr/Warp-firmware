void initINA219(const uint8_t i2cAddress);
void writeINARegister(uint8_t deviceRegister, uint8_t payload1, uint8_t payload2);
uint8_t readINA(uint8_t deviceRegister, int numberOfBytes);