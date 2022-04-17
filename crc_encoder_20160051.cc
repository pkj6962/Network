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
void file_open_check(FILE * src, FILE *output);
vector<bool> CharToBinary(char letter);

int calculate_padding_bits(FILE * src, int codeword_size, int dataword_size);
vector<bool> encodeFile(FILE * src, vector<bool>generator, int dataword_size);
vector<bool> crc_encode(vector<bool>& dataword, vector<bool> generator); 
void leftShift(vector<bool>& word, int bits); 
vector<bool> XOR(vector<bool> dataword, vector<bool> generator, int generator_size); 
int find_leftmost_hot_bit(vector<bool> dataword); 

// void add_padding_bits(FILE *output, int padding_bits_cnt);
void add_padding_bits(vector<bool> & buffer, int padding_bits_cnt);

// void write_output(FILE *output, vector<bool>codeword);
void write_onto_buffer(vector<bool> &buffer, vector<bool>codeword);

void print_onto_file(vector<bool> buffer, FILE * output); 
vector<bool> readByte(vector<bool> buffer); 
char BinaryToChar(vector<bool> byte);


int main(int argc, char * argv[]){
    // input_file output_file  generator dataword_size 
    //    1           2           3           4           
    char binary[8]; 
    int dataword_size; 
    
    argv_check(argc); 
    
    FILE *src = fopen(argv[1], "r"); 
    FILE *output = fopen(argv[2], "w"); 
    file_open_check(src, output); 
    
    vector<bool> generator(strlen(argv[3])); 
    for(int i = 0; i < strlen(argv[3]); i++){ // convert string to bitset 
        generator[i] = argv[3][i] - '0'; 
    }

    dataword_size = atoi(argv[4]); 
    
    dataword_check(dataword_size); 

    vector<bool> buffer = encodeFile(src, generator, dataword_size); 

    print_onto_file(buffer, output); 

}
void print_onto_file(vector<bool> buffer, FILE *output){

    // buffer 안의 비트를 8비트로 나눠서 output에 저장 
    

    int data_cnt = buffer.size() / 8;
    vector<bool> byteword(8); 
    char c; 

    for(int i = 0; i < data_cnt; i++){
        
        byteword = readByte({buffer.begin() + 8 * i, buffer.begin() + 8 *(i + 1)}); 
        
        c = BinaryToChar(byteword); 
        
        fputc(c, output);          
    }
} 

char BinaryToChar(vector<bool> byte){
    int result = 0;
    for(int i = 0; i < 8; i++){
        result *= 2; 
        result += byte[i]; 
    }
    return char(result); 
}

vector<bool> readByte(vector<bool> buffer)
{
    vector<bool> byteword(8); 
    copy(buffer.begin(), buffer.begin() + 8, byteword.begin());

    return byteword; 
}


vector<bool> encodeFile(FILE * src, vector<bool> generator, int dataword_size){
    char c; int total_bits = 0; 
    int codeword_size = dataword_size + generator.size() - 1; 
    vector<bool> codeword(codeword_size); 
    vector<bool> dataword(dataword_size); 
    vector<bool> charbyte(8);
    vector<bool> buffer; 

    // calculate_total_bits(src, codeword_size); 
    int padding_bits_cnt = calculate_padding_bits(src, codeword_size, dataword_size);
    //write_output(output, CharToBinary(padding_bits_cnt)); 
    write_onto_buffer(buffer, CharToBinary(padding_bits_cnt)); 
    //add_padding_bits(output, padding_bits_cnt); 
    add_padding_bits(buffer, padding_bits_cnt); 

    rewind(src); 

    while(fscanf(src, "%c", &c)!= -1){
    
        charbyte = CharToBinary(c); 

        if(dataword_size == 8){
            copy(charbyte.begin(), charbyte.end(), dataword.begin()); 
            codeword = crc_encode(dataword, generator); 

            total_bits += codeword_size; 
            //write_output(output, codeword); 
            write_onto_buffer(buffer, codeword); 
        }
        else{ // dataword_size == 4
            int j = 0; 
            for(int i = 0; i < 2; i++){
                for(int offset = 0; offset < 4; offset++){
                    dataword[offset] = charbyte[j]; 
                    j ++; 
                }
                codeword = crc_encode(dataword, generator);
                //write_output(output, codeword); 

                write_onto_buffer(buffer, codeword); 
            }
            total_bits += codeword_size * 2; 
        }
    }

    for(int i =0; i <buffer.size(); i++){
        cout << buffer[i]; 
    }
    cout << '\n'; 


    return buffer; 
}


