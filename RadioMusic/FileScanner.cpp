#include "FileScanner.h"
#include "AudioFileInfo.h"

#include "RadioMusic.h"

#ifdef DEBUG_FILES
#define D(x) x
#else
#define D(x)
#endif

// File name compare routine for qsort
int fileNameCompare(const void *a, const void *b) {
	AudioFileInfo *sa = (AudioFileInfo *) a;
	AudioFileInfo *sb = (AudioFileInfo *) b;

	return sa->name.compareTo(sb->name);
}

FileScanner::FileScanner() {
}

void FileScanner::sortFiles() {
	for (int i = 0; i < BANKS; i++) {
		if (numFilesInBank[i] > 0) {
			qsort(&(fileInfos[i][0]), numFilesInBank[i], sizeof(AudioFileInfo),
					fileNameCompare);
		}
	}
}

void FileScanner::scan(File* root, Settings& settings) {

	onlyNativeFormat = !settings.anyAudioFiles;

	if(!onlyNativeFormat) {
		maximumFilesPerBank = 32;
	}

	if (SD.exists("config.txt")) {
		D(Serial.println("Scan TipTop"); );
		getExtensionlessFilesInRoot(root);
		// disable looping for now
		// TODO : remove this when looping is fixed for 24 bit
		settings.looping = false;
		settings.hardSwap = true;
		settings.pitchMode = true;
		D(
			Serial.print("Finished Tip Top with "); Serial.print(lastBankIndex); Serial.println(" active banks"););
	} else {
		D(
			if(onlyNativeFormat) {
				Serial.println("Scan Radio Music. 44/16 Only.");
			} else {
				Serial.println("Scan Radio Music. All supported formats.");
			}

		);
		scanDirectory(root);
		D(Serial.println("Scan finished"); );
		if (settings.sort) {
			sortFiles();
			D(showSortedFiles());
		}
	}
}

void FileScanner::showSortedFiles() {
	for (int i = 0; i < BANKS; i++) {
		Serial.print("Bank ");
		Serial.println(i);
		if (numFilesInBank[i] > 0) {
			for(int j=0;j<numFilesInBank[i];j++) {
				Serial.println(fileInfos[i][j].name);
			}
		}
	}
}

void FileScanner::getExtensionlessFilesInRoot(File* root) {
	int directoryNumber = 0;

	while (true) {
		File currentFile = root->openNextFile(O_RDONLY);
		D(Serial.println(currentFile.name()); );
		if (!currentFile) {
			// no more files
			break;
		}
		boolean addFile = false;
		currentFilename = currentFile.name();
		if (!currentFile.isDirectory() && currentFilename.indexOf('.') == -1) {
			// assume its a tip top style wav file
			int numFiles = numFilesInBank[directoryNumber];

			AudioFileInfo& fileInfo = fileInfos[directoryNumber][numFiles];
			addFile = processWavFile(&currentFile, fileInfo);

			if (addFile) {
				fileInfo.dataOffset = 48;
				fileInfo.setBitsPerSample(24);
				fileInfo.size -= 6;
				D(
					if (fileInfo.size % 3 != 0) {
						Serial.print("FS data size not aligned ");
						Serial.println(fileInfo.size % 3);
					}
				);
				fileInfo.name = currentFile.name();
				numFilesInBank[directoryNumber]++;
			}
			if (numFilesInBank[directoryNumber] >= MAX_FILES) {
				directoryNumber++;
				if (directoryNumber >= BANKS) {
					D(Serial.println("Max Files reached"); );
					currentFile.close();
					break;
				} else {
					D(Serial.print("Moved to bank "); Serial.println(directoryNumber););
				}
				if (directoryNumber > lastBankIndex) {
					lastBankIndex = directoryNumber;
				}
			}
		}

		currentFile.close();
	}
}

