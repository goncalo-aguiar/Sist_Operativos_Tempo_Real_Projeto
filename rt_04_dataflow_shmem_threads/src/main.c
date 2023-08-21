#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdlib.h>
#include <stdio.h>
#include <drivers/gpio.h>
#include <math.h>
#include <drivers/adc.h>
#include <time.h>
#include <cab.h>

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>

#include <stdint.h>

///////////////////////////////////////////////////////////////
#include <zephyr.h>
#include <device.h>
#include <drivers/uart.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//////////////////////////////////////////////////

#define SLEEP_TIME_MS   10

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 1024

/* Thread scheduling priority */

#define thread_A_prio 2
#define thread_B_prio 5
#define thread_C_prio 4
#define thread_D_prio 3

/////////////////////////////////////////////////////////
#define MAIN_SLEEP_TIME_MS 1000 /* Time between main() activations */ 

#define FATAL_ERR -1 /* Fatal error return code, app terminates */

#define UART_NODE DT_NODELABEL(uart0)    /* UART Node label, see dts */

#define RXBUF_SIZE 128*128             /* RX buffer size */
#define TXBUF_SIZE 1                 /* TX buffer size */
#define RX_TIMEOUT 1000                  /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */

/////////////////////////////////////////////////////////////////7



/* Create thread stack space */

K_THREAD_STACK_DEFINE(thread_A_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_B_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_C_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_D_stack, STACK_SIZE);

  
/* Create variables for thread data */

struct k_thread thread_A_data;
struct k_thread thread_B_data;
struct k_thread thread_C_data;
struct k_thread thread_D_data;


/* Create task IDs */

k_tid_t thread_A_tid;
k_tid_t thread_B_tid;
k_tid_t thread_C_tid;
k_tid_t thread_D_tid;



/* Thread code prototypes */

void thread_A_code(void *argA, void *argB, void *argC);
void thread_B_code(void *argA, void *argB, void *argC);
void thread_C_code(void *argA, void *argB, void *argC);
void thread_D_code(void *argA, void *argB, void *argC);

#define IMGWIDTH 128 /* Square image. Side size, in pixels*/
uint8_t img[IMGWIDTH][IMGWIDTH];


uint8_t pos_;
float angle;
int obstaculos_na_CSA;
int num_Obstaculos;


/* Struct for UART configuration (if using default values is not needed) */
const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

/* UAR related variables */
const struct device *uart_dev;          /* Pointer to device struct */ 
static uint8_t rx_buf[RXBUF_SIZE];      /* RX buffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE];    /* chars actually received  */
volatile int uart_rxbuf_nchar=0;        /* Number of chars currnetly on the rx buffer */

/* UART callback function prototype */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

cab *cab_id;
int flag_A = 0;
int flag_B = 0;
int flag_C = 0;
uint8_t *imagem;
uint8_t *imagem_A;
uint8_t *imagem_B;
uint8_t *imagem_C;

