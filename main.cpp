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