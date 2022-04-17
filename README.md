# Network        

##usage

  ./crc_encoder input_file output_file generator dataword_size
  ./crc_decoder input_file output_file result_file generator dataword_size



## crc algorithm


  (while loop)
          1)- dataword를 MSB가 1이 될 때까지, 또는  left shift cnt == 0 이 될 때까지 left shift한다.(LSB는 0으로 padding)
              MSB가 1이 될 떄까지 필요한 leftshift 개수 vs 남은 left shift cnt: 더 적은 거 choice
              left_shift_cnt -= choiced 

          2)- MSB가 1이 아니면(left shift cnt 소진)
              그것(left generator_size - 1 bits )이 CRC value가 된다.  

          - MSB가 1이면
              그것과 generator를 XOR한다. (generator 비트만큼) --> 그 결과는 dataword 상위 generator_size bits에 저장

          반복문을 탈출했을 때 나오는 결과의 (generator-1)bits 가 CRC value가 되고, 이를 최초의 dataword에 붙이면 된다. 
        
