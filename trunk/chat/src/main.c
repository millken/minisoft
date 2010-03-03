#include "i32.h"
#include "ctrls.h"

void create_form ();

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_ctrls();

	//create_form();
	create_mainform();

	i32loop();
	return 0;
}