int primeira_imagem = 0;
/* Main function */
void main(void) {
    
   
    /* Create and init semaphores */
	
    int ret;
    /* Check if device is ready */
    
    /* Create tasks */
    

	thread_A_tid = k_thread_create(&thread_A_data, thread_A_stack,
        K_THREAD_STACK_SIZEOF(thread_A_stack), thread_A_code,
        NULL, NULL, NULL, thread_A_prio, 0, K_NO_WAIT);

    thread_B_tid = k_thread_create(&thread_B_data, thread_B_stack,
        K_THREAD_STACK_SIZEOF(thread_B_stack), thread_B_code,
        NULL, NULL, NULL, thread_B_prio, 0, K_NO_WAIT);

    thread_C_tid = k_thread_create(&thread_C_data, thread_C_stack,
        K_THREAD_STACK_SIZEOF(thread_C_stack), thread_C_code,
        NULL, NULL, NULL, thread_C_prio, 0, K_NO_WAIT);
    
    thread_D_tid = k_thread_create(&thread_D_data, thread_D_stack,
        K_THREAD_STACK_SIZEOF(thread_D_stack), thread_D_code,
        NULL, NULL, NULL, thread_D_prio, 0, K_NO_WAIT);

	
	/* Local vars */    
    int err=0; /* Generic error variable */
    uint8_t welcome_mesg[] = "UART demo: Type a few chars in a row and then pause for a little while ...\n\r"; 
    uint8_t rep_mesg[TXBUF_SIZE];

    /* Bind to UART */
    uart_dev= device_get_binding(DT_LABEL(UART_NODE));
    if (uart_dev == NULL) {
        printk("device_get_binding() error for device %s!\n\r", DT_LABEL(UART_NODE));
        return;
    }
    else {
        printk("UART binding successful\n\r");
    }

    /* Configure UART */
    err = uart_configure(uart_dev, &uart_cfg);
    if (err == -ENOSYS) { /* If invalid configuration */
        printk("uart_configure() error. Invalid configuration\n\r");
        return; 
    }

    /* Register callback */
    err = uart_callback_set(uart_dev, uart_cb, NULL);
    if (err) {
        printk("uart_callback_set() error. Error code:%d\n\r",err);
        return;
    }
		
    /* Enable data reception */
    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
    if (err) {
        printk("uart_rx_enable() error. Error code:%d\n\r",err);
        return;
    }

    /* Send a welcome message */ 
    /* Last arg is timeout. Only relevant if flow controll is used */
    // err = uart_tx(uart_dev, welcome_mesg, sizeof(welcome_mesg), SYS_FOREVER_MS);
    // if (err) {
    //     printk("uart_tx() error. Error code:%d\n\r",err);
    //     return;
    // }
    int contador_i=0;
	int contador_j=0;
	char * aux;
	int flag_msg =0;
    /* Main loop */
	

	
	
    while(1) {
        k_msleep(MAIN_SLEEP_TIME_MS);
        
        /* Print string received so far. */
        /* Very basic implementation, just for showing the use of the API */
        /* E.g. it does not prevent race conditions with the callback!!!!*/

		
        if(uart_rxbuf_nchar > 0) {
            
			
			
			
            uart_rxbuf_nchar = 0;           /* Reset counter */

			for (int j = 0;j< 128*128;j++){
				img[contador_i][contador_j] = rx_chars[j];
				if (j%128==0 && contador_i !=0){
					contador_i++;
					contador_j = 0;
					// printf("\n");
				}
				else{
					contador_j++;
				}
				// printf("%d ",rx_chars[j]);
				
			}
			// printf("\nCOMEÇA AQUI->\n");
			// for (int i =0;i< 128;i++){
			// 		for (int j =0;j<128;j++){
			// 			printf("%d ",img[i][j]);
			// 		}
			// 	printf("\n");
			// }

			if (primeira_imagem ==0){
				// printf("\nCHEGUEI AQUI1");
				cab_id = open_cab("cab",4,128*128,img);
				// printf("\nIMAGEM:\n");
				// for (int i=0;i<16*16;i++){
				// 	if (i%16==0){
				// 		printf("\n");
				// 	}
				// 	printf("%d ",*((cab_id->img_last)+i) );
					
					
				// }
				primeira_imagem = 1;
				flag_A =1;
				flag_B =1;
				flag_C =1;
				
				flag_msg = 1;
				contador_i =0;
				contador_j =0;
					
			}
			else{
				// printf("\nCHEGUEI AQUI2");
				uint8_t *buffer = reserve(cab_id);
				memcpy(buffer,img,sizeof(img));
				put_mes(buffer,cab_id);
				flag_A =1;
				flag_B =1;
				flag_C = 1;

				flag_msg =1;
				contador_i =0;
				contador_j =0;
			}

			

		}
		// printf("\n");
		// if(flag_msg ==1){
		// 	uint8_t ack[] = "1"; 
			
		// 	err = uart_tx(uart_dev, ack, sizeof(ack), SYS_FOREVER_MS);
		// 	if (err) {
		// 		printk("uart_tx() error. Error code:%d\n\r",err);
		// 		return;
		// 	}
		// }
		
		for(int i=0;i<5;i++){
			// printf("%d ",cab_id->array[i]);
		}
		
		
    }
    
    
    return;

} 

