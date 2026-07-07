struct GameData
{
      //mouse
      int src_width = 0;
      int src_height = 0;
      float last_x = 0.0f;
      float last_y = 0.0f;
      bool first_mouse = true;
      float mouse_dx = 0.0f;
      float mouse_dy = 0.0f;

      bool cursor_active = false;

      void Init(int in_src_width, int in_src_height)
      {
            UpdateWindow(in_src_width, in_src_height);
            last_x = src_height / 2.0f;
            last_y = src_width / 2.0f;
      }
      void UpdateWindow(int in_src_width, int in_src_height)
      {
            src_width = in_src_width;
            src_height = in_src_height;
      }
};
GameData game_data;

bool full_screen = false;

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

KeyInputs player_input;
