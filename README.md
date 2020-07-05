# midi-cube

A MIDI-Workstation written in C++. Inendend to both work on Single-Chip-Devices with a touch display (Raspberry Pi) and Computers.

## Planned features:
* Serveral integrated virtual instruments like a B3 organ, an E-Piano-synthesizer, a sampler, drumkits, a subtractive/additive/FM-synthesizer, etc.
* MIDI-Routing between devices
* Looper, Arpeggiator, Sequencer/Drum Machine
* A touch responsive GUI
* A web interface or a desktop client that allows uploading new sounds

## Used libraries
* JACK Audio Connection Kit API licensed under LGPL (https://jackaudio.org/api/ https://www.gnu.org/licenses/lgpl-3.0.html)
* RtMidi licensed under the RtMidi license which is pretty similar to the MIt license (https://github.com/thestk/rtmidi https://github.com/thestk/rtmidi/blob/master/LICENSE)
* libsndfile licensed under LGPL (https://github.com/erikd/libsndfile/commits/master https://github.com/erikd/libsndfile/blob/master/COPYING)

## Used ressources
* Format of .wav files from the german Wikipedia (https://de.wikipedia.org/wiki/RIFF_WAVE)
* Article about the Hammond-Organ from the german Wikiedia (https://de.wikipedia.org/wiki/Hammondorgel)
* Modulation and Delay Line based Digital Audio Effects by Disch Sascha and Zöler Udo (2002); helped me implement the rotary speaker (https://www.researchgate.net/publication/2830823_Modulation_And_Delay_Line_Based_Digital_Audio_Effects)
* Speed of the rotary speakers (https://www.musiker-board.de/threads/leslie-geschwindigkeiten-in-herz-frequenzen-fuer-die-vb3-orgel-gesucht.511349/)

