#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>

#include "veda_char_driver.h"

#define BUFSIZE 4000
/*
* This program uses the device driver veda_char_driver and opens a file on it.
* It performs the read, write and lseek operations apart from open() and close()
* Hence it checks the drivers functionality
*/

int main()
{
    static int fd,i;
    char my_buf[BUFSIZE];
    
    unsigned int size;
    char c;

    /*Fill my buffer with *'s */
    for(i=0;i<BUFSIZE;i++) my_buf[i] = '*';

    /* open the device for read/write/lseek */
    printf("[%d] - Opening device /dev/veda_cdrv \n", getpid());
    if( fd = open("/dev/veda_cdrv", O_RDWR) )
        printf("File opened successfully with id [%d]\n",fd);
    else {
        printf("\n**********\nDevice could not be opened \n**********\n");
        return 1;
    }
    printf("PID [%d]\n", getpid());
    
    /* Write the contents of my buffer into the device */
    write(fd, my_buf, BUFSIZE );

    /* empty my buffer now */
    bzero(my_buf, BUFSIZE);
    
    /* read 70 Characters from 20th character */ 
    lseek(fd,20,SEEK_SET);
    read(fd, my_buf, 70);
    printf("I read this from the device\n%s\n",my_buf);
    
    /*Empty my buffer again */
    bzero(my_buf, BUFSIZE);

    /*write something into the buffer */
    write(fd, "Veda_Solutions",14);

    /* Set the internal read/write pointer to the 60th character */
    lseek(fd, 60, SEEK_SET);

    /* read 70 characters from there and print it */
    read(fd, my_buf, 70);
    printf("I read this from the device \n%s\n", my_buf);

    /* These were the operations that were working using 
        char_driver.c and char_driver_ioctl.c also */

    /* Test cases for ioctl */ 
    lseek( fd, 0, SEEK_SET ); 
    /* Lets first resize our character buffer to 4096(4K) characters */ 
    ioctl( fd, VEDA_GET_SIZE, &size ); 
    printf("Present size of the buffer is %d\n", size); 
    size = 4096; 
    printf("Setting size of buffer to 4096\n"); 
    if( ioctl( fd, VEDA_SET_SIZE, &size ) < 0 ) 
        printf("ioctl failed\n"); 
    ioctl( fd, VEDA_GET_SIZE, &size ); 
    printf("New size of the buffer is %d\n", size); 

    /* lets fill the buffer with character '+' */
    bzero( my_buf, 4000 ); 
    read( fd, my_buf, 70 );
    printf("Current contents\n%s\nFilling +'s\n", my_buf); 
    c= '+'
    ioctl(fd, VEDA_FILL_CHAR, &c); 
    bzero( my_buf, 4000 ); 
    read( fd, my_buf, 70 ); 
    printf("New contents\n%s\n", my_buf);

    /* Close the device */ 
    close(fd); 
}
