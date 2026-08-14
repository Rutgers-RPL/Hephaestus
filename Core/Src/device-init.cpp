/**
 * 
 */

#include "device-init.h"
#include "main.h"

class TEST_CLASS
{
public:
  TEST_CLASS() = default;

private:
  bool var;
};

void device_init(void)
{
  TEST_CLASS thing;
}
