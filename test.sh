make clean
rm -rf 1kb-fs
rm -rf fileSystemOper makeFileSystem

make all
./makeFileSystem 1 1kb-fs
./fileSystemOper 1kb-fs mkdir "/newdir"
./fileSystemOper 1kb-fs mkdir "/newdir/subdir"
#./fileSystemOper 1kb-fs