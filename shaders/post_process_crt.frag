in vec2 oTexCoord;

out vec4 FragCol;

layout(location = POST_PROCESS_ITIME_UNIFORM_LOCATION)
uniform float iTime;

layout(binding = POST_PROCESS_TEXTURE_BINDING)
uniform sampler2D in_texture;

void main()
{
    vec2 q = oTexCoord;

    vec2 uv = 0.5 + (q - 0.5) * (0.9 + 0.001 * sin(0.2 * iTime));

    vec3 col;

    col.r = texture(in_texture, vec2(uv.x + 0.003, uv.y)).r;
    col.g = texture(in_texture, vec2(uv.x + 0.000, uv.y)).g;
    col.b = texture(in_texture, vec2(uv.x - 0.003, uv.y)).b;

    col = clamp(col * 0.5 + 0.5 * col * col * 1.2, 0.0, 1.0);

    col *= 0.5 + 0.5 * 16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);

    col *= vec3(0.95, 1.05, 0.95);

    col *= 0.9 + 0.1 * sin(10.0 * iTime + uv.y * 1000.0);

    col *= 0.99 + 0.01 * sin(110.0 * iTime);

    FragCol = vec4(col * 1.4, 1.0);
}
