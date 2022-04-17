#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <iostream>
#include <bitset>
#include <stdbool.h>
#include <vector>
#include <string.h>

using namespace std;


void dataword_check(int dataword_size);
void argv_check(int argc);
void file_open_check(FILE * src, FILE *output, FILE *result);
// void read_padding_bits(FILE *src);
void read_padding_bits(vector<bool> encodedData, int *base_idx);
void decode_file(vector<bool> encodedData, vector<bool> generator, int dataword_size, FILE *output, FILE *result);
// void decode_file(vector<bool> generator, int dataword_size, FILE * src, FILE *output, FILE *result); 
char  BinaryToChar(vector<bool> byte);
vector<bool> CharToBinary(unsigned int letter);

void leftShift(vector<bool>& word, int bits); 
vector<bool> XOR(vector<bool> dataword, vector<bool> generator, int generator_size); 
int find_leftmost_hot_bit(vector<bool> dataword); 

vector<bool> crc_decode(vector<bool> codeword, vector<bool> generator, int* err_cnt);
// int readCodeword(vector <bool>& codeword, FILE *src);
vector<bool> read_codeword(vector<bool>encodedData, int* baseIdx, int codeword_size);
vector<bool> read_file(FILE *src);


int main(int argc, char * argv[]){
    // input_file output_file result_file  generator dataword_size 
    //    1           2           3           4            5

    char binary[8]; 
    int dataword_size; 
    
    argv_check(argc); 
    
    FILE *src = fopen(argv[1], "r"); 
    FILE *output = fopen(argv[2], "w");
    FILE *result = fopen(argv[3], "w"); 
    file_open_check(src, output, result); 
    
    vector<bool> generator(strlen(argv[4])); 
    for(int i = 0; i < strlen(argv[4]); i++){ // convert string to bitset 
        generator[i] = argv[4][i] - '0'; 
    }

    dataword_size = atoi(argv[5]); 
    dataword_check(dataword_size); 


    vector<bool> encodedData;

    encodedData = read_file(src); 

    // decode_file(generator, dataword_size, src, output, result); 

    decode_file(encodedData, generator, dataword_size, output, result); 


}
vector<bool> read_file(FILE *src){
    unsigned int c; 
    vector<bool> byteword(8); 
    vector<bool> encodedData;
    while((c = fgetc(src)) != -1){
        byteword = CharToBinary(c);

        for(int i = 0; i < 8; i++){
            encodedData.push_back(byteword[i]);
        }
    }

    for(int i = 0; i < encodedData.size(); i++){
        cout << encodedData[i];
    }

    return encodedData;
}
/*
buffer에서 8비트 읽기 
padding_bit_cnt_binary = readByte(buffer)
padding_bit_cnt = int(BinaryToChar(pading_bit_cnt_binary));

int begin_idx = 8 + padding_bit_cnt;  // 여기서부터 읽기 시작 

int codeword_size; 

encoded_data_size = buffer.size();

codeword_cnt = (encoded_data_size - 8  - padding_bit_cnt) / 8;

vector<bool> codeword(codeword_size); 

for(int i = 0; i < codeword_cnt; i++){
    
    codeword = read_codeword({buffer.begin() + begin_idx + i * codeword_size, buffer.begin() + begin_idx + (i+1) * codeword_size});

}

*/



void decode_file(vector<bool> encodedData, vector<bool> generator, int dataword_size, FILE *output, FILE *result){
    
    int generator_size = generator.size();
    int codeword_size = dataword_size + generator_size - 1; 
    int err_cnt = 0, total_cnt = 0; 
    bool left_half_exist = false; 
    vector<bool> codeword(codeword_size);
    vector<bool> dataword(dataword_size); 
    vector<bool> bytedata(8); 
    vector<bool> left_half(4); 
    int baseIdx = 0; 
    int codeword_cnt;
 
    read_padding_bits(encodedData, &baseIdx);
    codeword_cnt = (encodedData.size() - baseIdx) / codeword_size; 

    cout << '\n' << codeword_cnt << '\n'; 
    for(int i = 0; i < codeword_cnt; i ++){
        
        codeword = read_codeword(encodedData, &baseIdx, codeword_size); 

        dataword = crc_decode(codeword, generator, &err_cnt); 

        if(dataword_size == 4){
            if(left_half_exist){
                copy(dataword.begin(), dataword.end(), bytedata.begin() + 4);
                left_half_exist = false; 
            } 
            else{
                copy(dataword.begin(), dataword.end(), bytedata.begin()); 
                left_half_exist = true; 
            }
        }
        else{ // dataword_size == 8
            copy(dataword.begin(), dataword.end(), bytedata.begin()); 
        }

        //출력
        if(dataword_size == 8 || !left_half_exist){
            char c = BinaryToChar(bytedata); 
            fputc(c, output); 
        }

    }
    fprintf(result, "%d %d", codeword_cnt, err_cnt); 

}
vector<bool> read_codeword(vector<bool>encodedData, int* baseIdx, int codeword_size){
    vector<bool> codeword(codeword_size);

    copy(encodedData.begin() + *baseIdx, encodedData.begin() + *baseIdx + codeword_size, codeword.begin()); 

    *baseIdx = *baseIdx + codeword_size; 

    return codeword; 
}

void read_padding_bits(vector<bool> encodedData, int *base_idx){
    vector <bool> padding_bits_cnt; 
    for(int i = 0; i < 8; i++){
        padding_bits_cnt.push_back(encodedData[i]); 
    }

    int cnt = int(BinaryToChar(padding_bits_cnt));
    
    *base_idx = 8 + cnt; // base_idx부터 실제 dataword의 전송 시작
}


