#pragma once

#include "opengl.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ShaderSet
{
      using ShaderHandle = GLuint;
      using ProgramHandle = GLuint;

      struct ShaderNameTypePair
      {
            std::string Name;
            GLenum Type;

            bool operator<(const ShaderNameTypePair& rhs) const
            {
                  return std::tie(Name, Type) < std::tie(rhs.Name, rhs.Type);
            }
      };

      struct Shader
      {
            ShaderHandle Handle = 0;
            uint64_t Timestamp = 0;
            uint32_t SourceID = 0;
      };

      struct Program
      {
            ProgramHandle PublicHandle = 0;
            ProgramHandle InternalHandle = 0;
      };

      static constexpr uint32_t PREAMBLE_SOURCE_ID = 1000000000;

      std::string mVersion;
      std::string mPreamble;

      uint32_t mNextSourceID = PREAMBLE_SOURCE_ID + 1;

      std::map<ShaderNameTypePair, Shader> mShaders;
      std::map<std::vector<const ShaderNameTypePair*>, Program> mPrograms;

public:
      ShaderSet() = default;

      ~ShaderSet();

      void SetVersion(const std::string& version);

      void SetPreamble(const std::string& preamble);

      void SetPreambleFile(const std::string& preambleFilename);

      GLuint* AddProgram(const std::vector<std::pair<std::string, GLenum>>& typedShaders);

      void UpdatePrograms();

      GLuint* AddProgramFromExts(const std::vector<std::string>& shaders);
};
