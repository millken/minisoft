#include "i32.h"
#include "ctrls.h"
#include "wins.h"

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_ctrls();

	create_mainform();

	return i32loop();
}
