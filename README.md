# mnemophonix
## A simple audio fingerprinting system

This project was inspired by the article https://www.codeproject.com/Articles/206507/Duplicates-detector-via-audio-fingerprinting
by Sergiu Ciumac that explains how to build a Shazam-like system that can index audio files and then, given an
audio file, try to identify it against the database previously built.

The work done here is a simplified version in C of this work. It is built from scratch without any dependency, since
the main goal was to learn in details how audio fingerprinting works. It has lots of comments and it is only
moderately optimized with multithreading to make the fingerprinting not too slow, but it tries to stay
easy to understand. There is no attempt at storing the signatures in an optimized way, so if you want to
use it at large scale, you will probably need to customize the I/O.

The canonical format that the program can process is 44100Hz 16-bit PCM. However,
for any input file that does not look like this, the program will attempt a conversion
on-the-fly with the best Swiss army knife media tool: ```ffmpeg```, so any file
with audio (including videos) can be fingerprinted.

For the record, fingerprinting the 130 songs from DEFCON 20 to DEFCON 27 generates a 75Mb database. Fingerprinting
a 2 hour movie produces a 16Mb signature and takes about 55 seconds on a MacBook Pro (including extracting the audio
from the movie with ```ffmpeg```). Once the database is loaded in memory, searching for an audio sample of a few
seconds is almost instantaneous.

## How to build it (Linux & MacOS)

Run ```make```.

## How to use it

In order to index a file and store the generated signature into a file, run this:

```
$ mnemophonix index test.wav > test.signature
```

The signatures are plain text, so you can create your database by just concatenating
signatures into one big file like this:

```
$ mnemophonix index song1.wav > db
$ mnemophonix index song2.mp3 >> db
$ mnemophonix index movie.mp4 >> db
```

Then, in order to identify a sample against the database, run this:

```
$ mnemophonix search sample.wav db
Reading 44100Hz samples...
Resampling to 5512Hz...
Normalizing samples...
31444 5512Hz mono samples
Got 42 spectral images
Applying Haar transform to spectral images
Building raw fingerprints
Generated 42 signatures
Loading database db...
(raw database loading took 2122 ms)
(lsh index building took 964 ms)
Searching...
(Search took 32 ms)

Found match: 'defcon 26/01 - Skittish & Bus - OTP.mp3'
Artist: Skittish & Bus
Track title: OTP
Album title: DEF CON 26: The Official Soundtrack
```

## Cool, but it would be even cooler to guess straight from the microphone...
...which is why there is a companion program for MacOS, written in Objective C.
You can build it with ```xcodebuild``` and then run it with your database, which
will print on the standard output any identified audio entry:

```
$ ./build/Release/ears db
 ---                                                       ---
 \  \  mnemophonix - A simple audio fingerprinting system  \  \
 O  O                                                      O  O
Loading 'db'...
Database loaded...
Starting to listen...

Found match: 'defcon 20/13 - High Sage feat. Katy Rokit - Stu.mp3'
Artist: Highsage feat. Katy Rokit
Track title: High Sage feat. Katy Rokit - Stuck on Ceazar's Challenge (KEW QEIMYUK QEIMYUK QEIM AYM)
Album title: DEF CON XX Commemorative Compilation


Found match: 'defcon 24/16 Mindex - Jazzy Mood (Crystal Mix).mp3'
Artist: Mindex
Track title: Jazzy Mood (Crystal Mix)
Album title: DEF CON 24: The Original Soundtrack
```

## Credits

This work used the following sources:

* https://www.codeproject.com/Articles/206507/Duplicates-detector-via-audio-fingerprinting: the original project this work replicates
* https://web.archive.org/web/20080113195252/http://www.borg.com/~jglatt/tech/wave.htm: description of the wav file format
* https://web.archive.org/web/20080114200405/http://www.borg.com/~jglatt/tech/aboutiff.htm: explanations about the chunk
  format that the RIFF wav format uses
* https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex and https://github.com/tpn/winddk-8.1/blob/master/Include/shared/mmreg.h for details about possible wave formats
* https://www.recordingblogs.com/wiki/list-chunk-of-a-wave-file: description of the metadata that can be found in a wave file
* https://introcs.cs.princeton.edu/java/97data/InplaceFFT.java.html: in place fast Fourier transform
* https://www.eecis.udel.edu/~amer/CISC651/wavelets_for_computer_graphics_Stollnitz.pdf: explanations about Haar wavelets
* https://medium.com/engineering-brainly/locality-sensitive-hashing-explained-304eb39291e4: explains how MinHash works
* https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/34409.pdf: explains how to generate good permutations for the MinHash algorithm
* https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter: explanations about low pass filters
* https://en.wikipedia.org/wiki/Spectral_leakage: explanations about spectral leakage
* The DEFCON songs used for good mood and testing:
  * https://media.defcon.org/DEF%20CON%2020/DEF%20CON%2020%20music/DEF%20CON%2020%20Music%20CD/
  * https://media.defcon.org/DEF%20CON%2021/DEF%20CON%2021%20music/DEF%20CON%2021%20Music%20CD%20-%20aac%20224k/
  * https://media.defcon.org/DEF%20CON%2022/DEF%20CON%2022%20music/DEF%20CON%2022%20music%20CD/DEF%20CON%2022%20-%20The%20Official%20Soundtrack%20-%20aac%20224k/
  * https://media.defcon.org/DEF%20CON%2023/DEF%20CON%2023%20music/DEF%20CON%2023%20music%20CD/DEF%20CON%2023%20music%20CD%20-%20aac%20224k/
  * https://media.defcon.org/DEF%20CON%2024/DEF%20CON%2024%20music/DEF%20CON%2024%20music%20CD/DEF%20CON%2024%20Official%20Soundtrack%20-%20m4a/
  * https://media.defcon.org/DEF%20CON%2025/DEF%20CON%2025%20music/DEF%20CON%2025%20Music%20CD%20-%20aac%20224k/
  * https://media.defcon.org/DEF%20CON%2026/DEF%20CON%2026%20music/DEF%20CON%2026%20Music%20CD%20-%20mp3%20256k/
  * https://media.defcon.org/DEF%20CON%2027/DEF%20CON%2027%20music/DEF%20CON%2027%20OST%20-%20AAC%20VBR%20224k/
