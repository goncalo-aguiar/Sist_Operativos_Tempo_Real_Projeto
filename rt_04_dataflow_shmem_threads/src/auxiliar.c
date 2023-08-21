// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


#define IMGWIDTH 128 /* Square image. Side size, in pixels*/
#define BACKGROUND_COLOR 0 /* Color of the background */
#define GUIDELINE_COLOR 255  /* Guideline color */
#define OBSTACLE_COLOR 128 /* Obstacle color */
#define PI 3.14159265

#define GN_ROW 0	/* Row to look for the guiode line - close */
#define GF_ROW (IMGWIDTH-1)	/* Row to look for the guiode line - far */
#define NOB_ROW 3	/* Row to look for near obstacles */
#define NOB_COL 5	/* Col to look for near obstacles */
#define NOB_WIDTH 5	/* WIDTH of the sensor area */



uint8_t pos_;
float angle;
/* Function that detects he position and agle of the guideline */
/* Very crude implemenation. Just for illustration purposes */
int guideLineSearch(uint8_t imageBuf[IMGWIDTH][IMGWIDTH], int16_t *pos_, float *angle) {
	int i, gf_pos;

	*pos_=-1;
	gf_pos=-1;

    int flag_mais_que_uma_guideline =0;
    // printf("\nNOVA->>>\n");
	// for (int i =0;i< 128;i++){
	// 	for (int j =0;j<128;j++){
	// 		printf("%d ",imageBuf[i][j]);
	// 	}
	// 	printf("\n");
	// }
	
	for(i=0; i < IMGWIDTH; i++) {
		if(imageBuf[GN_ROW][i] == GUIDELINE_COLOR) {
			if (flag_mais_que_uma_guideline ==0){
                *pos_ = i;
                flag_mais_que_uma_guideline =1;
            }
            else if(imageBuf[GN_ROW][i-1] != GUIDELINE_COLOR){
                *pos_ = -2;
                break;
            }
            
			// printf("\nI:____%d",i);
			
		}			
	}
	flag_mais_que_uma_guideline =0;
	
	for(i=0; i < IMGWIDTH; i++) {
		if(imageBuf[GF_ROW][i] == GUIDELINE_COLOR) {
            if (flag_mais_que_uma_guideline ==0){
			    gf_pos = i;
                flag_mais_que_uma_guideline =1;
            }
            else if (imageBuf[GF_ROW][i-1] != GUIDELINE_COLOR){
                gf_pos = -2;
                break;
            }
			
			
		}			
	}		
    flag_mais_que_uma_guideline =0;
	// printk("POSSSSSSSSSS: %d %d",*pos_, gf_pos);
	if(*pos_ == -1 || gf_pos == -1) {
		printf("\nFailed to find guideline pos_=%d, gf_pos=%d", *pos_, gf_pos);
		return -1;
	}
    if(*pos_ == -2 || gf_pos == -2) {
		printf("\nMore than one guideline detected", *pos_, gf_pos);
		return -1;
	}
		

	if(*pos_==gf_pos){
		*angle=0;
	} else {	
		float aux = (atan2(*pos_ - gf_pos, GN_ROW - GF_ROW));
		if(aux<0){
			*angle = (aux + PI)*-1;
		} else {
			*angle = PI - aux;
		}
	}
	return 0;	

   
}

/* Function to look for closeby obstacles */

int nearObstSearch(uint8_t imageBuf[IMGWIDTH][IMGWIDTH]) {
	int i, j;
	for(i=0; i < IMGWIDTH/2; i++) {
		for(j=IMGWIDTH/4; j < (IMGWIDTH/4)*3; j++) {
			if(imageBuf[i][j] == OBSTACLE_COLOR && imageBuf[i][j-1] != OBSTACLE_COLOR) {
				return 1;
			}			
		}
	}
	return 0;	
}

