    /* With this program it will automatically create mknod file /dev/veda_cdrv with functionality of udev */

#include<linux/module.h>
#include<linux/fs.h>
#include<asm/uaccess.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/sched.h>
#include<linux/errno.h>
#include<asm/current.h>
#include<linux/device.h>
#include<linux/slab.h>
#include "veda_char_device.h"	/*Including ioctl related macros*/

#define MAX_LENGTH length
#define CHAR_DEV_NAME "veda_cdrv"
#define SUCCESS 0


static char *char_device_buf;
struct cdev *veda_cdev;
static unsigned int length = 4000;
dev_t mydev;
int count = 1;
static struct class *veda_class;

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
    printk(KERN_INFO "Number of times open() was called : %d\n",counter);
    printk(KERN_INFO "MAJOR Number : %d, MINOR Number : %d\n",imajor(inode),iminor(inode));
    printk(KERN_INFO "process id of the current process :%d\n",current->pid);
    printk(KERN_INFO "ref = %d\n", module_refcount(THIS_MODULE));
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
    printk(KERN_INFO "Close() function invoked\n");
    return SUCCESS;
}


/*****************************************************************************
This function is called when the user calls reed on this device
It reads from a 'file' some data into 'buf' which is 'lbuf'
long starting from 'ppos' (present position)

Understanding the parameters
    * buf = buffer
    * ppos = present position
    * lbuf = length of the buffer
    * file = file to read

The function returns the number of bytes(characters)read.
*****************************************************************************/

static ssize_t char_dev_read(struct file *file, char *buf, 
							 size_t lbuf, loff_t *ppos)
{
    int maxbytes;   /* Number of bytes from ppos to MAX_LENGTH */
    int bytes_to_do;    /*number of bytes to read*/
    int nbytes;     /* Number of bytes actually read */

   maxbytes = MAX_LENGTH - *ppos;
   if(maxbytes > lbuf) bytes_to_do = lbuf;
   else bytes_to_do = maxbytes;
   
   if(bytes_to_do == 0)
   {
      printk("Reached end of device\n");
      return -ENOSPC; /* Causes read() to return EOF*/
   }
   
   nbytes = bytes_to_do - copy_to_user(buf, /* to */
                                       char_device_buf + *ppos, /* from*/
                                       bytes_to_do);   /* How many bytes*/
   
   *ppos += nbytes;
   printk(KERN_INFO "read() function invoked\n");
   return nbytes;
}


/*****************************************************************************
This function is called when the user calls write on this device it writes 
into 'file' the contents of 'but' starting from 'ppos' up to 'lbuf' bytes.

understanding the parameters 
   buf = buffer
   file = file to write into   
   lbuf = length of the buffer
   ppos = present position pointer
   
The function returs the number of characters(bytes) written
*****************************************************************************/

static ssize_t char_dev_write(struct file *file, const char *buf, 
							  size_t lbuf, loff_t *ppos)
{
   int nbytes; /* Number of bytes written */ 
   int bytes_to_do; /* Number of bytes to write */ 
   int maxbytes;  /* Maximum number of bytes that can be written */ 
   
   maxbytes = MAX_LENGTH - *ppos;
   if( maxbytes > lbuf ) bytes_to_do = lbuf;
   else bytes_to_do = maxbytes;
   
   if( bytes_to_do == 0 )
   {
      printk("Reached end of device\n");
      return -ENOSPC; /* Returns EOF at write() */
   }
   
   nbytes = bytes_to_do - copy_from_user(char_device_buf + *ppos, /* to*/
                                         buf, /* from*/
                                         bytes_to_do); /*how many bytes*/ 
   *ppos += nbytes;
   printk(KERN_INFO"write() function invoked\n");
   return nbytes; 
}


/*****************************************************************************
 This function is called when lseek() is called on the device. The function 
 should place the ppos pointer of 'file' at an offset of 'offset' from 'orig'
 
 * if orig = SEEK_SET
      ppos = offset
 * if orig = SEEK_END
      ppos = MAX_LENGTH + offset     
 * if orig = SEEK_CUR
      ppos += offset       
 
 returns the new position
*****************************************************************************/
static loff_t char_dev_lseek(struct file *file, loff_t offset, int orig)
{
   loff_t testpos;
   switch(orig)
   {
      case 0: /* SEEK_SET */
         testpos = offset;
         break;
      case 1: /*SEEK_CUR*/
         testpos = file->f_pos + offset;
         break; 
      case 2: /*SEEK END */
         testpos = MAX_LENGTH + offset;
         break;
      default: 
         return -EINVAL;
   }
   testpos = testpos < MAX_LENGTH ? testpos : MAX_LENGTH;
   testpos = testpos >= 0 ? testpos : 0;
   file->f_pos = testpos;
   printk(KERN_INFO "Seeking to pos = %ld\n", (long)testpos);
   return testpos;
}
          
