#include "shaderset.h"

#ifdef _WIN32

#include <Windows.h>

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

static uint64_t GetShaderFileTimestamp(const char* filename)
{
      uint64_t timestamp = 0;

#ifdef _WIN32

      int filename_buffer_size =
            MultiByteToWideChar(
                  CP_UTF8,
                  MB_ERR_INVALID_CHARS,
                  filename,
                  -1,
                  nullptr,
                  0
            );

      if (filename_buffer_size == 0)
      {
            return 0;
      }

      WCHAR* wide_filename = new WCHAR[filename_buffer_size];

      if (MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                filename,
                -1,
                wide_filename,
                filename_buffer_size))
      {
            HANDLE file = CreateFileW(
                  wide_filename,
                  GENERIC_READ,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  nullptr,
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,
                  nullptr
            );

            if (file != INVALID_HANDLE_VALUE)
            {
                  FILETIME last_write_time;

                  if (GetFileTime(file, nullptr, nullptr, &last_write_time))
                  {
                        LARGE_INTEGER large_write_time;
                        large_write_time.HighPart = last_write_time.dwHighDateTime;
                        large_write_time.LowPart = last_write_time.dwLowDateTime;

                        timestamp = large_write_time.QuadPart;
                  }

                  CloseHandle(file);
            }
      }

      delete[] wide_filename;

#else

      struct stat file_stat;

      if (stat(filename, &file_stat) == -1)
      {
            perror(filename);
            return 0;
      }

#if defined(__APPLE__)

      timestamp = static_cast<uint64_t>(file_stat.st_mtimespec.tv_sec);

#elif defined(__linux__)

      timestamp = static_cast<uint64_t>(file_stat.st_mtim.tv_sec);

#else

      timestamp = static_cast<uint64_t>(file_stat.st_mtime);

#endif

#endif

      return timestamp;
}

static std::string ShaderStringFromFile(const char* filename)
{
      std::ifstream file(filename);

      if (!file)
      {
            return "";
      }

      return std::string(
            std::istreambuf_iterator<char>{file},
            std::istreambuf_iterator<char>{}
      );
}

static void ReplaceSourceID(
      std::string& log,
      uint32_t source_id,
      const std::string& source_name)
{
      std::string source_id_string = std::to_string(source_id);
      size_t search_position = 0;

      while ((search_position = log.find(source_id_string, search_position)) != std::string::npos)
      {
            log.replace(
                  search_position,
                  source_id_string.size(),
                  source_name
            );

            search_position += source_name.size();
      }
}

ShaderSet::~ShaderSet()
{
      for (std::pair<const ShaderNameTypePair, Shader>& shader : mShaders)
      {
            glDeleteShader(shader.second.Handle);
      }

      for (std::pair<const std::vector<const ShaderNameTypePair*>, Program>& program : mPrograms)
      {
            glDeleteProgram(program.second.InternalHandle);
      }
}

void ShaderSet::SetVersion(const std::string& version)
{
      mVersion = version;
}

void ShaderSet::SetPreamble(const std::string& preamble)
{
      mPreamble = preamble;
}

GLuint* ShaderSet::AddProgram(
      const std::vector<std::pair<std::string, GLenum>>& typed_shaders)
{
      std::vector<const ShaderNameTypePair*> shader_name_types;

      for (const std::pair<std::string, GLenum>& shader_name_type : typed_shaders)
      {
            ShaderNameTypePair temporary_shader_name_type;

            std::tie(
                  temporary_shader_name_type.Name,
                  temporary_shader_name_type.Type
            ) = shader_name_type;

            auto found_shader =
                  mShaders.emplace(
                        std::move(temporary_shader_name_type),
                        Shader{}
                  ).first;

            if (!found_shader->second.Handle)
            {
                  found_shader->second.Handle =
                        glCreateShader(shader_name_type.second);

                  found_shader->second.SourceID = mNextSourceID++;
            }

            shader_name_types.push_back(&found_shader->first);
      }

      std::sort(
            shader_name_types.begin(),
            shader_name_types.end()
      );

      shader_name_types.erase(
            std::unique(
                  shader_name_types.begin(),
                  shader_name_types.end()
            ),
            shader_name_types.end()
      );

      auto found_program =
            mPrograms.emplace(
                  shader_name_types,
                  Program{}
            ).first;

      if (!found_program->second.InternalHandle)
      {
            found_program->second.PublicHandle = 0;
            found_program->second.InternalHandle = glCreateProgram();

            for (const ShaderNameTypePair* shader : shader_name_types)
            {
                  glAttachShader(
                        found_program->second.InternalHandle,
                        mShaders[*shader].Handle
                  );
            }
      }

      return &found_program->second.PublicHandle;
}

