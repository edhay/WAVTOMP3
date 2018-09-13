wavtomp3 converts all the .wav files to .mp3 files in a given directory

1. The usage of the program is 

LINUX 
./wavtomp3 <Directory with Wav Files>



WAVTOMP3.EXE <Directory with Wav Files>


The above executables are built on Unbuntu (64 bits) and Windows7 (64 bits using VC++)



2. Compiling the project

LINUX

In the parent directory
do a 
make

WINDOWS
create a VC++ project using the file:
WAVTOMP3\VC++ Project\WAVTOMP3.sln


3. Default configurations are as follows

The configuration file is optional and it is in 
D:\github\WAVTOMP3\config\settings.cfg


#Whether to Delete the original wav files after conversion or not
# No : 0 , Yes : 1. Default: 1
DeleteOriginalFiles = 1

#The Number of Threads 
# Range [ 1, 16] , default: 4
NrOfThreadsPerProcesser = 2


#Sampling Rate. Default is 44100
BitSamplingRate = 44100

#Whether to display additional information on the console. 
#By default there will be nothing printed in the console.
# No : 0 , Yes : 1. Default: 0
VerboseMode = 0


