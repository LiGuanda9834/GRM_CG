\ENCODING=ISO-8859-1
\Problem name: master_ILP.lp

Minimize
 obj1: theta.0
Subject To
 c1: x.0 <= 4
 c2: x.1 <= 4
 c3: x.2 <= 4
 c4: x.3 <= 4
 c5: x.4 <= 4
 c6: x.0 + x.1 + x.2 + x.3 + x.4 <= 8
 c7: - 145.025813447117 x.0 - 137.604294088029 x.1 - 78.5221349108773 x.2
     - 13.08923938908 x.3 - 11.7966403854716 x.4 - theta.0 <= -14.7816437728839
Bounds
 0 <= x.0 <= 4
 0 <= x.1 <= 4
 0 <= x.2 <= 4
 0 <= x.3 <= 4
 0 <= x.4 <= 4
-15.21 <= theta.0 <= 108.79
Generals
 x.0  x.1  x.2  x.3  x.4 
End