static int  char_dev_ioctl(struct inode *inode, struct file *filp,
						   unsigned int cmd, unsigned long arg)
{
/*	step 1: Verify args */
	unsigned int i, size;
	char *new_buf;
	char c;
	int retbytes;

	if( _IOC_TYPE(cmd) != VEDA_MAGIC ) return -ENOTTY;
	if( _IOC_NR(cmd) > VEDA_MAX_CMDS ) return -ENOTTY;

	if( _IOC_DIR(cmd) & _IOC_READ )
		if( !access_ok( VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd) ) )
			return -EFAULT;
	if( _IOC_DIR(cmd) & _IOC_WRITE )
		if( !access_ok( VERIFY_READ, (void *)arg, _IOC_SIZE(cmd) ) )
			return -EFAULT;

/*	Step 2 : implement support of commands using switch case */
	switch(cmd) 
	{
		case VEDA_FILL_ZERO:
				for(i=0; i<MAX_LENGTH; i++) char_device_buf[i] = 0;
				break;

		case VEDA_FILL_CHAR:
				retbytes = copy_from_user( &c, (char *)arg, sizeof(char) );
				for(i=0; i<MAX_LENGTH; i++) char_device_buf[i] = c;
				break;

		case VEDA_SET_SIZE:
				retbytes = copy_from_user( &size, (unsigned int*)arg,
										   sizeof(unsigned int) );
				new_buf = (char *)kmalloc( size*sizeof(char), GFP_KERNEL );
				if( !new_buf ) return -ENOSPC;
				kfree( char_device_buf );
				char_device_buf = (char*)new_buf;
				MAX_LENGTH = size;
				for(i=0; i<MAX_LENGTH; i++) char_device_buf[i] = 0;
				filp->f_pos = 0;	/*why this statement? */
				break;

		case VEDA_GET_SIZE:
				size = MAX_LENGTH;
				retbytes = copy_to_user( (unsigned int*)arg, &size, Sizeof(unsigned int) );
				break;
	}

	return SUCCESS;
}  

static struct file_operations char_dev_fops = {
   .owner = THIS_MODULE,
   .ioctl = char_dev_ioctl,
   .read = char_dev_read,
   .write = char_dev_write,
   .open = char_dev_open,
   .release= char_dev_release,
   .llseek = char_dev_lseek
};
          
/*****************************************************************************
Init module
*****************************************************************************/
static __init int char_dev_init(void)
{
   int ret;
   
   if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) <0 )
   {
      printk (KERN_ERR "failed to reserve major/minor range\n");
      return -1; 
   }
   
   if (!(veda_cdev  = cdev_alloc()))
   {
      printk(KERN_ERR "cdev_alloc() failed\n");
      unregister_chrdev_region(mydev, count);
      return -1;
   }
   cdev_init(veda_cdev,&char_dev_fops); 
   
   ret=cdev_add(veda_cdev,mydev,count);
   if( ret < 0 )
   {
      printk(KERN_INFO"Error registering device drive\n");
      cdev_del(veda_cdev);
      unregister_chrdev_region(mydev, count);
      return -1;
   }

   /*creating new class of devices a VIRTUAL Class 
     and creating device with that virtual class along with major,minor number*/
   veda_class = class_create (THIS_MODULE, "VIRTUAL");
   device_create (veda_class, NULL, mydev, NULL, "%s", "veda_cdrv");

   printk(KERN_INFO "\nDevice Registered: %s\n", CHAR_DEV_NAME);  
   printk(KERN_INFO "Major number= %d, Minor number = %d\n", MAJOR(mydev), MINOR(mydev));
   
   char_device_buf =(char *)kmalloc(MAX_LENGTH,GFP_KERNEL); 
   return 0;
}
          
/*****************************************************************************
Exit module
*****************************************************************************/
static __exit void char_dev_exit(void)
{
   device_destroy(veda_class,mydev);
   class_destroy(veda_class);
   cdev_del(veda_cdev);
   unregister_chrdev_region(mydev,1);
   kfree(char_device_buf);
   printk(KERN_INFO"\nDriver Unregistered\n");
}
          
module_init(char_dev_init);
module_exit(char_dev_exit);
          
MODULE_AUTHOR("VEDA");
MODULE_DESCRIPTION("CHARACTER DEVICE DRIVER - TEST");
MODULE_LICENSE("GPL");          
             

        
