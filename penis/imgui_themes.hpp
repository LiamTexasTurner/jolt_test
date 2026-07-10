#include "imgui.h"
void SetDarkTheme()
{
      auto& colors = ImGui::GetStyle().Colors;
      colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

      colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
      colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
      colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};

      colors[ImGuiCol_Button] = ImVec4{0.2f, 0.2f, 0.2f, 1.0f};
      colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.3f, 0.3f, 1.0f};
      colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};

      colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.2f, 0.2f, 1.0f};
      colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.3f, 0.3f, 1.0f};
      colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};

      colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.38f, 0.38f, 1.0f};
      colors[ImGuiCol_TabActive] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TabUnfocused] = ImVec4{0.2f, 0.2f, 0.2f, 1.0f};

      // Docking/unfocused tab colors
      colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.20f, 0.20f, 0.20f, 1.0f};

      colors[ImGuiCol_Border] = ImVec4{0.18f, 0.18f, 0.18f, 1.0f};
      colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

      colors[ImGuiCol_DockingPreview] = ImVec4{0.35f, 0.35f, 0.35f, 0.7f};
      colors[ImGuiCol_DockingEmptyBg] = ImVec4{0.10f, 0.105f, 0.11f, 1.0f};

      colors[ImGuiCol_TabSelectedOverline] = ImVec4{0.28f, 0.28f, 0.28f, 1.0f};
      colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4{0.20f, 0.20f, 0.20f, 1.0f};

      colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.15f, 0.15f, 1.0f};
      colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.9f, 0.15f, 0.15f, 1.0f};

      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

      // ImGuiStyle& style = ImGui::GetStyle();

      // style.Colors[ImGuiCol_Border]       = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
      // style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

      // style.WindowBorderSize = 10.0f;
      
      
}
void DrawTopBar(float height)
{
      ImGuiViewport* viewport = ImGui::GetMainViewport();

      float TopBarHeight = height;

      ImVec2 pos = viewport->WorkPos;

      ImVec2 size = viewport->WorkSize;
      size.y = TopBarHeight;

      ImGui::SetNextWindowPos(pos);
      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.15f, 0.15f, 0.15f, 1.0f});

      ImGui::Begin("TopBar", nullptr, flags);

      ImGui::PushFont(nullptr, 25.0f);
      ImGui::Text("AnimGraph");
      ImGui::PopFont();
      ImGui::SameLine();

      ImGui::PopStyleColor();
      

      ImGui::End();
}
void DrawBottomBar(float height, float block_time)
{
      ImGuiViewport* viewport = ImGui::GetMainViewport();

      float BottomBarHeight = height + 10;

      ImVec2 pos = viewport->WorkPos;
      pos.y += viewport->WorkSize.y - BottomBarHeight;

      ImVec2 size = viewport->WorkSize;
      size.y = BottomBarHeight;

      ImGui::SetNextWindowPos(pos);
      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.15f, 0.15f, 0.15f, 1.0f});

      ImGui::Begin("BottomBar", nullptr, flags);

      ImGui::PushFont(nullptr, 20.0f);
      ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
      ImGui::SameLine();
      ImGui::Text(" %.3f", 1000.0f / ImGui::GetIO().Framerate);

      // ImGui::Dummy(ImVec2(100, 0));

      ImGui::SameLine();

      ImGui::Text(" %f", block_time);
      
      ImGui::PopFont();

      ImGui::PopStyleColor();
      

      ImGui::End();
}