uint64_t time_taken_A =0;
void thread_A_code(void *argA , void *argB, void *argC)
{
	
    timing_t start_time, end_time;
    uint64_t total_cycles;
    uint64_t total_ns;

    timing_init();
    timing_start();

    
   
    while(1) {
        start_time = timing_counter_get();
        // k_sem_take(&sem_receber_to_A,  K_FOREVER);
		// printf("\nThread A");
		if (flag_A ==1){
			
			imagem = get_mes(cab_id);
			if (primeira_imagem == 1){
				unget(imagem_A,cab_id);
			}
			imagem_A = imagem;
			
			obstaculos_na_CSA = nearObstSearch(imagem);
			
			flag_A = 0;

		}
		
		
		end_time = timing_counter_get();

        
        total_cycles = timing_cycles_get(&start_time, &end_time);
    	total_ns = timing_cycles_to_ns(total_cycles);
		
		if (total_ns >time_taken_A){
			time_taken_A = total_ns;
			printk("\nA took ->%lld (ns)",total_ns);
		}
		k_msleep(1000);
        
    }

	timing_stop();


}
uint64_t time_taken_B =0;
void thread_B_code(void *argA , void *argB, void *argC)
{
    
    timing_t start_time, end_time;
    uint64_t total_cycles;
    uint64_t total_ns;

    timing_init();
    timing_start();

   
    while(1){
		start_time = timing_counter_get();
        // k_sem_take(&sem_ab,  K_FOREVER);
		// printf("\nThread B");
		if (flag_B ==1){
			
			imagem= get_mes(cab_id);
			if (primeira_imagem == 1){
				unget(imagem_B,cab_id);
			}
			imagem_B = imagem;
			num_Obstaculos = obstCount(imagem);
			
			flag_B =0;

			
			
		}
		end_time = timing_counter_get();

        
        total_cycles = timing_cycles_get(&start_time, &end_time);
    	total_ns = timing_cycles_to_ns(total_cycles);
		
		if (total_ns >time_taken_B){
			time_taken_B = total_ns;
			printk("\nB took ->%lld (ns)",total_ns);
		}

		k_msleep(1000);

    }

    timing_stop();
      
  
}
uint64_t time_taken_C =0;
void thread_C_code(void *argA , void *argB, void *argC)
{
	timing_t start_time, end_time;
    uint64_t total_cycles;
    uint64_t total_ns;

    timing_init();
    timing_start();

    while(1){
        start_time = timing_counter_get();
		if (flag_C ==1){
			imagem = get_mes(cab_id);
			if (primeira_imagem == 1){
				unget(imagem_C,cab_id);
			}
			imagem_C = imagem;
			int res = guideLineSearch(imagem, &pos_, &angle);
			
			flag_C =0;
			
			
		}
		end_time = timing_counter_get();

        
        total_cycles = timing_cycles_get(&start_time, &end_time);
    	total_ns = timing_cycles_to_ns(total_cycles);
		
		if (total_ns >time_taken_C){
			time_taken_C = total_ns;
			printk("\nC took ->%lld (ns)",total_ns);
		}
	
		k_msleep(1000);
    }
	timing_stop();
      

  
}
 
uint64_t time_taken_D =0;
void thread_D_code(void *argA , void *argB, void *argC){

	timing_t start_time, end_time;
    uint64_t total_cycles;
    uint64_t total_ns;

    timing_init();
    timing_start();

    while(1){
		start_time = timing_counter_get();
        // k_sem_take(&sem_cd,  K_FOREVER);
		// printf("\nThread D");
        printk("\nTem obstaculos na CSA? Sim=1 / Nao=0 -->%d",obstaculos_na_CSA);
        printk("\nNumero de obstaculos detetados:%d",num_Obstaculos);
		// float aux = ((pos_)/127);
		// int pos_percentagem = aux *100;
        printf("\nRobot position=%d, guideline angle = %f",pos_,angle);
		end_time = timing_counter_get();

        
        total_cycles = timing_cycles_get(&start_time, &end_time);
    	total_ns = timing_cycles_to_ns(total_cycles);
		
		if (total_ns >time_taken_D){
			time_taken_D = total_ns;
			printk("\nD took ->%lld (ns)",total_ns);
		}
	
		k_msleep(1000);
        
    }
	timing_stop();
   
    
}


