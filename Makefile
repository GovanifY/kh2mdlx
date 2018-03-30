WCXX = i686-w64-mingw32.static-g++
CXX = g++
linux:
	$(CC) -std=c99  dae2mdlx.c -o dae2mdlx
clean:
	rm -rf *.kh2v *.kh2m *.o *.dsm *.exe dae2mdlx
windows:
	$(WCC) -std=c99  dae2mdlx.c -o dae2mdlx.exe

