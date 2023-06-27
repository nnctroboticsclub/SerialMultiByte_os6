#include "SerialMultiByte.h"

SerialMultiByte::SerialMultiByte(PinName tx, PinName rx) : UnbufferedSerial(tx, rx, DEFAULT_BAUD) {
    header[0] = DEFAULT_FIRST_HEADER;
    header[1] = DEFAULT_SECOND_HEADER;
}

SerialMultiByte::SerialMultiByte(PinName tx, PinName rx, int baud) : UnbufferedSerial(tx, rx, baud) {
    header[0] = DEFAULT_FIRST_HEADER;
    header[1] = DEFAULT_SECOND_HEADER;
}

void SerialMultiByte::setHeaders(uint8_t firstHeader, uint8_t secondHeader) {
    header[0] = firstHeader;
    header[1] = secondHeader;
}

void SerialMultiByte::setHeaders(uint8_t headers[]) {
    header[0] = headers[0];
    header[1] = headers[1];
}

void SerialMultiByte::startReceive(uint16_t dataSize) {
    bufferSize = dataSize + 3;
  bufferPoint = 0;
  receivedBytes = 0;
  buffer = new uint8_t[bufferSize];
  data = new uint8_t[dataSize];
  attach(callback(this, &SerialMultiByte::receiveByte));

}

void SerialMultiByte::receiveByte() {
    uint8_t receivedData;
    if (read(&receivedData, 1)) {
        buffer[bufferPoint % bufferSize] = receivedData;

        if (bufferPoint != 0xff) {
            ++bufferPoint;
        } else {
            bufferPoint = (255 % bufferSize) + 1;
        }

        ++receivedBytes;

        if (receivedBytes >= bufferSize) {
            checkData();
        }
    }
}


void SerialMultiByte::checkData() {
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i % bufferSize] == header[0] &&
            buffer[(i + 1) % bufferSize] == header[1]) {
            uint8_t checksum = 0x00;
            for (int j = 0; j < bufferSize - 3; j++) {
                checksum += buffer[(i + 2 + j) % bufferSize];
            }
            if (checksum == buffer[(i + bufferSize - 1) % bufferSize]) {
                for (int j = 0; j < bufferSize - 3; j++) {
                    data[j] = buffer[(i + 2 + j) % bufferSize];
                }
                receivedBytes = 0;
                return;
            }
        }
    }
}

void SerialMultiByte::getData(uint8_t data_[]) {
    for (int i = 0; i < bufferSize - 3; i++) {
        data_[i] = data[i];
    }
}

void SerialMultiByte::sendData(uint8_t data_[], uint16_t length) {
    uint8_t checksum = 0x00;
    for (int i = 0; i < length; i++) {
        checksum += data_[i];
    }

    // ヘッダを送信
    write(&header[0], 1);
    write(&header[1], 1);

    // データを送信
    write(data_, length);

    // チェックサムを送信
    write(&checksum, 1);
}