/*
void add_padding_bits(FILE *output, int padding_bits_cnt){
    for(int i = 0; i < padding_bits_cnt; i++){
        fputc('0', output); 
    }
}
*/

void add_padding_bits(vector<bool> &buffer, int padding_bits_cnt){
    for(int i = 0; i < padding_bits_cnt; i++){
        buffer.push_back(0); 
    }
}


int calculate_padding_bits(FILE * src, int codeword_size, int dataword_size){
    
    char c;
    int total_char = 0; 
    while((c = fgetc(src)) != -1){
        total_char ++; 
    }
    int total_bits = total_char * codeword_size;
    if(dataword_size == 4) total_bits *= 2; 

    int padding_bits = 8 - total_bits % 8;  
    return padding_bits; 
}
/*
void write_output(FILE *output, vector<bool>codeword){
    for(int i = 0; i < codeword.size(); i++)
        fputc(codeword[i]? '1': '0', output);
}
*/


void write_onto_buffer(vector<bool> &buffer, vector<bool>codeword){
    for(int i = 0; i < codeword.size(); i++)
        buffer.push_back(codeword[i]); 
}

vector<bool> crc_encode(vector<bool>& dataword, vector<bool> generator){ // convert one dataword into codeword
    
    
    int codeword_size = dataword.size() + generator.size() - 1; 
    int generator_size = generator.size(); 
    int left_shift_cnt = dataword.size(); 
    vector<bool> XOR_result(generator_size);
    vector<bool> codeword(codeword_size,0);
    
    copy(dataword.begin(), dataword.end(), codeword.begin()); 



    if(generator[0] != 1){
        int first_set_bit = find_leftmost_hot_bit(generator);
        leftShift(generator, first_set_bit); 
    }

    while(true){
        int needed_cnt_for_MSB_1 = find_leftmost_hot_bit(dataword); 
        int choiced_cnt = (needed_cnt_for_MSB_1 < left_shift_cnt)? needed_cnt_for_MSB_1 : left_shift_cnt; 
        left_shift_cnt -= choiced_cnt; 
        leftShift(dataword, choiced_cnt); 

        if(left_shift_cnt == 0) break; 

        if(dataword[0] == 1){
            XOR_result = XOR(dataword, generator, generator_size); 
            copy(XOR_result.begin(), XOR_result.end(), dataword.begin()); 
        }
    }

    copy(dataword.begin(), dataword.begin() + generator_size - 1, codeword.begin() + dataword.size()); 


    return codeword; 
    
    //copy(dataword.begin(), dataword.end(), codeword.begin()); 

/*
    1. dataword
    
        1. dataword에 padding 0bits를 붙인다.(x^k를 곱하는 것과 같은 것 ) 
            - 몇 개를 붙이는가? 
                generator의 차수(strlen(generator) - 1)만큼 

        if generator's highest bit is not 1
            meaningless 0를 지우자. 
            >> 0의 개수만큼 left shift 
            

        given dataword(4bits or 8bits), generator( k bits)

*/
/*
    
    */

        /*
        (while loop)
        1)- dataword를 MSB가 1이 될 때까지, 또는  left shift cnt == 0 이 될 때까지 left shift한다.(LSB는 0으로 padding)
            MSB가 1이 될 떄까지 필요한 leftshift 개수 vs 남은 left shift cnt: 더 적은 거 choice
            left_shift_cnt -= choiced 

        2)- MSB가 1이 아니면(left shift cnt 소진)
            그것(left generator_size - 1 bits )이 CRC value가 된다.  

        - MSB가 1이면
            그것과 generator를 XOR한다. (generator 비트만큼) --> 그 결과는 dataword 상위 generator_size bits에 저장
           
        반복문을 탈출했을 때 나오는 결과의 (generator-1)bits 가 CRC value가 되고, 이를 최초의 dataword에 붙이면 된다. 
        


    */
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


vector<bool> CharToBinary(char letter){

    vector<bool> dataword(8); 
    int ascii = (int)letter; 
    
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
    if(argc != 5){
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n"); 
        exit(1); 
    }
}

void file_open_check(FILE * src, FILE *output){
    if(src == NULL){
        printf("input file open error\n"); 
        exit(2); 
    }
    else if(output == NULL){
        printf("output file open error\n"); 
        exit(3); 
    }

}
