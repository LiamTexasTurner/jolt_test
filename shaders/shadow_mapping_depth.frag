void main()
{
    float depth = gl_FragCoord.z;

    float bias =
        fwidth(depth) * (1.0 + 9);

    gl_FragDepth =
        min(depth + bias, 1.0);
}
