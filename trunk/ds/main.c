#include <stdio.h>
#include <stdlib.h>

#include "ds.h"

int main()
{
	glnode* root = glnew(NULL);
	glnode* p;
	int i;


	for (i = 0; i < 100; i++)
		gljoin(root, glnew(i), -1);

	gl_foreach (root, p)
		printf ("%d, ", p->data);

    return 0;
}