void ShaderSet::UpdatePrograms()
{
      std::set<std::pair<const ShaderNameTypePair, Shader>*> updated_shaders;

      for (std::pair<const ShaderNameTypePair, Shader>& shader : mShaders)
      {
            uint64_t timestamp =
                  GetShaderFileTimestamp(shader.first.Name.c_str());

            if (timestamp > shader.second.Timestamp)
            {
                  shader.second.Timestamp = timestamp;
                  updated_shaders.insert(&shader);
            }
      }

      for (std::pair<const ShaderNameTypePair, Shader>* shader : updated_shaders)
      {
            std::string version =
                  "#version " + mVersion + "\n";

            std::string preamble =
                  "#line 1 " +
                  std::to_string(PREAMBLE_SOURCE_ID) +
                  "\n" +
                  mPreamble +
                  "\n";

            std::string source =
                  "#line 1 " +
                  std::to_string(shader->second.SourceID) +
                  "\n" +
                  ShaderStringFromFile(shader->first.Name.c_str()) +
                  "\n";

            const char* strings[] =
            {
                  version.c_str(),
                  preamble.c_str(),
                  source.c_str()
            };

            GLint lengths[] =
            {
                  static_cast<GLint>(version.length()),
                  static_cast<GLint>(preamble.length()),
                  static_cast<GLint>(source.length())
            };

            glShaderSource(
                  shader->second.Handle,
                  static_cast<GLsizei>(sizeof(strings) / sizeof(*strings)),
                  strings,
                  lengths
            );

            glCompileShader(shader->second.Handle);

            GLint status = GL_FALSE;

            glGetShaderiv(
                  shader->second.Handle,
                  GL_COMPILE_STATUS,
                  &status
            );

            if (!status)
            {
                  GLint log_length = 0;

                  glGetShaderiv(
                        shader->second.Handle,
                        GL_INFO_LOG_LENGTH,
                        &log_length
                  );

                  std::vector<char> log(
                        static_cast<size_t>(log_length) + 1,
                        '\0'
                  );

                  glGetShaderInfoLog(
                        shader->second.Handle,
                        log_length,
                        nullptr,
                        log.data()
                  );

                  std::string log_string = log.data();

                  ReplaceSourceID(
                        log_string,
                        PREAMBLE_SOURCE_ID,
                        "preamble"
                  );

                  ReplaceSourceID(
                        log_string,
                        shader->second.SourceID,
                        shader->first.Name
                  );

                  fprintf(
                        stderr,
                        "Error compiling %s:\n%s\n",
                        shader->first.Name.c_str(),
                        log_string.c_str()
                  );
            }
      }

      for (std::pair<const std::vector<const ShaderNameTypePair*>, Program>& program : mPrograms)
      {
            bool program_needs_relink = false;

            for (const ShaderNameTypePair* program_shader : program.first)
            {
                  for (std::pair<const ShaderNameTypePair, Shader>* shader : updated_shaders)
                  {
                        if (&shader->first == program_shader)
                        {
                              program_needs_relink = true;
                              break;
                        }
                  }

                  if (program_needs_relink)
                  {
                        break;
                  }
            }

            bool can_relink = true;

            if (program_needs_relink)
            {
                  for (const ShaderNameTypePair* program_shader : program.first)
                  {
                        GLint status = GL_FALSE;

                        glGetShaderiv(
                              mShaders[*program_shader].Handle,
                              GL_COMPILE_STATUS,
                              &status
                        );

                        if (!status)
                        {
                              can_relink = false;
                              break;
                        }
                  }
            }

            if (program_needs_relink && can_relink)
            {
                  glLinkProgram(program.second.InternalHandle);

                  GLint log_length = 0;

                  glGetProgramiv(
                        program.second.InternalHandle,
                        GL_INFO_LOG_LENGTH,
                        &log_length
                  );

                  std::vector<char> log(
                        static_cast<size_t>(log_length) + 1,
                        '\0'
                  );

                  glGetProgramInfoLog(
                        program.second.InternalHandle,
                        log_length,
                        nullptr,
                        log.data()
                  );

                  std::string log_string = log.data();

                  ReplaceSourceID(
                        log_string,
                        PREAMBLE_SOURCE_ID,
                        "preamble"
                  );

                  for (const ShaderNameTypePair* shader_in_program : program.first)
                  {
                        ReplaceSourceID(
                              log_string,
                              mShaders[*shader_in_program].SourceID,
                              shader_in_program->Name
                        );
                  }

                  GLint status = GL_FALSE;

                  glGetProgramiv(
                        program.second.InternalHandle,
                        GL_LINK_STATUS,
                        &status
                  );

                  if (!status)
                  {
                        fprintf(stderr, "Error linking");
                  }
                  else
                  {
                        fprintf(stderr, "Successfully linked");
                  }

                  fprintf(stderr, " program (");

                  for (const ShaderNameTypePair* shader : program.first)
                  {
                        if (shader != program.first.front())
                        {
                              fprintf(stderr, ", ");
                        }

                        fprintf(
                              stderr,
                              "%s",
                              shader->Name.c_str()
                        );
                  }

                  fprintf(stderr, ")");

                  if (!log_string.empty())
                  {
                        fprintf(
                              stderr,
                              ":\n%s\n",
                              log_string.c_str()
                        );
                  }
                  else
                  {
                        fprintf(stderr, "\n");
                  }

                  if (!status)
                  {
                        program.second.PublicHandle = 0;
                  }
                  else
                  {
                        program.second.PublicHandle =
                              program.second.InternalHandle;
                  }
            }
      }
}

