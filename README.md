# FileCompression.
File Compression and Decompression program written in C

Uses Huffmancoding to ensure lossless compression.
Capable of compressing/decompressing a single file or recursively traversing directories for all normal files.

### Note: The compression is not actually writing bits but rather the integers '1' and '0'. 
###       This project was made to see how compressing would work. Changing it to write actual bits would not be that different.

# Commands:
- `./fileCompressor <flag> <path or file> |codebook|`
  - Flags:
     - `b` : Builds the codebook
     - `c` : compresses
     - `d` : decompresses
     - `R` : recursive mode
** Note: the `b` flag must be used before the c and d flag. (You must build the codebook of the file or directory before compressing/decompressing it)**
- Examples:
  - `./fileCompressor -R -b ./` will build the codebook for all files/directories in your current project.
  - `./fileCompressor -b file1.txt` will build the codebook for `file1.txt`
  - `./fileCompressor -R -c ./dir1 ./HuffmanCodeBook` will compress all of `dir1` using the HuffmanCodeBook provided.
  - `./fileCompressor -d ./dir1/file3.txt ./HuffmanCodeBook` will decompress `file3.txt` using the HuffmanCodeBook provided.

