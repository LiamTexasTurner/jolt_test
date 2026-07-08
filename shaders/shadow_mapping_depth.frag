void main()
{
    float depth = gl_FragCoord.z;

    float bias =
        fwidth(depth) * (1.0 + 1);

    gl_FragDepth =
        min(depth + bias, 1.0);
}