void FileScanner::scanDirectory(File* dir) {

	D(Serial.print("Scan Dir "); Serial.println(dir->name()); Serial.println(););

	while (true) {

		File currentFile = dir->openNextFile(FILE_READ);
		if (!currentFile) {
			D(Serial.print("No file from "); Serial.print(dir->name()); Serial.print("\t"); Serial.println(currentFile.name()););
			break;
		} else {
			D(Serial.print("Current file is ");Serial.println(currentFile.name()));
		}
		currentFilename = currentFile.name();
		boolean addFile = false;

		if (currentFilename.startsWith("_") != 0) {
			currentFile.close();
			continue;
		}

		if (currentFile.isDirectory()) {
			// Ignore OSX Spotlight and Trash Directories
			if (currentFilename.startsWith("SPOTL") == 0
					&& currentFilename.startsWith("TRASH") == 0) {
				currentDirectory = currentFilename;
				scanDirectory(&currentFile);
			}
		} else {
			for (int i = 0; i < NUM_FILE_TYPES; ++i) {
				if (currentFilename.endsWith(fileTypes[i])) {
					int directoryNumber = currentDirectory.toInt();
					if (directoryNumber >= BANKS) {
						currentFile.close();
						return;
					}
					if (directoryNumber > lastBankIndex) {
						lastBankIndex = directoryNumber;
					}
					int numFiles = numFilesInBank[directoryNumber];

					AudioFileInfo& fileInfo =
							fileInfos[directoryNumber][numFiles];
					// wav / WAV
					if (i == 2 || i == 3) {
						addFile = processWavFile(&currentFile, fileInfo);
					} else {
						addFile = processRawFile(&currentFile, fileInfo);
					}
					if (addFile) {

						fileInfo.name = currentDirectory + "/" + currentFilename;
						numFilesInBank[directoryNumber]++;
						D(
							Serial.print("Adding file ");
							Serial.print(numFilesInBank[directoryNumber]);
							Serial.print(" : ");
							Serial.println(currentFilename);
						);
						if (numFilesInBank[directoryNumber] == MAX_FILES) {
							D(Serial.println("Max Files reached"); );
							currentFile.close();
							return;
						}
					}
				}
			}
		}

		currentFile.close();
	}
}

boolean FileScanner::processRawFile(File* rawFile, AudioFileInfo& fileInfo) {
	D(Serial.print("Raw File "); Serial.println(rawFile->name()); Serial.println(););

	fileInfo.size = rawFile->size();
	fileInfo.setChannels(1);
	fileInfo.setBitsPerSample(16);
	fileInfo.setSampleRate(44100);

	D(Serial.print("Raw file format "); Serial.print(fileInfo.format); Serial.print("\t"); Serial.print(fileInfo.getChannels()); Serial.print("\t"); Serial.print(fileInfo.getBytesPerSample()); Serial.print("\t"); Serial.println(fileInfo.getSampleRate()););
	return true;
}

boolean FileScanner::processWavFile(File* wavFile, AudioFileInfo& fileInfo) {
	D(Serial.print("Process Wav File "); Serial.println(wavFile->name()); Serial.println(););

	if (wavHeaderReader.read(wavFile, fileInfo)) {
		D(
			Serial.print("Size: ");
			Serial.print(fileInfo.size);
			Serial.print("\tSRate: ");
			Serial.print(fileInfo.getSampleRate());
			Serial.print("\tChannels: ");
			Serial.print(fileInfo.getChannels());
			Serial.print("\tBPS: ");
			Serial.print(fileInfo.getBytesPerSample());
			Serial.print("\tOffset: ");
			Serial.println(fileInfo.dataOffset);
		);
		if (onlyNativeFormat) {
			if (fileInfo.getChannels() == 1 && fileInfo.getBytesPerSample() == 2
					&& fileInfo.getSampleRate() == 44100) {
				return true;
			} else {
				return false;
			}
		}
		// We don't handle 96/24 stereo now because its too memory intensive
		if (fileInfo.getChannels() == 2 && fileInfo.getSampleRate() > 90000
				&& fileInfo.getBytesPerSample() == 3) {
			return false;
		}

		// Only mono or stereo.
		if (fileInfo.getChannels() == 1 || fileInfo.getChannels() == 2) {
			return true;
		}
	}

	return false;
}
