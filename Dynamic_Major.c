#include<linux/kernel.h>
#include<linux/modules.h>

/*****************************************************************************
 * This function is called when a user wants to use this device and has
   called the open function.

 * The function will keep a count of how many people tried to open it and
   increments it each time this function is called

 * The function prints out two pieces of information
   1. Number of times open() was called on this device
   2. Number of processes accessing this device right now

 * Return value
        Always returns SUCCESS
*****************************************************************************/
static int char_dev_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    counter++;
    printk(KERN_INFO"Number of times open() was called : %d\n",counter);
    printk(KERN_INFO"MAJOR Number : %d, MINOR Number : %d\n",major(inode),minor(inode));
    printk(KERN_INFO"process id of the current process :%d\n",current->pid);
    printk(KERN_INFO"ref = %d\n", module_refcount(THIS_MODULE));
    return SUCCESS;
}

/*****************************************************************************
This function is called when the user program uses close() function

The function decrements the number of processes currently using this device.
This should be done because if there are no users of a driver for a long time,
the kernel will unload . the driver from the memory.

Return value :
        Always returns SUCCESS
*****************************************************************************/
static int char_dev_release(struct inode *inode, struct file *file)
{
    return SUCCESS;
}
/*****************************************************************************
This function is called when the user calls reed on this device
It reads from a 'file' some data into 'buf' which is 'lbuf'
long starting from 'ppos' (present position)

Understanding the parameters
    • buf = buffer
    * ppos = present position
    * lbuf = length of the buffer
    * file = file to read

The function returns the number of bytes(characters)read.
*****************************************************************************/

static size_t char_dev_read(struct file *file, char *buf, size_t lbuf, loff_t *ppos)
{
    int maxbytes;   /* Number of bytes from ppos to MAX_LENGTH */
    int bytes_to_do;    /*number of bytes to read*/
    int nbytes;     /* Number of bytes actually read */

    maxbytes = MAX
}
