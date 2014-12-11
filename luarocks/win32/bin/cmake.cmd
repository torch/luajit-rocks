if %1 == -E  (
cmake.exe  %* 
) else (
cmake.exe -G "NMake Makefiles"  -DWIN32=1  %*
)
		