/*
void decode_file(vector<bool> generator, int dataword_size, FILE * src, FILE *output, FILE *result){
    
    int generator_size = generator.size();
    int codeword_size = dataword_size + generator_size - 1; 
    int err_cnt = 0, total_cnt = 0; 
    bool left_half_exist = false; 
    vector<bool> codeword(codeword_size);
    vector<bool> dataword(dataword_size); 
    vector<bool> bytedata(8); 
    vector<bool> left_half(4); 


    read_padding_bits(src); 
    while(readCodeword(codeword, src) != -1){

        total_cnt ++; 

        dataword = crc_decode(codeword, generator, &err_cnt); 

        if(dataword_size == 4){
            if(left_half_exist){
                copy(dataword.begin(), dataword.end(), bytedata.begin() + 4);
                left_half_exist = false; 
            } 
            else{
                copy(dataword.begin(), dataword.end(), bytedata.begin()); 
                left_half_exist = true; 
            }
        }
        else{ // dataword_size == 8
            copy(dataword.begin(), dataword.end(), bytedata.begin()); 
        }

        //출력
        if(dataword_size == 8 || !left_half_exist){
            char c = BinaryToChar(bytedata); 
            fputc(c, output); 
        }

    }

    fprintf(result, "%d %d", total_cnt, err_cnt); 


}
*/

vector<bool> crc_decode(vector<bool> codeword, vector<bool> generator, int* err_cnt){

    int dataword_size = codeword.size() - (generator.size() - 1); 
    int generator_size = generator.size();
    vector<bool> dataword(dataword_size); 
    vector<bool> XOR_result(generator_size); 


    copy(codeword.begin(), codeword.begin() + dataword_size, dataword.begin()); 

    int left_shift_cnt = dataword.size(); 

    if(generator[0] != 1){
        int first_set_bit = find_leftmost_hot_bit(generator);
        leftShift(generator, first_set_bit); 
    }

    while(true){
        int needed_cnt_for_MSB_1 = find_leftmost_hot_bit(codeword); 
        int chosen_cnt = (needed_cnt_for_MSB_1 < left_shift_cnt)? needed_cnt_for_MSB_1 : left_shift_cnt; 
        left_shift_cnt -= chosen_cnt; 
        leftShift(codeword, chosen_cnt); 

        if(left_shift_cnt == 0) break; 

        if(codeword[0] == 1){
            XOR_result = XOR(codeword, generator, generator_size); 
            copy(XOR_result.begin(), XOR_result.end(), codeword.begin()); 
        }
    }

    for(int i = 0; i < generator_size -1 ; i++){
        if(codeword[i] != 0){
            *err_cnt = *err_cnt + 1; 
            break; 
        }
    }
     //상위 generator.size() - 1 비트가 다 0이면 나누어 떨어진 것

    return dataword; 
}



vector<bool> XOR(vector<bool> dataword, vector<bool> generator, int generator_size){
    vector<bool> XOR_result(generator_size); 

    for(int i = 0; i < generator_size; i++){
        XOR_result[i] = dataword[i]^generator[i]; 
    }
    return XOR_result; 
}


int find_leftmost_hot_bit(vector<bool> dataword){
    int idx = -1;
    for(idx= 0; idx < dataword.size(); idx++){
        if(dataword[idx] == 1)
            break; 
    }
    return idx; 
}

void leftShift(vector<bool>& word, int bits){
    int len = word.size();
    for(int i = bits; i < len; i++)
        word[i - bits] = word[i];
    for(int i = len - bits; i < len; i++)
        word[i] = 0; 
}
/*
void read_padding_bits(FILE *src){
    vector<bool> byte(8);
    int garbage; 
    for(int i = 0; i < 8 ; i ++){
        byte[i] = (fgetc(src) == '0')? 0 : 1; 
    }

    int padding_bits_cnt = int(BinaryToChar(byte)); 

    for(int i = 0; i < padding_bits_cnt; i++){
        garbage = fgetc(src); 
    }
}
*/
/*
int readCodeword(vector <bool>& codeword, FILE *src){
    int c;
    if((c = fgetc(src)) == EOF) return -1;
    codeword[0] = (c == '0')? 0 : 1;

    for(int i = 1; i < codeword.size(); i++){
        codeword[i] = (fgetc(src) == '0')? 0:1; 
    }
    return 0; 
}

*/
char BinaryToChar(vector<bool> byte){
    int result = 0;
    for(int i = 0; i < 8; i++){
        result *= 2; 
        result += byte[i]; 
    }
    return char(result); 
}

vector<bool> CharToBinary(unsigned letter){

    vector<bool> dataword(8); 
    unsigned int ascii = letter; 
    
    for(int i = 7; i >= 0; i--){
        dataword[i] = ascii % 2;
        ascii /= 2; 
    }
    return dataword; 
}

void dataword_check(int dataword_size){
    if(dataword_size != 4 && dataword_size != 8){
        printf("dataword size must be 4 or 8"); 
        exit(4);     
    }
}

void argv_check(int argc){
    if(argc != 6){
        printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n"); 
        exit(1); 
    }
}

void file_open_check(FILE * src, FILE *output, FILE *result){
    if(src == NULL){
        printf("input file open error\n"); 
        exit(2); 
    }
    else if(output == NULL){
        printf("output file open error\n"); 
        exit(3); 
    }
    else if (result == NULL){
        printf("result file open error");
        exit(4); 
    }

}

