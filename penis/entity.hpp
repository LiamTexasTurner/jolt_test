#pragma once
#include <stdint.h>

struct Entity
{
      uint32_t instance_ID;

      Entity(uint32_t in_instance_ID) :
            instance_ID(in_instance_ID)
      {
            
      }
};
