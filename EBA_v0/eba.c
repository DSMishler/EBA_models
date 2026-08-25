#include "eba_common.h"

int main(void)
{
   check_eba_assumptions();

   op_loader_t *opl1 = opl_init("./eba_boot/boot.so", "boot");

   void *my_eba_arg = init_eba_arg(1);
   set_eba_arg(my_eba_arg, 0, opl1);

   eba_op(my_eba_arg); // boot!

   // scaffold code to free the code we allocated so valgrind is happy.
   // of course, in production, this wouldn't be needed. It would be freed
   // when the OS shuts down
   dlclose(opl1->handler);
   free(opl1);
}
