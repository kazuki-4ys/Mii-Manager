unsigned short getCrc(unsigned char*buf,int size){
    unsigned int crc = 0x0000;
    int byteIndex,bitIndex,counter;
    for (byteIndex = 0; byteIndex < size; byteIndex++) {
        for (bitIndex = 7; bitIndex >= 0; bitIndex--) {
            crc = (((crc << 1) | ((buf[byteIndex] >> bitIndex) & 0x1)) ^
            (((crc & 0x8000) != 0) ? 0x1021 : 0)); 
        }
    }
    for (counter = 16; counter > 0; counter--) {
        crc = ((crc << 1) ^ (((crc & 0x8000) != 0) ? 0x1021 : 0));
    }
    return (unsigned short)(crc & 0xFFFF);
}