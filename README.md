# C/C++实现读写WAV音频文件

WAV音频文件本质上是一个二进制文件，C语言可通过fopen读写二进制文件的方法读写WAV音频文件。但WAV文件内不仅存储了音频的数据，还存储的文件信息，因此若要获取正确的WAV文件内容，需要知道WAV存储的的格式，按照WAV格式进行读写操作。

## WAV格式文件

WAV（Waveform Audio File Format）是一种常见的音频文件格式，它通常用于存储未经压缩的音频数据。WAV文件遵循RIFF规则，其内容以区块为最小单位进行存储。WAV文件一般由3个区块组成：RIFF区块、Format区块和Data区块。

## RIFF区块

| 名称      | 偏移地址 | 字节数 | 数据类型 | 端序 | 内容     |
| --------- | -------- | ------ | -------- | ---- | -------- |
| chunkID   | 0x00     | 4      | char     | 大端 | “RIFF”   |
| chunkSize | 0x04     | 4      | uint32_t | 小端 | 文件长度 |
| format    | 0x08     | 4      | char     | 大端 | “WAVE”   |

## Format区块

| 名称          | 偏移地址 | 字节数 | 数据类型 | 端序 | 内容                                                         |
| ------------- | -------- | ------ | -------- | ---- | ------------------------------------------------------------ |
| subchunk1ID   | 0x0C     | 4      | char     | 大端 | “fmt”，fmt标识                                               |
| audioFormat   | 0x10     | 4      | uint32_t | 小端 | 子块大小，通常为16                                           |
| audioFormat   | 0x14     | 2      | uint16_t | 小端 | 音频格式代码，1表示PCM无损编码                               |
| numChannels   | 0x16     | 2      | uint16_t | 小端 | 声道数，通常为1（单声道）或2（立体声）                       |
| sampleRate    | 0x18     | 2      | uint16_t | 小端 | 采样率，表示每秒的采样数                                     |
| byteRate      | 0x1C     | 4      | uint32_t | 小端 | 每秒的字节数，等于`sampleRate` * `numChannels` * (`bitsPerSample` / 8) |
| blockAlign    | 0x20     | 2      | uint16_t | 小端 | 块对齐，等于`numChannels` * (`bitsPerSample` / 8)            |
| bitsPerSample | 0x22     | 2      | uint16_t | 小端 | 每个样本的位深度，通常为8、16、24或32                        |

## Data区块

| 名称          | 偏移地址 | 字节数 | 数据类型 | 端序 | 内容                                                         |
| ------------- | -------- | ------ | -------- | ---- | ------------------------------------------------------------ |
| subchunk2ID   | 0x24     | 4      | char     | 大端 | 子块标识，通常为"data"                                       |
| subchunk2Size | 0x28     | 4      | uint32_t | 小端 | 音频数据大小，等于采样数 * `numChannels` * (`bitsPerSample` / 8) |
| data          | 0x2C     |        |          |      | 数据                                                         |

> `uint16_t` 和 `uint32_t` 是无符号整数类型的数据类型，它们是C/C++ \<stdint.h\>标准库中定义的确切位数的整数类型.**`uint16_t`**：这是一个16位无符号整数类型，确保有16位位数，没有符号位。**`uint32_t`**：这是一个32位无符号整数类型，确保有32位位数，没有符号位。

# C语言实现

```C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// 定义WAV文件头部结构
struct WAVHeader {
    char chunkID[4];        // 文件标识，通常为"RIFF"
    uint32_t chunkSize;     // 文件大小
    char format[4];         // 文件格式，“WAVE”
    char subchunk1ID[4];    // 子块标识，“fmt”
    uint32_t subchunk1Size; // 子块大小
    uint16_t audioFormat;   // 音频格式，1为PCM格式
    uint16_t numChannels;   // 声道数，1:单声道，2:双声道
    uint32_t sampleRate;    // 采样率
    uint32_t byteRate;      // 每秒的字节数
    uint16_t blockAlign;    // 块对齐
    uint16_t bitsPerSample; // 采样深度
    char subchunk2ID[4];    // 子块标识，“data”
    uint32_t subchunk2Size; // 子块大小
};

// 音频数据结构
struct WAVData {
    struct WAVHeader header;  // 音频头部数据
    uint8_t* sample;          // 音频数据
};

// 读取wav格式文件
struct WAVData* audioread(const char* filename);
// 写入wav格式文件
void audiowrite(const char* filename, struct WAVData* audio_data);

int main() {
    char filename[30] = "../inputs/test.wav";
    struct WAVData* audio_data = audioread(filename);
    if (audio_data == NULL) {
        perror("文件打开失败!");
        return 1;
    }

    char outfilename[30] = "../outputs/test.wav";
    audiowrite(outfilename, audio_data);

    return 0;
}

struct WAVData* audioread(const char* filename) {
    // 打开文件
    FILE* inputFile = fopen(filename, "rb");
    struct WAVData* audio_data = NULL;
    if (inputFile == NULL) {
        perror("文件代开失败!");
        return NULL;
    }

    // 读取文件头部信息
    struct WAVHeader header;
    fread(&header, sizeof(struct WAVHeader), 1, inputFile);

    // 验证文件格式
    if (strncmp(header.chunkID, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        perror("无效文件!");
        fclose(inputFile);
        return NULL;
    }

    // 输出wav文件信息
    printf("音频格式: %hu\n", header.audioFormat);
    printf("声道数: %hu\n", header.numChannels);
    printf("采样率: %hu\n", header.sampleRate);
    printf("采样深度: %hu\n", header.bitsPerSample);

    // 读取wav文件数据
    audio_data = (struct WAVData*)malloc(sizeof(struct WAVData));
    if (audio_data == NULL) {
        perror("内存申请失败!");
        return NULL;
    }
    audio_data->header = header;
    audio_data->sample = (uint8_t*)malloc(header.subchunk2Size);
    if (audio_data->sample == NULL) {
        perror("内存申请失败!");
        free(audio_data);
        return NULL;
    }
    fread(audio_data->sample, header.subchunk2Size, 1, inputFile);
    fclose(inputFile);

    return audio_data;
}

void audiowrite(const char* filename, struct WAVData* audio_data) {
    // 打开输出文件
    FILE* outputFile = fopen(filename, "wb");
    if (outputFile == NULL) {
        perror("文件打开失败!");
        return;
    }

    // 写入文件
    fwrite(&audio_data->header, sizeof(struct WAVHeader), 1, outputFile);
    fwrite(audio_data->sample, audio_data->header.subchunk2Size, 1, outputFile);

    // 关闭文件
    fclose(outputFile);
}
```

