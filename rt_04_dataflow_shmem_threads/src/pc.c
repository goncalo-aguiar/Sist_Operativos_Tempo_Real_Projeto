
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
  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  int serial_port = open("/dev/ttyACM0", O_RDWR);

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if (tcgetattr(serial_port, &tty) != 0)
  {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
  tty.c_cflag |= CS8;            // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;                                                        // Disable echo
  tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
  tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
  tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 115200
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
  {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 1;
  }

  for (int t = 1; t <=99; t++)
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
      printf("file %s can't be opened \n",filename);
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
        array[contador_i][contador_j]  = substr(array[contador_i][contador_j], 2, 4);
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
    uint8_t msg[128*128] = {};
    printf("\nARRAY:\n");
    for (int i = 0; i < 128; i++)
    {

      for (int j = 0; j < 128; j++)
      {
        printf("%s ", array[i][j]);
        
        if (j == 127)
        {
          printf("\n");
        }
      }
    }
     // Allocate memory for read buffer, set size according to your needs
    char read_buf [256];

    // Normally you wouldn't do this memset() call, but since we will just receive
    // ASCII data for this example, we'll set everything to 0 so we can
    // call printf() easily.
    memset(&read_buf, '\0', sizeof(read_buf));
    int contador=0;
    int contador_msg =0;
    for (int i =0;i< 128;i++){
      for (int j =0;j<128;j++){
       
        msg[contador_msg] = hexadecimalToDecimal(array[i][j]);
        if (contador_msg==16383){
          contador_msg =0; 
          // for (int k=0;k<16;k++){
          //   printf("%d ",msg[k]);
          // }
          // printf("\n");
          write(serial_port, &msg, 128*128);
          int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
          if (num_bytes < 0) {
              printf("Error reading: %s", strerror(errno));
              return 1;
          }
          // printf("\nRead %i bytes. Received message: %s", num_bytes, "ACK");  
          // printf("\nRead %i bytes. Received message: %s", num_bytes, read_buf);  
          
          // while(atoi(read_buf) !=1){
          //   // printf("\nRead %i bytes. Received message: %s", num_bytes, read_buf);  
            
          
          // }

          
          usleep (2000000);
          
          
          tcflush(serial_port,TCIFLUSH);

          
        
        }
        else{
          // printf("\n%d",contador_msg);
          contador_msg++;
        }

      }

    }

    fclose(ptr);
  }

  close(serial_port);
  return 0; // success
}