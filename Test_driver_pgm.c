#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>

#define BUFSIZE 4000
/*
* This program uses the device driver veda_char_driver and opens a file on it.
* It performs the read, write and lseek operations apart from open() and close()
* Hence it checks the drivers functionality
*/

int main()
{
    static int fd,i,ret;
    char my_buf[BUFSIZE],temp_buf[10];

    /*Fill my buffer with *'s */
    for(i=0;i<BUFSIZE;i++) my_buf[i] = '*';

    /* open the device for read/write/lseek */
    printf("[%d] - Opening device veda_chr_driver\n", getpid());
    if(fd = open("/dev/veda_cdrv", O_RDWR))
    printf("File opened successfully\n");
    printf("PID [%d]\n", getpid());
    getchar();
    
    /* Write the contents of my buffer into the device */
    ret = write(fd, my_buf, BUFSIZE );
     
    lseek(fd,10,0);
    read(fd,temp_buf,10);
    printf("\n%s\n",temp_buf);
    close(fd);

}
