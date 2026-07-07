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
      KeyInput E;
      KeyInput Q;
      KeyInput C;
      KeyInput V;
      
};

struct MouseData
{
      float dx;
      float dy;
      float last_x = 0.0f;
      float last_y = 0.0f;
      bool first_mouse = true;
      int scroll;
      
};

struct PlayerInput
{
      KeyInputs key_inputs;
      MouseData mouse_data;
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

struct GameData
{
      PlayerInput player_input;
};