/* Function that counts obstacles. 
/* Crude version. Only works if one obstacle per row at max. */
int obstCount(uint8_t imageBuf[IMGWIDTH][IMGWIDTH]) {
	int i, j, nobs;
	uint8_t anterior, atual;
		
	nobs=0;
	anterior = 0;
	atual = 0;
		
	for(i=0; i < IMGWIDTH; i++) {
		for(j=1; j < IMGWIDTH; j++) {
			if(imageBuf[i][j] == OBSTACLE_COLOR && imageBuf[i][j-1] != OBSTACLE_COLOR) {
				nobs++;
			}			
		}
	}
	return nobs;
}
int hexadecimalToDecimal(char * hexdecnumber)
{
    int decimalnumber, i;

    // used to store the power index
    int cnt;

    // used to store the digit
    int digit;

    cnt = 0;
    decimalnumber = 0;

    // iterating the loop using length of hexadecnumber
    for (i = (strlen(hexdecnumber) - 1); i >= 0; i--) {

        // using switch case finding the equivalent decimal
        // digit for each hexa digit
        switch (hexdecnumber[i]) {
        case 'A':
            digit = 10;
            break;
        case 'B':
            digit = 11;
            break;
        case 'C':
            digit = 12;
            break;
        case 'D':
            digit = 13;
            break;
        case 'E':
            digit = 14;
            break;
        case 'F':
            digit = 15;
            break;
        default:
            digit = hexdecnumber[i] - 0x30;
        }

        // obtaining the decimal number
        decimalnumber = decimalnumber + (digit)*pow((double)16, (double)cnt);
        cnt++;
    }
    return decimalnumber;
}

// Following function extracts characters present in `src`
// between `m` and `n` (excluding `n`)
char *substr(const char *src, int m, int n)
{
  // get the length of the destination string
  int len = n - m;

  // allocate (len + 1) chars for destination (+1 for extra null character)
  char *dest = (char *)malloc(sizeof(char) * (len + 1));

  // extracts characters between m'th and n'th index from source string
  // and copy them into the destination string
  for (int i = m; i < n && (*(src + i) != '\0'); i++)
  {
    *dest = *(src + i);
    dest++;
  }

  // null-terminate the destination string
  *dest = '\0';

  // return the destination string
  return dest - len;
}
int main()
{

  for (int t = 1; t <=21; t++)
  {
    char *filename = (char *)malloc(100);

    char int_str[20];

    sprintf(int_str, "%d", t);

    strcpy(filename, "images/img");
    strcat(filename, int_str);
    strcat(filename, ".raw");

    printf("\nFILENAME: %s\n", filename);

    FILE *ptr;
    char ch;
    char *array[128][128];
    char *s = (char *)malloc(128 * 128 * 8);
    ptr = fopen(filename, "r");

    if (NULL == ptr)
    {
      printf("file can't be opened \n");
      exit(1);
    }

    printf("content of this file are \n");

    char *line = NULL;
    size_t len = 0;
    ssize_t read_line;
    int contador_i = 0;
    int contador_j = 0;
    while ((read_line = getline(&line, &len, ptr)) != -1)
    {
      char delim[] = " ";
      char *p = strtok(line, delim);

      while (p != NULL)
      {

        array[contador_i][contador_j] = p; 
        array[contador_i][contador_j]  = substr(array[contador_i][contador_j], 0, 4);
        p = strtok(NULL, delim);
        // printf("%s ", array[contador_i][contador_j]);

        if (contador_j == 127)
        {
          contador_j = 0;
          contador_i++;
        }
        else
        {
          contador_j++;
        }
      }

        
    }
    // printf("AQUIII->%s",array[7][14]);
    // printf("\n");
    uint8_t msg[128][128] = {};
    printf("\nARRAY:\n");
    for (int i = 0; i < 128; i++)
    {

      for (int j = 0; j < 128; j++)
      {
       
        msg[i][j] = hexadecimalToDecimal(array[i][j]);
        // printf("%d ", msg[i][j]);
        if (j == 127)
        {
        //    printf("\n");
        }
      }
    }

    int res = guideLineSearch(msg, &pos_, &angle);
    int obstaculos_na_CSA = nearObstSearch(msg);
    int num_Obstaculos = obstCount(msg);
    printf("\nTem obstaculos na CSA? Sim=1 / Nao=0 -->%d",obstaculos_na_CSA);
    printf("\nNumero de obstaculos detetados:%d",num_Obstaculos);
    // float pos_percentagem = ((int)(pos_)/127) *100;
    printf("\nRobot position=%d, guideline angle = %f",pos_,angle);
   
  }

 
  return 0; // success
}