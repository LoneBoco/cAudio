#include "../Headers/cWavDecoder.h"
#include <string.h>

namespace cAudio
{
	cWavDecoder::cWavDecoder(IDataSource* stream) : IAudioDecoder(stream), Valid(false)
    {
        const char* RIFFTAG = "RIFF";
		const char* WAVETAG = "WAVE";
		const char* FORMATTAG = "fmt ";
		const char* DATATAG = "data";

		char ident[4];
		int tempint32 = 0;
		short tempint16 = 0;
		char tempint8 = 0;

		unsigned int startOffset = 0;

		//Read the first 4 bytes
		Stream->seek(0, false);
		Stream->read(ident, 4);
		//Check to see if it is a valid RIFF file
		if(strncmp(ident, RIFFTAG, 4) == 0)
		{
			Stream->read(&tempint32, 4);
			//Check to see if the file is big enough to be valid (not completely accurate)
			if(tempint32 >= 44)
			{
				Stream->read(ident, 4);
				//Check that it is a wave file
				if(strncmp(ident, WAVETAG, 4) == 0)
				{
					//Save our position
					startOffset = Stream->getCurrentPos();

					//Scan for the first fmt chuck (not necessarily right after)
					do
					{
						Stream->read(ident, 4);
					}
					while((strncmp(ident, FORMATTAG, 4) != 0) && (Stream->getCurrentPos() < Stream->getSize()));

					//Did we find it?
					if(Stream->getCurrentPos() < (Stream->getSize() - 16))
					{
						//Yes, read it in
						Stream->read(&tempint32, 4);
						if(tempint32 >= 16)
						{
							//Check that it is in PCM format, we don't support compressed wavs
							Stream->read(&tempint16, 2);
							if(tempint16 == 1)
							{
								Stream->read(&tempint16, 2);
								Channels = tempint16;
								//We only support mono or stereo wavs
								if(Channels == 1 || Channels == 2)
								{
									Stream->read(&tempint32, 4);
									SampleRate = tempint32;
									Stream->read(&tempint32, 4);
									ByteRate = tempint32;
									Stream->read(&tempint16, 2);
									BlockAlign = tempint16;
									Stream->read(&tempint16, 2);
									BitsPerSample = tempint16;

									//We only support 8 bit or 16 bit wavs
									if(BitsPerSample == 8 || BitsPerSample == 16)
									{
										//Reset our pointer to start scanning for the data block
										Stream->seek(startOffset, false);
										//Scan for the first data chuck (not necessarily right after)
										do
										{
											Stream->read(ident, 4);
										}
										while((strncmp(ident, DATATAG, 4) != 0) && (Stream->getCurrentPos() < Stream->getSize()));

										//Did we find it?
										if(Stream->getCurrentPos() < Stream->getSize())
										{
											//Get size of data block
											Stream->read(&tempint32, 4);
											DataSize = tempint32;
											DataOffset = Stream->getCurrentPos();

											Valid = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
    }

    cWavDecoder::~cWavDecoder()
    {
		Channels = 0;
		SampleRate = 0;
		ByteRate = 0;
		BlockAlign = 0;
		BitsPerSample = 0;
		DataSize = 0;
		DataOffset = 0;
		Valid = false;
    }

    //!Returns wav channel format
    AudioFormats cWavDecoder::getFormat()
    {
        if(Channels == 1 && BitsPerSample == 8)
            return EAF_8BIT_MONO;
		else if(Channels == 1 && BitsPerSample == 16)
            return EAF_16BIT_MONO;
		else if(Channels == 2 && BitsPerSample == 8)
            return EAF_8BIT_STEREO;
        else
            return EAF_16BIT_STEREO;
    }

    //!Returns wav data frequency
    int cWavDecoder::getFrequency()
    {
        return SampleRate;
    }

    //!Returns if seeking is supported
    bool cWavDecoder::isSeekingSupported()
    {
        return true;
    }

	bool cWavDecoder::isValid()
	{
		return Valid;
	}

    //!Reads wav data
    int cWavDecoder::readAudioData(void* output, int amount)
    {
		int currentPos = Stream->getCurrentPos();
		int startPos = DataOffset;
		int endPos = DataOffset + DataSize;
		int amountToRead = amount;

		//Bounds checks (and adjustments if possible)
		if(currentPos > endPos)
			return 0;

		if(currentPos < startPos)
		{
			Stream->seek(startPos, false);
			currentPos = Stream->getCurrentPos();
		}

		if((currentPos + amountToRead) > endPos)
			amountToRead = endPos - currentPos;

		if(amountToRead < 0)
			amountToRead = 0;

		return Stream->read(output,amountToRead);

    }

    //!Sets data reader position
    bool cWavDecoder::setPosition(int position, bool relative)
    {
		int currentPos = Stream->getCurrentPos();
		int startPos = DataOffset;
		int endPos = DataOffset + DataSize;

		//Bounds checks (and adjustments if possible)
		if(!relative && position < startPos)
			position = startPos;
		if(!relative && position > endPos)
			position = endPos;
		if(relative && currentPos + position < startPos)
			position = startPos - currentPos;
		if(relative && currentPos + position > startPos)
			position = endPos - currentPos;

        Stream->seek(position,relative);
        return true;
    }

    //!Seeks wav data
    bool cWavDecoder::seek(float seconds,bool relative)
    {
		int amountToSeek = seconds * (float)SampleRate * (float)Channels * (float)(BitsPerSample/8);
        return setPosition(amountToSeek, relative);
    }


}