void ShaderSet::SetPreambleFile(const std::string& preamble_filename)
{
      SetPreamble(
            ShaderStringFromFile(preamble_filename.c_str())
      );
}

GLuint* ShaderSet::AddProgramFromExts(
      const std::vector<std::string>& shaders)
{
      std::vector<std::pair<std::string, GLenum>> typed_shaders;

      for (const std::string& shader : shaders)
      {
            size_t extension_location =
                  shader.find_last_of('.');

            if (extension_location == std::string::npos)
            {
                  return nullptr;
            }

            GLenum shader_type = 0;

            std::string extension =
                  shader.substr(extension_location + 1);

            if (extension == "vert")
            {
                  shader_type = GL_VERTEX_SHADER;
            }
            else if (extension == "frag")
            {
                  shader_type = GL_FRAGMENT_SHADER;
            }
            else if (extension == "geom")
            {
                  shader_type = GL_GEOMETRY_SHADER;
            }
            else if (extension == "tesc")
            {
                  shader_type = GL_TESS_CONTROL_SHADER;
            }
            else if (extension == "tese")
            {
                  shader_type = GL_TESS_EVALUATION_SHADER;
            }
            else if (extension == "comp")
            {
                  shader_type = GL_COMPUTE_SHADER;
            }
            else
            {
                  return nullptr;
            }

            typed_shaders.emplace_back(
                  shader,
                  shader_type
            );
      }

      return AddProgram(typed_shaders);
}
