MODULE test_left
LOCAL CONST string YuMi_App_Program_Version:="1.0.1"; !Do not edit or remove this line!
LOCAL VAR robtarget s4 := [[289.242,147.534,135.05],[0.569201,0.734218,0.242229,-0.27975],[-1,0,0,10],[100.002,9E+09,9E+09,9E+09,9E+09,9E+09]];
LOCAL VAR robtarget s3 := [[285.68,169.649,202.705],[0.420195,0.863551,0.0426515,0.275494],[-1,1,0,10],[70.0021,9E+09,9E+09,9E+09,9E+09,9E+09]];
LOCAL VAR robtarget s2 := [[289.292,147.51,135.077],[0.569206,0.734233,0.242208,-0.279718],[-1,0,0,10],[99.9969,9E+09,9E+09,9E+09,9E+09,9E+09]];
LOCAL VAR robtarget s1 := [[285.676,169.696,202.731],[0.420196,0.863527,0.0427507,0.275553],[-1,1,0,10],[69.995,9E+09,9E+09,9E+09,9E+09,9E+09]];
PROC main()
MoveSync s1;
MoveSync s2;
MoveSync s3;
MoveSync s4;
ENDPROC
ENDMODULE