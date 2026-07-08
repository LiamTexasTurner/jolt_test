void main()
{
   
      float depth =
            gl_FragCoord.z;

      float bias =
            fwidth(depth) * 4;

      gl_FragDepth =
            min(depth + bias, 1.0);
}
