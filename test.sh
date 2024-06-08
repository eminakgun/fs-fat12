make clean
rm -rf 1kb-fs
rm -rf fileSystemOper makeFileSystem

make all
./makeFileSystem 1 1kb-fs

./fileSystemOper 1kb-fs mkdir "/usr"
./fileSystemOper 1kb-fs mkdir "/usr/ysa"
./fileSystemOper 1kb-fs mkdir "/bin/ysa"

./fileSystemOper 1kb-fs write "/usr/ysa/file1 test_file.data"
./fileSystemOper 1kb-fs write "/usr/file2 test_file.data"
./fileSystemOper 1kb-fs write "/file3 test_file.data"

./fileSystemOper 1kb-fs dir "/"
./fileSystemOper 1kb-fs dir "/usr"
./fileSystemOper 1kb-fs dir "/usr/ysa"
./fileSystemOper 1kb-fs write "/usr/ysa/file1 test_file.txt"
./fileSystemOper 1kb-fs read "/usr/ysa/file1 read_file.txt"

./fileSystemOper 1kb-fs chmod "/usr/ysa/file1 -rw"
./fileSystemOper 1kb-fs read "/usr/ysa/file1 read_file.txt" # fails due to permissions
./fileSystemOper 1kb-fs chmod "/usr/ysa/file1 +rw"
./fileSystemOper 1kb-fs read "/usr/ysa/file1 read_file.txt" #succeeds
./fileSystemOper 1kb-fs dumpe2fs
#./fileSystemOper 1kb-fs