static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int err;

    switch (evt->type) {
	
        case UART_TX_DONE:
		    // printk("UART_TX_DONE event \n\r");
            break;

    	case UART_TX_ABORTED:
	    	// printk("UART_TX_ABORTED event \n\r");
		    break;
		
	    case UART_RX_RDY:
		    // printk("UART_RX_RDY event \n\r");
            /* Just copy data to a buffer. Usually it is preferable to use e.g. a FIFO to communicate with a task that shall process the messages*/
            memcpy(&rx_chars[uart_rxbuf_nchar],&(rx_buf[evt->data.rx.offset]),evt->data.rx.len); 
			// printk("AQUIII->>>>>%d",evt->data.rx.len);
			for (int i =0;i< 128;i++){	
				// printk("%d ",rx_chars[i]);	
			}
			// printk("\n");
            uart_rxbuf_nchar++;           
		    break;

	    case UART_RX_BUF_REQUEST:
		    // printk("UART_RX_BUF_REQUEST event \n\r");
		    break;

	    case UART_RX_BUF_RELEASED:
		    // printk("UART_RX_BUF_RELEASED event \n\r");
		    break;
		
	    case UART_RX_DISABLED: 
            /* When the RX_BUFF becomes full RX is is disabled automaticaly.  */
            /* It must be re-enabled manually for continuous reception */
            // printk("UART_RX_DISABLED event \n\r");
		    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
            if (err) {
                // printk("uart_rx_enable() error. Error code:%d\n\r",err);
                exit(FATAL_ERR);                
            }
		    break;

	    case UART_RX_STOPPED:
		    // printk("UART_RX_STOPPED event \n\r");
		    break;
		
	    default:
            // printk("UART: unknown event \n\r");
		    break;
    }

}
 

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


/* Function that detects he position and agle of the guideline */
/* Very crude implemenation. Just for illustration purposes */
int guideLineSearch(uint8_t imageBuf[IMGWIDTH][IMGWIDTH], int16_t *pos_, float *angle) {
	int i, gf_pos;

	*pos_=-1;
	gf_pos=-1;

    int flag_mais_que_uma_guideline_gn =0;
	int flag_mais_que_uma_guideline_gf =0;
    // printf("\nNOVA->>>\n");
	// for (int i =0;i< 128;i++){
	// 	for (int j =0;j<128;j++){
	// 		printf("%d ",imageBuf[i][j]);
	// 	}
	// 	printf("\n");
	// }
	
	for(i=0; i < IMGWIDTH; i++) {
		if(imageBuf[GN_ROW][i] == GUIDELINE_COLOR) {
			if (flag_mais_que_uma_guideline_gn ==0){
                *pos_ = i;
                flag_mais_que_uma_guideline_gn =1;
            }
            else if(imageBuf[GN_ROW][i-1] != GUIDELINE_COLOR){
                *pos_ = -2;
				gf_pos = -2;
                break;
            }
            
			// printf("\nI:____%d",i);
			
		}			
	}
	
	
	for(i=0; i < IMGWIDTH; i++) {
		if(imageBuf[GF_ROW][i] == GUIDELINE_COLOR) {
            if (flag_mais_que_uma_guideline_gf ==0){
			    gf_pos = i;
                flag_mais_que_uma_guideline_gf =1;
            }
            else if (imageBuf[GF_ROW][i-1] != GUIDELINE_COLOR){
                gf_pos = -2;
				*pos_  = -2;
                break;
            }
			
			
		}			
	}		
    
	// printk("POSSSSSSSSSS: %d %d",*pos_, gf_pos);
	if(*pos_ == -1 || gf_pos == -1) {
		printf("\nFailed to find guideline pos_=%d, gf_pos=%d", *pos_, gf_pos);
		*pos_ = -1;
		gf_pos = -1;
		return -1;
	}
    if(*pos_ == -2 || gf_pos == -2) {
		printf("\nMore than one guideline detected", *pos_, gf_pos);
		*pos_ = -2;
		gf_pos = -2;
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