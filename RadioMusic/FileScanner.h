#ifndef FileScanner_h
#define FileScanner_h

#include <SD.h>
#include "AudioFileInfo.h"
#include "WavHeaderReader.h"
#include "Settings.h"

#define BANKS 16
#define MAX_FILES 48

// .raw and .wav but both lower and upper case
#define NUM_FILE_TYPES 4

class FileScanner {
	public:
		FileScanner();
		void scan(File* root, Settings& settings);

		int activeBanks = 0;
		AudioFileInfo fileInfos[BANKS][MAX_FILES];
		int numFilesInBank[BANKS];

	private:
		void scanDirectory(File* dir);
		void sortFiles();
		void showSortedFiles();

		void getExtensionlessFilesInRoot(File* root);
		boolean processWavFile(File* wavFile, AudioFileInfo& fileInfo);
		boolean processRawFile(File* wavFile, AudioFileInfo& fileInfo);
		String fileTypes[4] = {
				"RAW", "raw", "WAV", "wav"
		};
		String currentDirectory = "0";

		WavHeaderReader wavHeaderReader;
		// If true only scan for 44k, 16bit mono files.
		boolean onlyNativeFormat = false;

		String currentFilename;
};

#endif
