mkdir lp

copy area* area.txt

copy interco* interco.txt

python  ..\..\..\src_python\get_names.py > mps.txt

..\\..\\..\build\\RelWithDebInfo\lp_namer.exe .