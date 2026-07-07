#pragma once
struct KeyInput
{
      bool is_down = false;
      bool pressed = false;
      bool released = false;
};

struct KeyInputs
{
      KeyInput rmb;
      KeyInput lmb;
      KeyInput tab;
      KeyInput space;
      KeyInput shift;
      KeyInput ctrl;
      

      KeyInput W;
      KeyInput A;
      KeyInput S;
      KeyInput D;
      KeyInput C;
      KeyInput V;
      
};

inline void press_key(KeyInput &key)
{
      key.released = false;
      key.pressed = false;
      if(key.is_down == false)
      {
            key.pressed = true;
      };          
      key.is_down = true;
};
inline void release_key(KeyInput &key)
{
      key.is_down ? key.released = true : key.released = false;
      key.pressed = false;
      key.is_down = false;
      
}


