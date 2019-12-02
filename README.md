# mnemophonix
An audio fingerprinting system


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
