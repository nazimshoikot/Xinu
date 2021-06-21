/* printer.c - printer */

#include <xinu.h>

/*------------------------------------------------------------------------
 * type -  print a character provided by the user putc
 *------------------------------------------------------------------------
 */
void	type(
	  char		ch		/* Character to print		*/
	)
{   
    // busy waiting
	while (go == 0) {;}

	while (1) {
        putc(stdout, ch);

    }
}

/*------------------------------------------------------------------------
 * kerneltype -  print a character provided by the user kputc
 *------------------------------------------------------------------------
 */
void	kerneltype(
	  char		ch		/* Character to print		*/
	)
{
    // busy waiting
    while (go == 0) {;}
    
    while (1) {
        kputc(ch);
    }
	
}


/*------------------------------------------------------------------------
 * print -  print a character provided by the user uprintf
 *------------------------------------------------------------------------
 */
void	print(
	  char		ch		/* Character to print		*/
	)
{
    // busy waiting
    while (go == 0) {;}
    
    // create the string
    char str[50];

    int i = 0;
    for (i = 0; i < 50; i++) {
        str[i] = ch;
    }

    while (1) {

        uprintf(str);
    }
	
}

/*------------------------------------------------------------------------
 * kernelprint -  print a character provided by the user kprintf
 *------------------------------------------------------------------------
 */
void	kernelprint(
	  char		ch		/* Character to print		*/
	)
{
    // busy waiting
    while (go == 0) {;}
    
    // create the string
    char str[50];
    int i = 0;
    for (i = 0; i < 50; i++) {
        str[i] = ch;
    }

    while (1) {
        kprintf(str);
    }
}
